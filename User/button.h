#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "ch32v30x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define BUTTON_ACTION_SHORT 0
#define BUTTON_ACTION_LONG  1

extern QueueHandle_t button_cmd;

#define BUTTON_TASK_PRIO 5
#define BUTTON_TASK_STK  256

uint32_t button_init();

#endif
