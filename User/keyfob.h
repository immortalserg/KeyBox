#ifndef __KEYFOB_H__
#define __KEYFOB_H__

#include "ch32v30x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define KEYFOB_TASK_PRIO 5
#define KEYFOB_TASK_STK  256

uint32_t keyfob_init();
void keyfob_set(uint8_t relay, uint8_t hold);

#endif
