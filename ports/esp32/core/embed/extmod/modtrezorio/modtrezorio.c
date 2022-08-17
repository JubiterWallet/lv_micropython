#include <string.h>

#include "py/mphal.h"
#include "py/objstr.h"
#include "py/runtime.h"

//#if MICROPY_PY_TREZORIO

#include <unistd.h>

// #include "button.h"
#include "touch.h"
#include "usb.h"
#include "trezorobj.h"

#define CHECK_PARAM_RANGE(value, minimum, maximum)  \
  if (value < minimum || value > maximum) {         \
    mp_raise_ValueError(#value " is out of range"); \
  }

// clang-format off
#include "modtrezorio_ext_flash.h"
#include "modtrezorio-hid.h"
#include "modtrezorio-poll.h"
#include "modtrezorio-vcp.h"
#include "modtrezorio-webusb.h"
#include "modtrezorio-usb.h"
// clang-format on
#if defined TREZOR_MODEL_T
//#include "modtrezorio-fatfs.h"
//#include "modtrezorio-sbu.h"
//#include "modtrezorio-sdcard.h"
#endif


/// package: trezorio.__init__
/// from . import fatfs, sdcard

/// POLL_READ: int  # wait until interface is readable and return read data
/// POLL_WRITE: int  # wait until interface is writable
///
/// TOUCH: int  # interface id of the touch events
/// TOUCH_START: int  # event id of touch start event
/// TOUCH_MOVE: int  # event id of touch move event
/// TOUCH_END: int  # event id of touch end event

/// BUTTON: int  # interface id of button events
/// BUTTON_PRESSED: int  # button down event
/// BUTTON_RELEASED: int  # button up event
/// BUTTON_LEFT: int  # button number of left button
/// BUTTON_RIGHT: int  # button number of right button

/// WireInterface = Union[HID, WebUSB]

STATIC const mp_rom_map_elem_t mp_module_trezorio_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_trezorio)},
	{MP_ROM_QSTR(MP_QSTR_extflash), MP_ROM_PTR(&ext_flash_type)},			//替代trezorio_fatfs_module，需要挂载到虚拟文件系统中

//#if defined TREZOR_MODEL_T
//    {MP_ROM_QSTR(MP_QSTR_fatfs), MP_ROM_PTR(&mod_trezorio_fatfs_module)},	//之所以没给外部flash做FATFS是因为IDF本身就带FATFS,编译会冲突
//    {MP_ROM_QSTR(MP_QSTR_SBU), MP_ROM_PTR(&mod_trezorio_SBU_type)},
//    {MP_ROM_QSTR(MP_QSTR_sdcard), MP_ROM_PTR(&mod_trezorio_sdcard_module)},

    {MP_ROM_QSTR(MP_QSTR_TOUCH), MP_ROM_INT(TOUCH_IFACE)},
    {MP_ROM_QSTR(MP_QSTR_TOUCH_START), MP_ROM_INT((TOUCH_START >> 24) & 0xFFU)},
    {MP_ROM_QSTR(MP_QSTR_TOUCH_MOVE), MP_ROM_INT((TOUCH_MOVE >> 24) & 0xFFU)},
    {MP_ROM_QSTR(MP_QSTR_TOUCH_END), MP_ROM_INT((TOUCH_END >> 24) & 0xFFU)},
    
// #elif defined TREZOR_MODEL_1 || defined TREZOR_MODEL_R
//     {MP_ROM_QSTR(MP_QSTR_BUTTON), MP_ROM_INT(BUTTON_IFACE)},
//     {MP_ROM_QSTR(MP_QSTR_BUTTON_PRESSED),
//      MP_ROM_INT((BTN_EVT_DOWN >> 24) & 0x3U)},
//     {MP_ROM_QSTR(MP_QSTR_BUTTON_RELEASED),
//      MP_ROM_INT((BTN_EVT_UP >> 24) & 0x3U)},
//     {MP_ROM_QSTR(MP_QSTR_BUTTON_LEFT), MP_ROM_INT(BTN_LEFT)},
//     {MP_ROM_QSTR(MP_QSTR_BUTTON_RIGHT), MP_ROM_INT(BTN_RIGHT)},
// #endif

    // {MP_ROM_QSTR(MP_QSTR_FlashOTP), MP_ROM_PTR(&mod_trezorio_FlashOTP_type)},

    {MP_ROM_QSTR(MP_QSTR_USB), MP_ROM_PTR(&mod_trezorio_USB_type)},
    {MP_ROM_QSTR(MP_QSTR_HID), MP_ROM_PTR(&mod_trezorio_HID_type)},
    {MP_ROM_QSTR(MP_QSTR_VCP), MP_ROM_PTR(&mod_trezorio_VCP_type)},
    {MP_ROM_QSTR(MP_QSTR_WebUSB), MP_ROM_PTR(&mod_trezorio_WebUSB_type)},

    {MP_ROM_QSTR(MP_QSTR_poll), MP_ROM_PTR(&mod_trezorio_poll_obj)},
    {MP_ROM_QSTR(MP_QSTR_POLL_READ), MP_ROM_INT(POLL_READ)},
    {MP_ROM_QSTR(MP_QSTR_POLL_WRITE), MP_ROM_INT(POLL_WRITE)},
};

STATIC MP_DEFINE_CONST_DICT(mp_module_trezorio_globals,
                            mp_module_trezorio_globals_table);

const mp_obj_module_t mp_module_trezorio = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t*)&mp_module_trezorio_globals,
};
MP_REGISTER_MODULE(MP_QSTR_trezorio, mp_module_trezorio);

