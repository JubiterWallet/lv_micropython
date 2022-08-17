#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "extmod/vfs_fat.h"

//#include "esp_flash.h"
#include "esp_flash_spi_init.h"
//#include "esp_vfs_fat.h"
#include "esp_system.h"
#include "esp_log.h"

#include "modtrezorio_ext_flash.h"

#define DEBUG 0
#if DEBUG
#define DEBUG_printf(...) ESP_LOGI("modExtFlash", __VA_ARGS__)
#else
#define DEBUG_printf(...) (void)0
#endif

typedef struct {
    union {
      int mosi_io_num;    ///< GPIO pin for Master Out Slave In (=spi_d) signal, or -1 if not used.
      int data0_io_num;   ///< GPIO pin for spi data0 signal in quad/octal mode, or -1 if not used.
    };
    union {
      int miso_io_num;    ///< GPIO pin for Master In Slave Out (=spi_q) signal, or -1 if not used.
      int data1_io_num;   ///< GPIO pin for spi data1 signal in quad/octal mode, or -1 if not used.
    };
    int sclk_io_num;      ///< GPIO pin for SPI Clock signal, or -1 if not used.
    union {
      int quadwp_io_num;  ///< GPIO pin for WP (Write Protect) signal, or -1 if not used.
      int data2_io_num;   ///< GPIO pin for spi data2 signal in quad/octal mode, or -1 if not used.
    };
    union {
      int quadhd_io_num;  ///< GPIO pin for HD (Hold) signal, or -1 if not used.
      int data3_io_num;   ///< GPIO pin for spi data3 signal in quad/octal mode, or -1 if not used.
    };
    int data4_io_num;     ///< GPIO pin for spi data4 signal in octal mode, or -1 if not used.
    int data5_io_num;     ///< GPIO pin for spi data5 signal in octal mode, or -1 if not used.
    int data6_io_num;     ///< GPIO pin for spi data6 signal in octal mode, or -1 if not used.
    int data7_io_num;     ///< GPIO pin for spi data7 signal in octal mode, or -1 if not used.
    int max_transfer_sz;  ///< Maximum transfer size, in bytes. Defaults to 4092 if 0 when DMA enabled, or to `SOC_SPI_MAXIMUM_BUFFER_SIZE` if DMA is disabled.
    uint32_t flags;       ///< Abilities of bus to be checked by the driver. Or-ed value of ``SPICOMMON_BUSFLAG_*`` flags.
    int intr_flags;       /**< Interrupt flag for the bus to set the priority, and IRAM attribute, see
                           *  ``esp_intr_alloc.h``. Note that the EDGE, INTRDISABLED attribute are ignored
                           *  by the driver. Note that if ESP_INTR_FLAG_IRAM is set, ALL the callbacks of
                           *  the driver, and their callee functions, should be put in the IRAM.
                           */
} spi_bus_config_t;

esp_err_t spi_bus_initialize(spi_host_device_t host_id, const spi_bus_config_t *bus_config, int dma_chan);
esp_err_t spi_bus_free(spi_host_device_t host_id);


// Forward declaration
const mp_obj_type_t ext_flash_type;



#define EXT_FLASH_INIT_DONE     0x01
#define EXT_FLASH_SECTOR_SIZE   4096

STATIC gpio_num_t pin_or_int(const mp_obj_t arg) {
    if (mp_obj_is_small_int(arg)) {
        return MP_OBJ_SMALL_INT_VALUE(arg);
    } else {
        // This raises a value error if the argument is not a Pin.
        return machine_pin_get_id(arg);
    }
}

#define SET_CONFIG_PIN(config, pin_var, arg_id) \
    if (arg_vals[arg_id].u_obj != mp_const_none) \
    config.pin_var = pin_or_int(arg_vals[arg_id].u_obj);


STATIC mp_obj_t ext_flash_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    enum {
        ARG_slot,
        ARG_hd,
        ARG_wp,
        ARG_miso,
        ARG_mosi,
        ARG_sck,
        ARG_cs,
        ARG_freq,
    };
    STATIC const mp_arg_t allowed_args[] = {
        { MP_QSTR_slot,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 10} },
        { MP_QSTR_hd,       MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_wp,       MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        // These are only needed if using SPI mode
        { MP_QSTR_miso,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_mosi,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_sck,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_cs,       MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        // freq is valid for both SPI and SDMMC interfaces
        { MP_QSTR_freq,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000000} },
    };
    mp_arg_val_t arg_vals[MP_ARRAY_SIZE(allowed_args)];
    mp_map_t kw_args;

    DEBUG_printf("Making new Ext flash:");
    DEBUG_printf("  Unpacking arguments");

    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);

    mp_arg_parse_all(n_args, args, &kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, arg_vals);

    DEBUG_printf("  slot=%d, hd=%p, wp=%p",
        arg_vals[ARG_slot].u_int, arg_vals[ARG_hd].u_obj, arg_vals[ARG_wp].u_obj);

    DEBUG_printf("  miso=%p, mosi=%p, sck=%p, cs=%p",
        arg_vals[ARG_miso].u_obj, arg_vals[ARG_mosi].u_obj,
        arg_vals[ARG_sck].u_obj, arg_vals[ARG_cs].u_obj);

    int slot_num = arg_vals[ARG_slot].u_int;
    if (slot_num < 4) {
        mp_raise_ValueError(MP_ERROR_TEXT("slot number must be >= 4"));
    }

    DEBUG_printf("  Setting up host configuration");

    ext_flash_obj_t *self = m_new_obj_with_finaliser(ext_flash_obj_t);
    self->base.type = &ext_flash_type;
    self->flags = 0;

    spi_bus_config_t slot_config = {
            .mosi_io_num = 11,
            .miso_io_num = 13,
            .sclk_io_num = 12,
            .quadhd_io_num = -1,
            .quadwp_io_num = -1,

    };
    esp_flash_spi_device_config_t device_config = {
            .host_id = SPI3_HOST,
            .cs_id = 0,
            .cs_io_num = 10,
            .io_mode = SPI_FLASH_DIO,
            .speed = ESP_FLASH_10MHZ,
        
    };

    self->host_id = device_config.host_id;

    SET_CONFIG_PIN(slot_config, mosi_io_num, ARG_mosi);
    SET_CONFIG_PIN(slot_config, miso_io_num, ARG_miso);
    SET_CONFIG_PIN(slot_config, sclk_io_num, ARG_sck);
    SET_CONFIG_PIN(slot_config, quadhd_io_num, ARG_hd);
    SET_CONFIG_PIN(slot_config, quadwp_io_num, ARG_wp);

    SET_CONFIG_PIN(device_config, cs_io_num, ARG_cs);
	
    switch(arg_vals[ARG_freq].u_int){
        case 5000000:
            device_config.speed = 0;
            break;
        case 10000000:
            device_config.speed = 1;
            break;
        case 20000000:
            device_config.speed = 2;
            break;
        case 26000000:
            device_config.speed = 3;
            break;
        case 40000000:
            device_config.speed = 4;
            break;
        case 80000000:
            device_config.speed = 5;
            break;
        case 120000000:
            device_config.speed = 6;
            break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            device_config.speed = arg_vals[ARG_freq].u_int;
            break;
        default:
            device_config.speed = 0;
            break;
    }

    DEBUG_printf("  miso=%d, mosi=%d, sck=%d, cs=%d",
        slot_config.miso_io_num, slot_config.mosi_io_num,
        slot_config.sclk_io_num, device_config.cs_io_num);

    DEBUG_printf("  slot=%d, hd=%d, wp=%d, freq = %d",
        arg_vals[ARG_slot].u_int, slot_config.quadhd_io_num, 
        slot_config.quadwp_io_num, device_config.speed);

    DEBUG_printf("  Calling init_slot()");
    // Initialize the SPI bus
    check_esp_err(spi_bus_initialize(self->host_id, &slot_config, 3));
    // Add device to the SPI bus
    check_esp_err(spi_bus_add_flash_device(&self->ext_flash, &device_config));

    check_esp_err(esp_flash_init(self->ext_flash));

    self->flags |= EXT_FLASH_INIT_DONE;
    DEBUG_printf("  Returning new card object: %p", self);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t ext_flash_deinit(mp_obj_t self_in) {
    ext_flash_obj_t *self = self_in;

    DEBUG_printf("De-init host\n");

    if (self->flags & EXT_FLASH_INIT_DONE) {
        check_esp_err(spi_bus_remove_flash_device(self->ext_flash));
        check_esp_err(spi_bus_free(self->host_id));
        self->flags &= ~EXT_FLASH_INIT_DONE;
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(ext_flash_deinit_obj, ext_flash_deinit);

STATIC mp_obj_t ext_flash_info(mp_obj_t self_in) {
    ext_flash_obj_t *self = self_in;

    if(!(self->flags & EXT_FLASH_INIT_DONE))
    {
        return mp_obj_new_bool(false);
    }

    mp_obj_t tuple[2] = {
        mp_obj_new_int_from_ull(self->ext_flash->size),
        mp_obj_new_int_from_uint(EXT_FLASH_SECTOR_SIZE),
    };
    return mp_obj_new_tuple(2, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ext_flash_info_obj, ext_flash_info);

STATIC mp_obj_t ext_flash_readblocks(mp_obj_t self_in, mp_obj_t block_num, mp_obj_t buf) {
    ext_flash_obj_t *self = self_in;
    mp_buffer_info_t bufinfo;
    esp_err_t err;
	
    if(!(self->flags & EXT_FLASH_INIT_DONE))
    {
        return mp_obj_new_bool(false);
    }

    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    err = esp_flash_read(self->ext_flash, bufinfo.buf, mp_obj_get_int(block_num) * EXT_FLASH_SECTOR_SIZE, bufinfo.len);
	DEBUG_printf("\nreadblocks: block_num = %d, len = %d, err = %d", mp_obj_get_int(block_num), bufinfo.len, err);
    return mp_obj_new_bool(err == ESP_OK);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(ext_flash_readblocks_obj, ext_flash_readblocks);

STATIC mp_obj_t ext_flash_writeblocks(mp_obj_t self_in, mp_obj_t block_num, mp_obj_t buf) {
    ext_flash_obj_t *self = self_in;
    mp_buffer_info_t bufinfo;
    esp_err_t err;
    uint32_t erase_len;
	
    if(!(self->flags & EXT_FLASH_INIT_DONE))
    {
        return mp_obj_new_bool(false);
    }

    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
	DEBUG_printf("\nwriteblocks: block_num = %d, len = %d", mp_obj_get_int(block_num), bufinfo.len);
    if(bufinfo.len % EXT_FLASH_SECTOR_SIZE)
    {
       erase_len = ((bufinfo.len / EXT_FLASH_SECTOR_SIZE) + 1) * EXT_FLASH_SECTOR_SIZE;
    }
    else
    {
        erase_len = bufinfo.len;
    }
     
    uint32_t addr = mp_obj_get_int(block_num) * EXT_FLASH_SECTOR_SIZE;
    err = esp_flash_erase_region(self->ext_flash, addr, erase_len);
    if(err != ESP_OK)
    {
        return mp_obj_new_bool(false);
    }
    err = esp_flash_write(self->ext_flash, bufinfo.buf, addr, bufinfo.len);
    return mp_obj_new_bool(err == ESP_OK);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(ext_flash_writeblocks_obj, ext_flash_writeblocks);

STATIC mp_obj_t ext_flash_eraselocks(mp_obj_t self_in, mp_obj_t block_num, mp_obj_t len) {
    ext_flash_obj_t *self = self_in;
    esp_err_t err;
    uint32_t length = mp_obj_get_int(len);
    uint32_t erase_len;
	
	DEBUG_printf("\neraselocks: block_num = %d, len = %d", mp_obj_get_int(block_num), length);

    if(!(self->flags & EXT_FLASH_INIT_DONE))
    {
        return mp_obj_new_bool(false);
    }

    if(length % EXT_FLASH_SECTOR_SIZE)
    {
       erase_len = ((length / EXT_FLASH_SECTOR_SIZE) + 1) * EXT_FLASH_SECTOR_SIZE;
    }
    else
    {
        erase_len = length;
    }

    err = esp_flash_erase_region(self->ext_flash, mp_obj_get_int(block_num) * EXT_FLASH_SECTOR_SIZE, erase_len);

    return mp_obj_new_bool(err == ESP_OK);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(ext_flash_eraselocks_obj, ext_flash_eraselocks);

STATIC mp_obj_t ext_flash_erasechip(mp_obj_t self_in) {
    ext_flash_obj_t *self = self_in;
    esp_err_t err;

    if(!(self->flags & EXT_FLASH_INIT_DONE))
    {
        return mp_obj_new_bool(false);
    }

    err = esp_flash_erase_chip(self->ext_flash);

    return mp_obj_new_bool(err == ESP_OK);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ext_flash_erasechip_obj, ext_flash_erasechip);


STATIC mp_obj_t ext_flash_ioctl(mp_obj_t self_in, mp_obj_t cmd_in, mp_obj_t arg_in) {
    ext_flash_obj_t *self = self_in;
 //   esp_err_t err = ESP_OK;
    mp_int_t cmd = mp_obj_get_int(cmd_in);

	DEBUG_printf("\nioctl: cmd = %d", cmd);
    switch (cmd) {
        case MP_BLOCKDEV_IOCTL_INIT:
            //err = sdcard_ensure_card_init(self, false);
            return MP_OBJ_NEW_SMALL_INT(0);

        case MP_BLOCKDEV_IOCTL_DEINIT:
            // Ensure that future attempts to look at info re-read the card
            if (self->flags & EXT_FLASH_INIT_DONE) {
                check_esp_err(spi_bus_remove_flash_device(self->ext_flash));
                check_esp_err(spi_bus_free(self->host_id));
                self->flags &= ~EXT_FLASH_INIT_DONE;
            }
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_SYNC:
            // nothing to do
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT:
            if((self->flags & EXT_FLASH_INIT_DONE) == 0){
                return MP_OBJ_NEW_SMALL_INT(-1);
            }
            return MP_OBJ_NEW_SMALL_INT(self->ext_flash->size / EXT_FLASH_SECTOR_SIZE);

        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE:
            if((self->flags & EXT_FLASH_INIT_DONE) == 0){
                return MP_OBJ_NEW_SMALL_INT(-1);
            }
            return MP_OBJ_NEW_SMALL_INT(EXT_FLASH_SECTOR_SIZE);
		case MP_BLOCKDEV_IOCTL_BLOCK_ERASE:
			//mp_int_t block = mp_obj_get_int(arg_in);
			
            if((self->flags & EXT_FLASH_INIT_DONE) == 0){
                return MP_OBJ_NEW_SMALL_INT(-1);
            }
			esp_flash_erase_region(self->ext_flash,  mp_obj_get_int(arg_in) * EXT_FLASH_SECTOR_SIZE, EXT_FLASH_SECTOR_SIZE);
			return MP_OBJ_NEW_SMALL_INT(0); // success
        default: // unknown command
            return MP_OBJ_NEW_SMALL_INT(-1); // error
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(ext_flash_ioctl_obj, ext_flash_ioctl);



STATIC const mp_rom_map_elem_t ext_flash_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&ext_flash_info_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&ext_flash_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&ext_flash_deinit_obj) },
    // block device protocol
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&ext_flash_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&ext_flash_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&ext_flash_ioctl_obj) },
    { MP_ROM_QSTR(MP_QSTR_eraselocks), MP_ROM_PTR(&ext_flash_eraselocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_erasechip), MP_ROM_PTR(&ext_flash_erasechip_obj) },    
};

STATIC MP_DEFINE_CONST_DICT(ext_flash_locals_dict, ext_flash_locals_dict_table);

const mp_obj_type_t ext_flash_type = {
    { &mp_type_type  },       //mp_type_type
    .name = MP_QSTR_ext_flash,
    .make_new = ext_flash_make_new,
    .locals_dict = (mp_obj_dict_t *)&ext_flash_locals_dict,
};




