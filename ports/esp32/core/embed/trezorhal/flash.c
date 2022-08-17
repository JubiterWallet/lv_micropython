/*
 * This file is part of the Trezor project, https://trezor.io/
 *
 * Copyright (c) SatoshiLabs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include STM32_HAL_H

#include <string.h>

#include "common.h"
#include "flash.h"
#include "esp_spi_flash.h"
#include "esp_flash_internal.h"


//#define FLASH_DEBUG
#ifdef FLASH_DEBUG

#define DEBUG(format, args...)         do{ printf(format, ##args); }while(0)

void print_var(uint8_t const* buf, uint32_t bufsize)
{
  for(uint32_t i=0; i<bufsize; i++) DEBUG("%02X ", buf[i]);
}

#else

#define DEBUG(format, args...)			do{ }while(0)

void print_var(uint8_t const* buf, uint32_t bufsize){}

#endif


static const uint32_t FLASH_SECTOR_TABLE[FLASH_SECTOR_COUNT + 1] = {
    [0] = 0x00000000,   // - 0x00008FFF |  36 KiB			bootloader
    [1] = 0x00009000,   // - 0x0000EFFF |  24 KiB			nvs
    [2] = 0x0000F000,   // - 0x0000FFFF |   4 KiB			phy_init
    [3] = 0x00010000,   // - 0x0001FFFF |  64 KiB			FLASH_SECTOR_FIRMWARE_START
    [4] = 0x00020000,   // - 0x0002FFFF |  64 KiB
    [5] = 0x00030000,   // - 0x0003FFFF |  64 KiB
    [6] = 0x00040000,   // - 0x0004FFFF |  64 KiB
    [7] = 0x00050000,   // - 0x0005FFFF |  64 KiB
    [8] = 0x00060000,   // - 0x0006FFFF |  64 KiB
    [9] = 0x00070000,   // - 0x0007FFFF |  64 KiB
    [10] = 0x00080000,  // - 0x0008FFFF |  64 KiB
    [11] = 0x00090000,  // - 0x0009FFFF |  64 KiB
    [12] = 0x000A0000,  // - 0x000AFFFF |  64 KiB
    [13] = 0x000B0000,  // - 0x000BFFFF |  64 KiB
    [14] = 0x000C0000,  // - 0x000CFFFF |  64 KiB
    [15] = 0x000D0000,  // - 0x000DFFFF |  64 KiB
    [16] = 0x000E0000,  // - 0x000EFFFF |  64 KiB
    [17] = 0x000F0000,  // - 0x000FFFFF |  64 KiB
    [18] = 0x00100000,  // - 0x0010FFFF |  64 KiB
    [19] = 0x00110000,  // - 0x0011FFFF |  64 KiB
    [20] = 0x00120000,  // - 0x0012FFFF |  64 KiB
    [21] = 0x00130000,  // - 0x0013FFFF |  64 KiB
    [22] = 0x00140000,  // - 0x0014FFFF |  64 KiB
    [23] = 0x00150000,  // - 0x0015FFFF |  64 KiB
    [24] = 0x00160000,	// - 0x0016FFFF |  64 KiB
    [25] = 0x00170000,	// - 0x0017FFFF |  64 KiB
    [26] = 0x00180000,	// - 0x0018FFFF |  64 KiB
    [27] = 0x00190000,	// - 0x0019FFFF |  64 KiB
    [28] = 0x001A0000,	// - 0x001AFFFF |  64 KiB
    [29] = 0x001B0000,	// - 0x001BFFFF |  64 KiB
    [30] = 0x001C0000,	// - 0x001CFFFF |  64 KiB
    [31] = 0x001D0000,	// - 0x001DFFFF |  64 KiB
    [32] = 0x001E0000,	// - 0x001EFFFF |  64 KiB
    [33] = 0x001F0000,	// - 0x001FFFFF |  64 KiB
    [34] = 0x00200000,	// - 0x0020FFFF |  64 KiB
    [35] = 0x00210000,	// - 0x0021FFFF |  64 KiB
    [36] = 0x00220000,	// - 0x0022FFFF |  64 KiB
    [37] = 0x00230000,	// - 0x0023FFFF |  64 KiB
    [38] = 0x00240000,	// - 0x0024FFFF |  64 KiB
    [39] = 0x00250000,	// - 0x0025FFFF |  64 KiB
    [40] = 0x00260000,	// - 0x0026FFFF |  64 KiB
    [41] = 0x00270000,	// - 0x0027FFFF |  64 KiB
    [42] = 0x00280000,	// - 0x0028FFFF |  64 KiB
    [43] = 0x00290000,	// - 0x0029FFFF |  64 KiB
    [44] = 0x002A0000,	// - 0x002AFFFF |  64 KiB
    [45] = 0x002B0000,	// - 0x002BFFFF |  64 KiB
    [46] = 0x002C0000,	// - 0x002CFFFF |  64 KiB
    [47] = 0x002D0000,	// - 0x002DFFFF |  64 KiB
    [48] = 0x002E0000,	// - 0x002EFFFF |  64 KiB
    [49] = 0x002F0000,	// - 0x002FFFFF |  64 KiB			FLASH_SECTOR_FIRMWARE_END
    [50] = 0x00400000,	// - 0x0030FFFF |  64 KiB
    [51] = 0x00410000,	// - 0x0031FFFF |  64 KiB
    [52] = 0x00420000,	// - 0x0032FFFF |  64 KiB
    [53] = 0x00430000,	// - 0x0033FFFF |  64 KiB
    [54] = 0x00440000,	// - 0x0034FFFF |  64 KiB
    [55] = 0x00450000,	// - 0x0035FFFF |  64 KiB
    [56] = 0x00460000,	// - 0x0036FFFF |  64 KiB
    [57] = 0x00470000,	// - 0x0037FFFF |  64 KiB
    [58] = 0x00480000,	// - 0x0038FFFF |  64 KiB
    [59] = 0x00490000,	// - 0x0039FFFF |  64 KiB
    [60] = 0x004A0000,	// - 0x003AFFFF |  64 KiB
    [61] = 0x004B0000,	// - 0x003BFFFF |  64 KiB
    [62] = 0x004C0000,	// - 0x003CFFFF |  64 KiB
    [63] = 0x004D0000,	// - 0x003DFFFF |  64 KiB	
    [64] = 0x004F0000,  // last element - not a valid sector
};

const uint8_t FIRMWARE_SECTORS[FIRMWARE_SECTORS_COUNT] = {
    FLASH_SECTOR_FIRMWARE_START,
    4,
    5,
    6,
    7,
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    FLASH_SECTOR_FIRMWARE_END,
    // FLASH_SECTOR_FIRMWARE_EXTRA_START,
    // 18,
    // 19,
    // 20,
    // 21,
    // 22,
    // FLASH_SECTOR_FIRMWARE_EXTRA_END,
};

const uint8_t STORAGE_SECTORS[STORAGE_SECTORS_COUNT] = {
    FLASH_SECTOR_STORAGE_1,
    FLASH_SECTOR_STORAGE_2,
};



// Take mutex protecting access to spi_flash_* APIs
extern void spi_flash_op_lock(void);

// Release said mutex
extern void spi_flash_op_unlock(void);


secbool flash_unlock_write(void) {
  spi_flash_op_unlock();
  return sectrue;
}

secbool flash_lock_write(void) {
  spi_flash_op_lock();
  return sectrue;
}

const void *flash_get_address(uint8_t sector, uint32_t offset, uint32_t size) {
  if (sector >= FLASH_SECTOR_COUNT) {
    return NULL;
  }
  const uint32_t addr = FLASH_SECTOR_TABLE[sector] + offset;
  const uint32_t next = FLASH_SECTOR_TABLE[sector + 1];
  if (addr + size > next) {
    return NULL;
  }
  return (const void *)addr;
}

uint32_t flash_sector_size(uint8_t sector) {
  if (sector >= FLASH_SECTOR_COUNT) {
    return 0;
  }
  return FLASH_SECTOR_TABLE[sector + 1] - FLASH_SECTOR_TABLE[sector];
}

secbool flash_erase_sectors(const uint8_t *sectors, int len,
                            void (*progress)(int pos, int len)) {
  ensure(flash_unlock_write(), NULL);
  if (progress) {
    progress(0, len);
  }
  
  for (int i = 0; i < len; i++) {
  	uint32_t size = flash_sector_size(sectors[i]);
	DEBUG("\n\terase: sectors = %d, size = %d, addr = 0x%x", sectors[i], size, FLASH_SECTOR_TABLE[sectors[i]]);
	if(sectors[i] < 50){
		DEBUG("\nCode area is not allowed to erase");
		return secfalse;
	}
		
	if (spi_flash_erase_range(FLASH_SECTOR_TABLE[sectors[i]], size) != ESP_OK) {
      ensure(flash_lock_write(), NULL);
	  DEBUG("\n\terase sectors failed, err1");
      return secfalse;
    }
	
    // check whether the sector was really deleted (contains only 0xFF)
    const uint32_t addr_start = FLASH_SECTOR_TABLE[sectors[i]],
                   addr_end = FLASH_SECTOR_TABLE[sectors[i] + 1];
	uint8_t read[256];
    for (uint32_t addr = addr_start; addr < addr_end; addr += 256) {
	  spi_flash_read(addr, read, 256);
	  for(int j = 0; j < 256; j++){
	      if (read[j] != 0xFF) {
	        ensure(flash_lock_write(), NULL);
			DEBUG("\n\terase sectors failed, err2");
	        return secfalse;
	      }
	  }
    }
	
    if (progress) {
      progress(i + 1, len);
    }
  }
  ensure(flash_lock_write(), NULL);
  return sectrue;
}

secbool flash_write_byte(uint8_t sector, uint32_t offset, uint8_t data) {
  uint32_t address = (uint32_t)flash_get_address(sector, offset, 1);
  DEBUG("\nflash_write_byte: sector = %d, offset = %d, address = 0x%x, data = 0x%x, ", sector,offset, address, data);
  if (sector < FLASH_SECTOR_STORAGE_1) {
  	DEBUG("\nsector error");
    return secfalse;
  }
  
  if (ESP_OK != spi_flash_write(address, &data, 1)) {
  	DEBUG("\n%s:write err1", __FUNCTION__);
    return secfalse;
  }
  
  uint8_t readvalue;
  spi_flash_read(address, (uint8_t *)&readvalue, 1);
  if (data != readvalue) {
  	DEBUG("\n%s:write err2", __FUNCTION__);
    return secfalse;
  }
  return sectrue;
}

secbool flash_read_byte(uint8_t sector, uint32_t offset, uint8_t* data) {
  uint32_t address = (uint32_t)flash_get_address(sector, offset, 1);
  if (address < FLASH_SECTOR_TABLE[0] || (address + 1 > FLASH_SECTOR_TABLE[FLASH_SECTOR_COUNT])) {
	DEBUG("\n%s:read err1", __FUNCTION__);	
	return secfalse;
  }

  if (ESP_OK != spi_flash_read(address, data, 1)) {
	DEBUG("\n%s:read err2", __FUNCTION__);
    return secfalse;
  }
  
  DEBUG("\n%s: sector = %d, offset = %d, address = %d, data = 0x%02x", __FUNCTION__, sector, offset, address, *data);
  return sectrue;
}

secbool flash_read_nbyte_by_address(uint32_t address, uint8_t* data, uint16_t len) {
	if (address < FLASH_SECTOR_TABLE[0] || (address + len > FLASH_SECTOR_TABLE[FLASH_SECTOR_COUNT])) {
	  DEBUG("\n%s:read err1", __FUNCTION__);
	  return secfalse;
	}

	if (ESP_OK != spi_flash_read(address, data, len)) {
	  DEBUG("\n%s:read err2", __FUNCTION__);
	  return secfalse;
	}
	
	return sectrue;
}

secbool flash_read_nbyte(uint8_t sector, uint32_t offset, uint8_t* data, uint16_t len) {
	uint32_t address = (uint32_t)flash_get_address(sector, offset, len);
	if (address < FLASH_SECTOR_TABLE[0] || (address + len > FLASH_SECTOR_TABLE[FLASH_SECTOR_COUNT])) {
	  DEBUG("\n%s:read err1", __FUNCTION__);
	  return secfalse;
	}
	
	if (ESP_OK != spi_flash_read(address, data, len)) {
		return secfalse;
	}
	
	DEBUG("\n%s: sector = %d, offset = %d, address = %d, data:", __FUNCTION__, sector, offset, address);
	print_var(data, len);
	return sectrue;
}

secbool flash_read_word(uint8_t sector, uint32_t offset, uint32_t* data) {
	uint32_t address = (uint32_t)flash_get_address(sector, offset, 4);
	if (address < FLASH_SECTOR_TABLE[0] || (address + 4 > FLASH_SECTOR_TABLE[FLASH_SECTOR_COUNT])) {
	  DEBUG("\n%s:read err1", __FUNCTION__);
	  return secfalse;
	}
	uint8_t read[4] = {0};
	if (ESP_OK != spi_flash_read(address, read, 4)) {
		DEBUG("\n%s:read err2", __FUNCTION__);
		return secfalse;
	}

	*data = read[0] + (read[1] << 8) + (read[2] << 16) + (read[3] << 24);
	DEBUG("\nflash_read_word: sector = %d, offset = %d, address = 0x%08x, data = 0x%x", sector,offset, address, *data);
	return sectrue;
}


secbool flash_write_word(uint8_t sector, uint32_t offset, uint32_t data) {
  uint32_t address = (uint32_t)flash_get_address(sector, offset, 4);
  DEBUG("\nflash_write_word: sector = %d, offset = %d, address = 0x%x, data = 0x%x", sector,offset, address, data);
  if (sector < FLASH_SECTOR_STORAGE_1) {
  	DEBUG("\n%s:sector error", __FUNCTION__);
    return secfalse;
  }
  
  //if (offset % sizeof(uint32_t)) {  // we write only at 4-byte boundary
  //  return secfalse;
  //}

  uint8_t write[4] = {data & 0xFF, (data >> 8) & 0xff, (data >> 16) & 0xff, (data >> 24) & 0xff};
  if (ESP_OK != spi_flash_write(address, write, 4)) {
  	DEBUG("\n%s:write err1", __FUNCTION__);
    return secfalse;
  }

  uint8_t read[4] = {0};
  spi_flash_read(address, read, 4);
  
  uint32_t readvalue = read[0] + (read[1] << 8) + (read[2] << 16) + (read[3] << 24);
  if (data != readvalue) {
  	DEBUG("\n%s:write err2", __FUNCTION__);
    return secfalse;
  }
  return sectrue;
}

/*
#define FLASH_OTP_LOCK_BASE 0x1FFF7A00U

secbool flash_otp_read(uint8_t block, uint8_t offset, uint8_t *data,
                       uint8_t datalen) {
  if (block >= FLASH_OTP_NUM_BLOCKS ||
      offset + datalen > FLASH_OTP_BLOCK_SIZE) {
    return secfalse;
  }
  for (uint8_t i = 0; i < datalen; i++) {
    data[i] = *(__IO uint8_t *)(FLASH_OTP_BASE + block * FLASH_OTP_BLOCK_SIZE +
                                offset + i);
  }
  return sectrue;
}

secbool flash_otp_write(uint8_t block, uint8_t offset, const uint8_t *data,
                        uint8_t datalen) {
  if (block >= FLASH_OTP_NUM_BLOCKS ||
      offset + datalen > FLASH_OTP_BLOCK_SIZE) {
    return secfalse;
  }
  ensure(flash_unlock_write(), NULL);
  for (uint8_t i = 0; i < datalen; i++) {
    uint32_t address =
        FLASH_OTP_BASE + block * FLASH_OTP_BLOCK_SIZE + offset + i;
    ensure(sectrue * (HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
                                                  address, data[i])),
           NULL);
  }
  ensure(flash_lock_write(), NULL);
  return sectrue;
}

secbool flash_otp_lock(uint8_t block) {
  if (block >= FLASH_OTP_NUM_BLOCKS) {
    return secfalse;
  }
  ensure(flash_unlock_write(), NULL);
  HAL_StatusTypeDef ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
                                            FLASH_OTP_LOCK_BASE + block, 0x00);
  ensure(flash_lock_write(), NULL);
  return sectrue * (ret == HAL_OK);
}

secbool flash_otp_is_locked(uint8_t block) {
  return sectrue * (0x00 == *(__IO uint8_t *)(FLASH_OTP_LOCK_BASE + block));
}

*/
