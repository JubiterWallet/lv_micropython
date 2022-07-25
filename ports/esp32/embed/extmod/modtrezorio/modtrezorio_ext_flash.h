#ifndef __EXT_FLASH__
#define __EXT_FLASH__

#include "py/obj.h"
#include "esp_flash.h"

typedef struct _ext_flash_obj_t {
    mp_obj_base_t base;
    mp_int_t flags;
    mp_int_t host_id;
    esp_flash_t *ext_flash;
} ext_flash_obj_t;



extern const mp_obj_type_t ext_flash_type;




#endif

