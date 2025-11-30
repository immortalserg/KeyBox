/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ed25519.h"
#include "led.h"
#include "keyfob.h"
#include "button.h"
#include "ble.h"
#include "rtc.h"
#include "flash.h"
#include "rng.h"
#include "cfg.h"
#include "rs485.h"
#include "monocypher.h"

#define APP_VER 0x01

struct __attribute__((packed)) app_req {
	uint8_t ver;
    uint16_t serial;
	uint8_t user[16];       // §Ú§Õ§Ö§ß§ä§Ú§æ§Ú§Ü§Ñ§ä§à§â §á§à§Ý§î§Ù§à§Ó§Ñ§ä§Ö§Ý§ñ
	int32_t time;           // timestamp §Ü§Ý§Ú§Ö§ß§ä§Ñ
	uint8_t command;        // APP_CMD_* §Ü§à§Õ §Ü§à§Þ§Ñ§ß§Õ§í
	uint8_t payload_sz;
	uint8_t payload[151];   // §Õ§Ý§ñ §Ü§à§Þ§Ñ§ß§Õ§í §á§Ö§â§Ó§í§Ö 4 §Ò§Ñ§Û§ä§Ñ §ï§ä§à nonce
	uint8_t signature[64];
};

struct __attribute__((packed)) app_reply {
	uint8_t ver;
	int32_t time;
	uint8_t command; // APP_CMD_*
	uint8_t code; // APP_ERR_*
	uint8_t payload_sz;
	uint8_t payload[153];
	uint8_t signature[64];
};

// Raw input from BLE
#define APP_RAW_SZ BLE_DATA_RX_SZ
uint8_t app_raw[APP_RAW_SZ];

// Parsed data
struct app_req app_in;
struct app_reply app_out;

// ----- nonce
uint8_t app_expected_nonce[4];
uint8_t app_nonce_active = 0;
// -----
#define APP_ERR_OK                     0
#define APP_ERR_WRONG_SERIAL           1
#define APP_ERR_WRONG_VER              2
#define APP_ERR_UNKNOWN_USER           3
#define APP_ERR_SIGNATURE_CHECK_FAILED 4
#define APP_ERR_UNKNOWN_COMMAND        5
#define APP_ERR_PAYLOAD_WRONG_SIZE     6
#define APP_ERR_USER_ALREADY_ADDED     7
#define APP_ERR_FLASH_FAILURE          8
#define APP_ERR_EXPIRED                9
#define APP_ERR_USER_BLOCKED           10
#define APP_ERR_USER_NAME_INVALID      11
#define APP_ERR_NONCE_CHECK_FAILED     12 // ----- §à§ê§Ú§Ò§Ü§Ñ §á§â§à§Ó§Ö§â§Ü§Ú nonce

#define APP_CMD_HELLO          0
#define APP_CMD_ADD_FIRST_USER 1
#define APP_CMD_SECURE_HELLO   100
#define APP_CMD_CHANGE_NAME    101
#define APP_CMD_CHANGE_PASSKEY 102
#define APP_CMD_FACTORY_RESET  103
#define APP_CMD_USER_BLOCK     104
#define APP_CMD_ADD_USER       105
#define APP_CMD_SYNC_TIME      106
#define APP_CMD_GET_USER_COUNT 107
#define APP_CMD_GET_USER_INFO  108
#define APP_CMD_PROXY_ADD_USER 109
#define APP_CMD_RESET_REASON   110
#define APP_CMD_DEVICE_INFO    111
#define APP_CMD_PRESS          200
#define APP_CMD_RS_SEND        201
#define APP_CMD_RS_RECV        202


uint16_t app_get_serial() {
    // R32_ESIG_UNIID1
    uint32_t *sig = (uint32_t *)0x1FFFF7E8;

    return *sig;
}

#define APP_RESET_REASON_POWER (1 << 0)
#define APP_RESET_REASON_WDT   (1 << 1)
#define APP_RESET_REASON_SOFT  (1 << 2)
#define APP_RESET_REASON_HARD  (1 << 3)

uint8_t app_get_reset_reason() {
    uint32_t hw_reason = 0;
    static uint8_t reason = 0;

    // Reset reason flag can be obtained only once
    if (reason)
        return reason;

    // Get reason
    hw_reason = RCC->RSTSCKR;

    // Reset reason flag
    RCC->RSTSCKR = RCC_RMVF;

    // Parce reason
    if ((hw_reason & RCC_LPWRRSTF) || (hw_reason & RCC_PORRSTF))
        reason |= APP_RESET_REASON_POWER;

    if ((hw_reason & RCC_WWDGRSTF) || (hw_reason & RCC_IWDGRSTF))
        reason |= APP_RESET_REASON_WDT;

    if (hw_reason & RCC_SFTRSTF)
        reason |= APP_RESET_REASON_SOFT;

    if (hw_reason & RCC_PINRSTF)
        reason |= APP_RESET_REASON_HARD;

    return reason;
}

// Generate reply with code and payload
uint32_t app_out_generate(uint8_t code, uint8_t *payload, uint8_t payload_sz) {
    app_out.ver = APP_VER;
    app_out.time = rtc_get();
    app_out.command = app_in.command;
    app_out.code = code;
    app_out.payload_sz = payload_sz;
    memcpy(app_out.payload, payload, payload_sz);

    return code;
}

// Generate reply with code only
uint32_t app_out_generate_short(uint8_t code) {
    return app_out_generate(code, NULL, 0);
}

struct flash_device_info app_dev_info;
struct flash_user_info app_user_info;

// Regular cmd active time
#define APP_TIME_DELTA_MAX (60 * 5)
// (Proxy) add user cmd active time
#define APP_TIME_DELTA_PROXY_MAX (60 * 60 * 24)

uint32_t rtc_check_range(int32_t remote, int32_t r) {
    int32_t local = rtc_get();

    if (((local - r) < remote) && (remote < (local + r)))
        return 0;
    
    return 1;
}

uint8_t app_rs_buff[RS_RX_SZ];

uint32_t app_check_new_name(uint8_t *name) {
    uint32_t sz = strnlen((const char *)name, 32);

    if (sz < 3)
        return 1;

    if (sz > 31)
        return 2;

    return 0;
}

const uint8_t app_ver[] = "Device: BTA320 REV.A" "\n" "Build: " __DATE__ " " __TIME__;

// ----- §æ§å§ß§Ü§è§Ú§ñ §Ô§Ö§ß§Ö§â§Ñ§è§Ú§Ú nonce
void app_generate_nonce(void) {
    rng_get((uint32_t *)app_expected_nonce, 1); // 4 §Ò§Ñ§Û§ä§Ñ
    app_nonce_active = 1;
}
// -----

uint32_t app_in_logic() {
    memcpy(&app_in, app_raw, sizeof(struct app_req));

    dbg_printf("*** req ***\n");
    dbg_printf(" ver: %u\n", app_in.ver);
    dbg_printf(" ser: %04X\n", app_in.serial);
    dbg_print_array(" usr", app_in.user, 16);
    dbg_printf(" tim: %d\n", app_in.time);
    dbg_printf(" cmd: %u\n", app_in.command);
    dbg_print_array(" pld", app_in.payload, app_in.payload_sz);
    dbg_print_array(" sig", app_in.signature, 64);
    dbg_printf("***********\n");

    // Check version
    if (app_in.ver != APP_VER)
        return app_out_generate_short(APP_ERR_WRONG_VER);

    // Check payload size
    if (app_in.payload_sz > sizeof((struct app_req) { 0 }.payload))
        return app_out_generate_short(APP_ERR_PAYLOAD_WRONG_SIZE);

    // Check if this req is for us
    if (app_in.serial != app_get_serial())
        return app_out_generate_short(APP_ERR_WRONG_SERIAL);

    int32_t tim = rtc_get();

    // Check if req is too old
    if (tim > 0 && rtc_check_range(app_in.time, app_in.command == APP_CMD_ADD_USER ? APP_TIME_DELTA_PROXY_MAX : APP_TIME_DELTA_MAX))
        return app_out_generate_short(APP_ERR_EXPIRED);

    // Check if user is blocked
    if (!flash_user_search(app_in.user, &app_user_info, NULL)) {
        if (app_user_info.state == FLASH_USER_BLOCKED) {
            return app_out_generate_short(APP_ERR_USER_BLOCKED);
        }
    }

    if (app_in.command >= 100) {
        // This is secure command

        // Search for user
        uint32_t err = flash_user_search(app_in.user, &app_user_info, NULL);

        // Fail, if user was not found
        if (err)
            return app_out_generate_short(APP_ERR_UNKNOWN_USER);
// ----- §Õ§à§Ò§Ñ§Ó§Ý§ñ§Ö§Þ §á§â§à§Ó§Ö§â§Ü§å nonce §Ú §ã§Ò§â§à§ã nonce
        if (app_in.command != APP_CMD_SECURE_HELLO) {
            if (!app_nonce_active || app_in.payload_sz < 4)
                return app_out_generate_short(APP_ERR_NONCE_CHECK_FAILED);
            if (memcmp(app_in.payload, app_expected_nonce, 4) != 0)
                return app_out_generate_short(APP_ERR_NONCE_CHECK_FAILED);
            memset(app_expected_nonce, 0, 4);
            app_nonce_active = 0;
        }
// -----
        if (app_in.command == APP_CMD_PROXY_ADD_USER) {
            // Reset proxy payload for correct verification
            memset(app_raw + 25, 0, 16 + 32);
        }

        // Verify message
        if (!ed25519_verify(app_in.signature, app_raw, sizeof(struct  app_req) - 64, app_user_info.public_key))
            return app_out_generate_short(APP_ERR_SIGNATURE_CHECK_FAILED);
        
    }

    uint8_t tmp;

    switch (app_in.command) {
    case APP_CMD_HELLO:
    case APP_CMD_SECURE_HELLO:
// ----- §Õ§à§Ò§Ñ§Ó§Ý§Ö§ß§Ú§Ö §à§Ò§â§Ñ§Ò§à§ä§Ü§Ú nonce §Ó §à§ä§Ó§Ö§ä APP_CMD_SECURE_HELLO
        app_generate_nonce();
        return app_out_generate(APP_ERR_OK, app_expected_nonce, 4);
        return app_out_generate_short(APP_ERR_OK);
// -----
    case APP_CMD_DEVICE_INFO:
        return app_out_generate(APP_ERR_OK, (uint8_t *)app_ver, strlen((const char *)app_ver));

    case APP_CMD_RESET_REASON:
        tmp = app_get_reset_reason();
        return app_out_generate(APP_ERR_OK, &tmp, 1);

    case APP_CMD_SYNC_TIME:
        rtc_set(app_in.time);
        return app_out_generate_short(APP_ERR_OK);

    case APP_CMD_ADD_FIRST_USER:
        if (flash_user_count())
            return app_out_generate_short(APP_ERR_USER_ALREADY_ADDED);

        if (app_in.payload_sz != 64)
            return app_out_generate_short(APP_ERR_PAYLOAD_WRONG_SIZE);

        if (app_check_new_name(app_in.payload + 32))
            return app_out_generate_short(APP_ERR_USER_NAME_INVALID);

        memcpy(app_user_info.user, app_in.user, 16);
        memcpy(app_user_info.public_key, app_in.payload, 32);
        memcpy(app_user_info.name, app_in.payload + 32, 32);
        app_user_info.state = FLASH_USER_OKAY;
        
        if (flash_user_add(&app_user_info))
            return app_out_generate_short(APP_ERR_FLASH_FAILURE);

        // Change advert state now
        ble_set_advert(0);

        // If this is the first command after RTC battery install
        if (tim < 0) {
            rtc_set(app_in.time);
            tim = app_in.time;
        }

        // Reply with our public key
        return app_out_generate(APP_ERR_OK, app_dev_info.public_key, 32);

    case APP_CMD_ADD_USER:
    case APP_CMD_PROXY_ADD_USER:
        if (app_in.payload_sz != (16 + 32 + 32))
            return app_out_generate_short(APP_ERR_PAYLOAD_WRONG_SIZE);
        
        if (app_check_new_name(app_in.payload + 16 + 32))
            return app_out_generate_short(APP_ERR_USER_NAME_INVALID);

        memcpy(app_user_info.user, app_in.payload, 16);
        memcpy(app_user_info.public_key, app_in.payload + 16, 32);
        memcpy(app_user_info.name, app_in.payload + 32 + 16, 32);
        app_user_info.state = FLASH_USER_OKAY;

        // Check if user already exists
        uint32_t user_index = 0;
        if (!flash_user_search(app_in.payload, NULL, &user_index)) {
            if (flash_user_overwrite(&app_user_info, user_index))
                return app_out_generate_short(APP_ERR_FLASH_FAILURE);

        } else {
            if (flash_user_add(&app_user_info))
                return app_out_generate_short(APP_ERR_FLASH_FAILURE);

        }

        // Reply with our public key
        return app_out_generate(APP_ERR_OK, app_dev_info.public_key, 32);

    case APP_CMD_PRESS:
        // Check for payload size.
        // payload[0] - relay id mask
        // payload[1] - relay hold time
        if (app_in.payload_sz != 2)
            return app_out_generate_short(APP_ERR_PAYLOAD_WRONG_SIZE);
        
        keyfob_set(app_in.payload[0], app_in.payload[1]);
        led_set_state(LED_STATE_PRESS);

        return app_out_generate_short(APP_ERR_OK);

    case APP_CMD_RS_SEND:
        rs_send(app_in.payload, app_in.payload_sz);
        return app_out_generate_short(APP_ERR_OK);

    case APP_CMD_RS_RECV:
        xQueueReceive(rs_recv, app_rs_buff, 0);
        return app_out_generate(APP_ERR_OK, app_rs_buff + 1, app_rs_buff[0]);

    case APP_CMD_CHANGE_PASSKEY:
        // Check for payload size.
        // payload[0 .. 3] - new passkey for ble
        // uint32_t with range 000000 .. 999999
        if (app_in.payload_sz != 4)
            return app_out_generate_short(APP_ERR_PAYLOAD_WRONG_SIZE);

        // Copy
        memcpy(&app_dev_info.passkey, app_in.payload, 4);

        // Program to flash
        if (flash_device_info_set(&app_dev_info))
            return app_out_generate_short(APP_ERR_FLASH_FAILURE);

        // Change BLE passkey now
        ble_set_passkey(app_dev_info.passkey);

        return app_out_generate_short(APP_ERR_OK);

    case APP_CMD_GET_USER_COUNT:
        tmp = flash_user_count();
        return app_out_generate(APP_ERR_OK, &tmp, 1);

    case APP_CMD_GET_USER_INFO:
        // Check for payload size.
        if (app_in.payload_sz != 1)
            return app_out_generate_short(APP_ERR_PAYLOAD_WRONG_SIZE);

        // Search for user
        if (flash_user_get_at(app_in.payload[0], &app_user_info))
            return app_out_generate_short(APP_ERR_UNKNOWN_USER);

        return app_out_generate(APP_ERR_OK, (uint8_t *)&app_user_info, sizeof(struct flash_user_info));

    case APP_CMD_CHANGE_NAME:
        // Check payload size.
        // payload[0 .. 19] - new name.
        // null-terminated string
        if (app_in.payload_sz < 4 || app_in.payload_sz > 20)
            return app_out_generate_short(APP_ERR_PAYLOAD_WRONG_SIZE);

        // Copy
        memset(app_dev_info.name, 0, 32);
        strcpy((char *)app_dev_info.name, (char *)app_in.payload);

        // Program to flash
        if (flash_device_info_set(&app_dev_info))
            return app_out_generate_short(APP_ERR_FLASH_FAILURE);

        // Cheange BLE name now
        ble_set_name(app_dev_info.name);

        return app_out_generate_short(APP_ERR_OK);

    case APP_CMD_USER_BLOCK:
        if (app_in.payload_sz != 16)
            return app_out_generate_short(APP_ERR_PAYLOAD_WRONG_SIZE);

        // Block user now
        if (flash_user_block(app_in.payload))
            return app_out_generate_short(APP_ERR_UNKNOWN_USER);

        // Return blocked user id
        return app_out_generate(APP_ERR_OK, app_in.payload, 16);

    case APP_CMD_FACTORY_RESET:
        ble_factory_reset();

        return app_out_generate_short(APP_ERR_OK);

    default:
        // Unknown command
        return app_out_generate_short(APP_ERR_UNKNOWN_COMMAND);
    }
}



void app_out_reply() {

    ed25519_sign(app_out.signature, (uint8_t *)&app_out, sizeof(struct app_reply) - 64, app_dev_info.public_key, app_dev_info.private_key);
    memcpy(app_raw, &app_out, sizeof(struct app_reply));

    ble_data_send(app_raw, sizeof(struct app_reply));

    dbg_printf("*** reply ***\n");
    dbg_printf(" ver: %u\n", app_out.ver);
	dbg_printf(" tim: %d dt: %d\n", app_out.time, app_in.time - app_out.time);
	dbg_printf(" cmd: %u\n", app_out.command);
	dbg_printf(" cde: %u\n", app_out.code);
    dbg_print_array(" pld", app_out.payload, app_out.payload_sz);
    dbg_print_array(" sig", app_out.signature, 64);
    dbg_printf("*************\n");
}

void app_ble_config(uint32_t is_reset) {
    dbg_printf("*** ble setup ***\n");

    if (is_reset) {
        ble_factory_reset();
    }

    ble_set_passkey(app_dev_info.passkey);
    ble_set_name(app_dev_info.name);
    ble_set_advert(is_reset);
    ble_set_serial(app_get_serial());
    ble_continue();

    dbg_printf("*****************\n");
}

#define APP_ADD_DEBUG_USER 0

#if APP_ADD_DEBUG_USER
struct flash_user_info app_debug_user1_info = {
    .user = { 0x11, 0x18, 0xA2, 0xEF, 0x27, 0xC7, 0x4A, 0xC4, 0xB1, 0xA4, 0x91, 0xF7, 0x34, 0x3D, 0x22, 0x91 },
    .public_key = { 0x48, 0xc4, 0xb4, 0xff, 0x14, 0x22, 0x18, 0xae, 0x02, 0xd1, 0xc6, 0xb0, 0xe9, 0x19, 0x58, 0x2c, 0x9e, 0xfd, 0x51, 0xbb, 0x8c, 0xb6, 0x28, 0xb2, 0x29, 0x7c, 0x9a, 0x00, 0x70, 0x4e, 0x8f, 0x8c },
    .state = FLASH_USER_OKAY
};

struct flash_user_info app_debug_user2_info = {
    .user = { 0x22, 0x18, 0xA2, 0xEF, 0x27, 0xC7, 0x4A, 0xC4, 0xB1, 0xA4, 0x91, 0xF7, 0x34, 0x3D, 0x22, 0x92 },
    .public_key = { 0x48, 0xc4, 0xb4, 0xff, 0x14, 0x22, 0x18, 0xae, 0x02, 0xd1, 0xc6, 0xb0, 0xe9, 0x19, 0x58, 0x2c, 0x9e, 0xfd, 0x51, 0xbb, 0x8c, 0xb6, 0x28, 0xb2, 0x29, 0x7c, 0x9a, 0x00, 0x70, 0x4e, 0x8f, 0x8c },
    .state = FLASH_USER_OKAY
};
#endif

void app_factory_reset() {    
    // Generate new data
    uint8_t seed[32];

    led_set_state(LED_STATE_ERASE);
    dbg_printf("*** factory reset ***\n");

    rng_get((uint32_t *)seed, sizeof(seed) / sizeof(uint32_t));
    ed25519_create_keypair(app_dev_info.public_key, app_dev_info.private_key, seed);
    app_dev_info.passkey = CFG_FACTORY_PASSKEY;
    memset(app_dev_info.name, 0, 32);
    strcpy((char *)app_dev_info.name, CFG_FACTORY_NAME);

    dbg_print_array("seed", seed, sizeof(seed));
    dbg_print_array("pub", app_dev_info.public_key, 32);
    dbg_print_array("prv", app_dev_info.private_key, 64);
    dbg_printf("pwd: %d\n", app_dev_info.passkey);
    dbg_printf("name: %s\n", app_dev_info.name);

    // Program new data
    flash_erase_chip();
    flash_device_info_set(&app_dev_info);

#if APP_ADD_DEBUG_USER
    dbg_printf("=== clean ===\n");
    dbg_flash_report();
    dbg_printf("=== add users ===\n");
    flash_user_add(&app_debug_user1_info);
    flash_user_add(&app_debug_user2_info);
    dbg_flash_report();

    dbg_printf("=== block user 2 ===\n");
    flash_user_block(app_debug_user2_info.user);
    dbg_flash_report();

    dbg_printf("=== unblock user 2 ===\n");
    uint32_t blocked;
    flash_user_search(app_debug_user2_info.user, NULL, &blocked);
    flash_user_overwrite(&app_debug_user2_info, blocked);
    dbg_flash_report();
#endif

    // Reset RTC
    rtc_set(CFG_FACTORY_TIME);

    // Reset BLE
    app_ble_config(1);

    led_set_state(LED_STATE_IDLE);

    dbg_printf("*********************\n");
}

#define APP_PEREODIC_BLE_OVERVIEW 0

void app_task() {
    uint32_t status = 0;

    // Check flash storage integrity
    if (flash_device_info_get(&app_dev_info))
        app_factory_reset();

    // Check user count. If there is 0 users then switch to first user mode
    uint32_t users = flash_user_count();

    // Report device state for debug
    dbg_printf("*** device report ***\n");
    dbg_flash_report();
    dbg_printf("dev ser: %04X\n", app_get_serial());
    dbg_printf("dev tim: %d\n", rtc_get());
    dbg_printf("dev rst: %02X\n", app_get_reset_reason());
    dbg_printf("*********************\n");

    // Wait for ble to start
    vTaskDelay(100);

    // Apply BLE config
    app_ble_config(users == 0);

#if APP_PEREODIC_BLE_OVERVIEW
    uint32_t cnt = 0;
#endif

    while (1) {
        // Check if ble message awailable

        if (xQueueReceive(ble_data_recv, &app_raw, 100) == pdPASS) {
            app_in_logic();
            app_out_reply();

            // Clean buffer
            memset(app_raw, 0, APP_RAW_SZ);
        }

        // Check ble status
        status = ble_status_get();

        if (status & BLE_STATUS_UPDATED) {
            led_set_state((status & BLE_STATUS_CONNECTED) ? LED_STATE_CONNECTED : LED_STATE_IDLE);

            dbg_printf("ble conn -> %d\n", status & 1);
        }

        // Check button updates
        uint8_t btn;
        if (xQueueReceive(button_cmd, &btn, 0) == pdPASS) {
            if (btn == BUTTON_ACTION_LONG) {
                app_factory_reset();

                // Reset connection state
                status = 0;
            }
        }

#if APP_PEREODIC_BLE_OVERVIEW
        if (cnt++ == 50) {
            cnt = 0;
            ble_overview();
        }
#endif
        if (xQueueReceive(rs_recv, app_rs_buff, 0 == pdPASS)) {
            dbg_printf("rs sz: %u\n", app_rs_buff[0]);
            dbg_print_array("rs dat", app_rs_buff + 1, app_rs_buff[0]);
        }

    }

}

#define APP_TASK_PRIO 5
#define APP_TASK_STK  256 * 4

uint32_t app_init() {
    // Create task for cmd execution
    int32_t err = xTaskCreate((TaskFunction_t )app_task,
        (const char*    )"app",
        (uint16_t       )APP_TASK_STK,
        (void*          )NULL,
        (UBaseType_t    )APP_TASK_PRIO,
        (TaskHandle_t*  )NULL);

    return err != pdPASS;
}

#if CFG_WDT_ENABLE
// Setup wdt for 4sec
void wdt_init() {
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    IWDG_SetReload(4095); // 4095 MAX
    IWDG_ReloadCounter();
    IWDG_Enable();
}
#endif

void halt() {
    while (1) {
        led_set(LED_COL_RED);
        
    }
}

void init_check(uint32_t code, const char *name) {
    if (code) {
        dbg_printf("%s init error %lu\n", name, code);
        dbg_printf("halt\n");
        led_set(LED_COL_RED);
        while (1);
    } else {
        dbg_printf("%s init ok\n", name);
    }
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SystemCoreClockUpdate();

	dbg_init(115200);
    dbg_printf("\n\n");
    dbg_printf("*** bta320 x keybox ***\n");
	dbg_printf("freq: %d MHz\r\n", SystemCoreClock / 1000000);
	dbg_printf("FreeRTOS %s\r\n", tskKERNEL_VERSION_NUMBER);
    dbg_printf("build: " __DATE__ " " __TIME__ "\n");
    dbg_printf("***********************\n");

    rtc_init();
#if CFG_WDT_ENABLE
    // Led task should reset the counter.
    wdt_init();
    dbg_printf("wdt enabled\n");
#endif

	rng_init();

    init_check(led_init(), "led");
    init_check(keyfob_init(), "keyfob");
    init_check(button_init(), "button");
    init_check(ble_init(), "ble");
    init_check(app_init(), "app");
    init_check(flash_init(), "flash");
    init_check(rs_init(), "rs");

    vTaskStartScheduler();

	while (1);
}

