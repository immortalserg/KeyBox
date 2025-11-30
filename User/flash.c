/*
 * flash.c
 *
 *  Created on: Dec 2, 2023
 *      Author: batya
 */

#include <string.h>
#include "flash.h"
#include "debug.h"

uint8_t flash_spi(uint8_t dout) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    SPI1->DATAR = dout;

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

    return SPI1->DATAR;
}

#define flash_cs0 SPI_SSOutputCmd(SPI1, ENABLE);
#define flash_cs1 SPI_SSOutputCmd(SPI1, DISABLE);

void flash_write_en() {
    flash_cs0;

    flash_spi(0x06);

    flash_cs1;
}

uint32_t flash_is_busy() {
    uint8_t sts;

    flash_cs0;

    flash_spi(0x05);
    sts = flash_spi(0xFF);

    flash_cs1;

    return sts & 1;
}

void flash_erase_chip() {
    flash_write_en();

    flash_cs0;

    flash_spi(0xC7);

    flash_cs1;

    while (flash_is_busy());
}

void flash_erase_sector(uint32_t addr) {
    flash_write_en();

    flash_cs0;

    flash_spi(0x20);
    flash_spi(addr >> 16);
    flash_spi(addr >> 8);
    flash_spi(addr);

    flash_cs1;

    while (flash_is_busy());
}

// Program up to 256 bytes
void flash_prog(uint32_t addr, uint8_t *src, uint32_t len) {
    flash_write_en();

    flash_cs0;

    flash_spi(0x02);
    flash_spi(addr >> 16);
    flash_spi(addr >> 8);
    flash_spi(addr);

    uint32_t i;

    for (i = 0; i < len; i++) {
        flash_spi(*src++);
    }

    flash_cs1;

    while (flash_is_busy());
}

void flash_read(uint32_t addr, uint8_t *dst, uint32_t len) {
    flash_cs0;

    flash_spi(0x03);
    flash_spi(addr >> 16);
    flash_spi(addr >> 8);
    flash_spi(addr);

    uint32_t i;

    for (i = 0; i < len; i++) {
        *dst++ = flash_spi(0xFF);
    }

    flash_cs1;
}

// Returns 1 if flash range is not erased
uint32_t flash_check_erasure(uint32_t addr, uint32_t len) {
    flash_cs0;

    flash_spi(0x03);
    flash_spi(addr >> 16);
    flash_spi(addr >> 8);
    flash_spi(addr);

    uint32_t i;
	uint8_t dat;
	uint32_t failed = 0;

    for (i = 0; i < len; i++) {
        dat = flash_spi(0xFF);
		
		if (dat != 0xFF) {
			failed = 1;
			break;
		}
    }

    flash_cs1;
	
	return failed;
}

/**
 * @brief Get flash id words
 * @param manid Pointer to manid to be stored in. Set to NULL to ignore
 * @param memtype Pointer to manid to be stored in. Set to NULL to ignore
 * @param memdensity Pointer to memdensity to be stored in. Set to NULL to ignore
 */
void flash_get_flash_id(uint8_t *manid, uint8_t *memtype, uint8_t *memdensity) {
    uint8_t dat;

    flash_cs0;

    flash_spi(0x9F);

    dat = flash_spi(0xFF);
    if (manid)
        *manid = dat;

    dat = flash_spi(0xFF);
    if (memtype)
        *memtype = dat;

    dat = flash_spi(0xFF);
    if (memdensity)
        *memdensity = dat;

    flash_cs1;
}

/**
 * @brief Init GPIO & SPI
 */
uint32_t flash_init() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_InitTypeDef i;
    i.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    i.SPI_Mode = SPI_Mode_Master;
    i.SPI_DataSize = SPI_DataSize_8b;
    i.SPI_CPOL = SPI_CPOL_Low;
    i.SPI_CPHA = SPI_CPHA_1Edge;
    i.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    i.SPI_NSS = SPI_NSS_Soft;
    i.SPI_FirstBit = SPI_FirstBit_MSB;

    SPI_Init(SPI1, &i);
    SPI_Cmd(SPI1, ENABLE);

    flash_cs1;

    if (sizeof(struct flash_user_info) > FLASH_PAGE_SZ)
        return 1;

    if (sizeof(struct flash_device_info) > FLASH_PAGE_SZ)
        return 2;

    return 0;
}

/////////////////
// device info //
/////////////////

/**
 * @brief Read device info from flash
 * @return 0 if magic is ok.
 */
uint32_t flash_device_info_get(struct flash_device_info *flash_device) {
	flash_read(0, (uint8_t *)flash_device, sizeof(struct flash_device_info));
	
	// Check if content is valid
    return flash_device->magic != FLASH_MAGIC;
}

/**
 * @brief Program device info to flash
 * @return 1 if flash is not erased. 0 if ok.
 */
uint32_t flash_device_info_set(struct flash_device_info *flash_device) {
    flash_erase_sector(0);

	if (flash_check_erasure(0, sizeof(struct flash_device_info))) {
		// Flash was not erased
		return 1;
	}
	
	// Set magic const
	flash_device->magic = FLASH_MAGIC;

	// Program whole sector
	flash_prog(0, (uint8_t *)flash_device, sizeof(struct flash_device_info));
	
	return 0;
}

///////////////
// user info //
///////////////

uint8_t flash_user_invalid[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

uint32_t flash_user_get_addr(uint32_t user_index) {
    return FLASH_SECTOR_SZ + FLASH_SECTOR_SZ * user_index;
}

/**
 * @brief Get user count from flash
 * @return Number of users found in flash.
 */
uint32_t flash_user_count() {
    uint32_t i = 0;
	uint8_t cur[16];

	while (1) {
        if (i > FLASH_USERS_MAX)
            return FLASH_USERS_MAX;

		uint32_t addr = flash_user_get_addr(i);
		flash_read(addr, cur, 16);
		
		if (!memcmp(cur, flash_user_invalid, 16)) {
			return i;
		}
		
		i++;
	}
}

/**
 * @brief Read by number from flash
 * @return 1 in case of error. 0 otherwise
 */
uint32_t flash_user_get_at(uint32_t i, struct flash_user_info *flash_user) {
    uint32_t addr = flash_user_get_addr(i);

    // Basic check
    if (i > FLASH_USERS_MAX)
        return 1;

    // Read user info
    flash_read(addr, (uint8_t *)flash_user, sizeof(struct flash_user_info));

    // Check if user is valid
    if (!memcmp(flash_user->user, flash_user_invalid, 16))
        return 2;
    
    return 0;
}

/**
 * @brief Get user by name from flash
 * @param user - uuid of user to be found
 * @param flash_user - pointer to store user info in. Can be NULL
 * @param n - pointer to store index of found user. Can be NULL. Will not be set if user was not found
 * @return 1 if user was not found. 0 if ok
 */
uint32_t flash_user_search(uint8_t *user, struct flash_user_info *flash_user, uint32_t *n) {
    uint32_t i = 0;
	uint8_t cur[16];
	
	while (1) {
		uint32_t addr = flash_user_get_addr(i);
		flash_read(addr, cur, 16);
		
		if (!memcmp(cur, user, 16)) {
            if (flash_user)
			    flash_read(addr, (uint8_t *)flash_user, sizeof(struct flash_user_info));

            if (n)
                *n = i;
			return 0;
		}
		
		if (!memcmp(cur, flash_user_invalid, 16)) {
			return 1; // Not found
		}
		
		i++;
	}
}

/**
 * @brief Program user to flash
 * return 1 if user limit excedeed. 2 if flash was not erased. 0 if ok
 */
uint32_t flash_user_add(struct flash_user_info *flash_user) {
	int32_t n = flash_user_count();

    if (n >= FLASH_USERS_MAX) {
        return 1;
    }

    uint32_t addr = flash_user_get_addr(n);

    if (flash_check_erasure(addr, sizeof(struct flash_user_info))) {
        return 2;
    }

    flash_prog(addr, (uint8_t *)flash_user, sizeof(struct flash_user_info));

    return 0;
}

uint32_t flash_user_overwrite(struct flash_user_info *flash_user, uint32_t user_index) {
    uint32_t addr = flash_user_get_addr(user_index);

    // Erase whole sector
    flash_erase_sector(addr);

    // Check if erased
    if (flash_check_erasure(addr, FLASH_SECTOR_SZ))
        return 1;

    // Program new user info
    flash_prog(addr, (uint8_t *)flash_user, sizeof(struct flash_user_info));

    return 0;
}

uint32_t flash_user_block(uint8_t *user) {
	uint32_t i = 0;
	uint8_t cur[16];
	
	while (1) {
		uint32_t addr = flash_user_get_addr(i);
		flash_read(addr, cur, 16);
		
		if (!memcmp(cur, user, 16)) {
			// Program 
			uint8_t new = FLASH_USER_BLOCKED;
			flash_prog(addr + 48 + 32, &new, 1);
			return 0;
		}
		
		if (!memcmp(cur, flash_user_invalid, 16)) {
			return 1; // Not found
		}
		
		i++;
	}
}

#if CFG_DEBUG_ENABLE
struct flash_device_info flash_report_dev;
struct flash_user_info flash_report_user;

void dbg_flash_report() {
    if (flash_device_info_get(&flash_report_dev)) {
        dbg_printf("flash magic missmatch. abort.\n");
        return;
    }

    // report device info
    dbg_print_array("dev pub", flash_report_dev.public_key, 32);
    dbg_print_array("dev prv", flash_report_dev.private_key, 64);
    dbg_printf("dev pwd: %d\n", flash_report_dev.passkey);
    dbg_printf("dev nam: %s\n", flash_report_dev.name);

    // report users info
    uint32_t u = flash_user_count();

    dbg_printf("users: %u\n", u);

    uint32_t i;

    for (i = 0; i < u; i++) {
        flash_user_get_at(i, &flash_report_user);
        dbg_printf("#%u\n", i);
        dbg_print_array(" usr uid", flash_report_user.user, 16);
        dbg_print_array(" usr pub", flash_report_user.public_key, 32);
        dbg_printf(" usr sts: %02X\n", flash_report_user.state);
    }
}
#endif