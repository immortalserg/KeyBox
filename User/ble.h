#ifndef __BLE_H__
#define __BLE_H__

#include "ch32v30x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>
#include "cfg.h"

#define BLE_DATA_RECV_TASK_PRIO 5
#define BLE_DATA_RECV_TASK_STK  (256 * 2)

#define BLE_DATA_RX_SZ 240

uint32_t ble_set_name(uint8_t *name);
uint32_t ble_set_passkey(uint32_t passkey);
uint32_t ble_set_advert(uint32_t is_b);
uint32_t ble_factory_reset();
uint32_t ble_continue();
uint32_t ble_set_serial(uint16_t serial);
uint32_t ble_overview();

// Send array
void ble_data_send(uint8_t *dat, uint32_t sz);

// Send string
void ble_data_send_str(char *str);

// Recv array up to BLE_DATA_RX_SZ bytes
extern QueueHandle_t ble_data_recv;

uint32_t ble_init();

#define BLE_STATUS_UPDATED 2
#define BLE_STATUS_CONNECTED 1

uint32_t ble_status_get();

#endif
