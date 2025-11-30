#ifndef __RS485_H__
#define __RS485_H__

#include "debug.h"
#include "cfg.h"
#include "ch32v30x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define RS_RECV_TASK_PRIO 5
#define RS_RECV_TASK_STK  256

#define RS_RX_SZ 152

// Send array
void rs_send(uint8_t *buf, uint32_t size);

extern QueueHandle_t rs_recv;

uint32_t rs_init();

#endif
