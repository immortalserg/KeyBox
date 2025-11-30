/*
 * flash.h
 *
 *  Created on: Dec 2, 2023
 *      Author: batya
 */

#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>
#include "cfg.h"

uint32_t flash_init();
void flash_get_flash_id(uint8_t *manid, uint8_t *memtype, uint8_t *memdensity);
void flash_erase_chip();

// Must be no more than 128 bytes total
struct __attribute__((packed)) flash_device_info {
	uint32_t magic;
	// ED25519
	uint8_t public_key[32];
	uint8_t private_key[64];
	// BLE passkey
	uint32_t passkey;
	// BLE name
	uint8_t name[32];
// --+-- §Õ§à§Ò§Ñ§Ó§Ý§Ö§ß§Ú§Ö §ç§â§Ö§ß§Ö§ß§Ú§ñ §á§å§Ò§Ý§Ú§é§ß§à§Ô§à §Ü§Ý§ð§é§Ñ x25519, X25519 private key §Þ§í §¯§¦ §ç§â§Ñ§ß§Ú§Þ §à§ä§Õ§Ö§Ý§î§ß§à ¡ª §Þ§í §Ü§Ñ§Ø§Õ§í§Û §â§Ñ§Ù §Ü§à§ß§Ó§Ö§â§ä§Ú§â§å§Ö§Þ §Ú§Ù Ed25519 private_key
    uint8_t x25519_public_key[32];
// --+--
};

#define FLASH_MAGIC (('K' << 24 )| ('B' << 16) | ('O' << 8) | 'X')

// Erasable sector size
#define FLASH_SECTOR_SZ 4096

// Programmable page size
#define FLASH_PAGE_SZ 256

#define FLASH_USERS_MAX 128

#define FLASH_USER_BLOCKED 0
#define FLASH_USER_OKAY    0xFF

// Must be no more than 64 bytes total
struct __attribute__((packed)) flash_user_info {
	uint8_t user[16];
	uint8_t public_key[32];
	uint8_t name[32];
	uint8_t state;
};

uint32_t flash_user_count();
uint32_t flash_user_get_at(uint32_t i, struct flash_user_info *flash_user);
uint32_t flash_user_search(uint8_t *user, struct flash_user_info *flash_user, uint32_t *n);
uint32_t flash_user_block(uint8_t *user);
uint32_t flash_user_add(struct flash_user_info *flash_user);
uint32_t flash_user_overwrite(struct flash_user_info *flash_user, uint32_t user_index);

uint32_t flash_device_info_get(struct flash_device_info *flash_device);
uint32_t flash_device_info_set(struct flash_device_info *flash_device);

#if CFG_DEBUG_ENABLE
void dbg_flash_report();
#else
#define dbg_flash_report()
#endif


#endif /* USER_FONTFLASH_H_ */
