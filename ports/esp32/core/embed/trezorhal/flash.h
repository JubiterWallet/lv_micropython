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

#ifndef TREZORHAL_FLASH_H
#define TREZORHAL_FLASH_H

#include <stdint.h>
#include <stdlib.h>
#include "secbool.h"

/*
// see docs/memory.md for more information
#define FLASH_SECTOR_COUNT 			2

#if defined TREZOR_MODEL_T || defined TREZOR_MODEL_R
#define FLASH_SECTOR_STORAGE_1 4
#define FLASH_SECTOR_STORAGE_2 16
#elif defined TREZOR_MODEL_1
#define FLASH_SECTOR_STORAGE_1 2
#define FLASH_SECTOR_STORAGE_2 3
#else
#define FLASH_SECTOR_STORAGE_1 0
#define FLASH_SECTOR_STORAGE_2 1
#endif
*/



// see docs/memory.md for more information

#if defined TREZOR_MODEL_T || defined TREZOR_MODEL_R
#define FLASH_SECTOR_COUNT 64
#elif defined TREZOR_MODEL_1
#define FLASH_SECTOR_COUNT 12
#else
#define FLASH_SECTOR_COUNT 64
#endif

//#define FLASH_SECTOR_BOARDLOADER_START 0
//                                           1
//#define FLASH_SECTOR_BOARDLOADER_END 0

//                                           3

#if defined TREZOR_MODEL_T || defined TREZOR_MODEL_R
#define FLASH_SECTOR_STORAGE_1 50
#define FLASH_SECTOR_STORAGE_2 51
#elif defined TREZOR_MODEL_1
#define FLASH_SECTOR_STORAGE_1 2
#define FLASH_SECTOR_STORAGE_2 3
#else
#define FLASH_SECTOR_STORAGE_1 50
#define FLASH_SECTOR_STORAGE_2 51
#endif

#define FLASH_SECTOR_BOOTLOADER 0

#define FLASH_SECTOR_FIRMWARE_START 3
//                                           7
//                                           8
//                                           9
//                                          10
#define FLASH_SECTOR_FIRMWARE_END 49

#define FLASH_SECTOR_UNUSED_START 52
//                                          13
//                                          14
#define FLASH_SECTOR_UNUSED_END 63

//#define FLASH_SECTOR_FIRMWARE_EXTRA_START 17
//                                          18
//                                          19
//                                          20
//                                          21
//                                          22
//#define FLASH_SECTOR_FIRMWARE_EXTRA_END 23

#define BOOTLOADER_SECTORS_COUNT (1)
#define STORAGE_SECTORS_COUNT (2)
#define FIRMWARE_SECTORS_COUNT (47)

extern const uint8_t STORAGE_SECTORS[STORAGE_SECTORS_COUNT];
extern const uint8_t FIRMWARE_SECTORS[FIRMWARE_SECTORS_COUNT];





void flash_init(void);

secbool __wur flash_unlock_write(void);
secbool __wur flash_lock_write(void);

const void *flash_get_address(uint8_t sector, uint32_t offset, uint32_t size);
uint32_t flash_sector_size(uint8_t sector);
secbool __wur flash_erase_sectors(const uint8_t *sectors, int len,
                                  void (*progress)(int pos, int len));
static inline secbool flash_erase(uint8_t sector) {
  return flash_erase_sectors(&sector, 1, NULL);
}
secbool __wur flash_write_byte(uint8_t sector, uint32_t offset, uint8_t data);
secbool __wur flash_write_word(uint8_t sector, uint32_t offset, uint32_t data);
secbool __wur flash_read_byte(uint8_t sector, uint32_t offset, uint8_t* data);
secbool __wur flash_read_nbyte(uint8_t sector, uint32_t offset, uint8_t* data, uint16_t len);
secbool __wur flash_read_word(uint8_t sector, uint32_t offset, uint32_t* data);
secbool  flash_read_nbyte_by_address(uint32_t address, uint8_t* data, uint16_t len);

/*
#define FLASH_OTP_NUM_BLOCKS 16
#define FLASH_OTP_BLOCK_SIZE 32

// OTP blocks allocation
#define FLASH_OTP_BLOCK_BATCH 0
#define FLASH_OTP_BLOCK_BOOTLOADER_VERSION 1
#define FLASH_OTP_BLOCK_VENDOR_HEADER_LOCK 2
#define FLASH_OTP_BLOCK_RANDOMNESS 3

secbool __wur flash_otp_read(uint8_t block, uint8_t offset, uint8_t *data,
                             uint8_t datalen);
secbool __wur flash_otp_write(uint8_t block, uint8_t offset,
                              const uint8_t *data, uint8_t datalen);
secbool __wur flash_otp_lock(uint8_t block);
secbool __wur flash_otp_is_locked(uint8_t block);
*/
#endif  // TREZORHAL_FLASH_H
