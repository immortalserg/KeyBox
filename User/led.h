#ifndef __LED_H__
#define __LED_H__

#include "ch32v30x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_COL_OFF 0
#define LED_COL_RED 1
#define LED_COL_BLUE 4
#define LED_COL_GREEN 2
#define LED_COL_CYAN (LED_COL_BLUE | LED_COL_GREEN)
#define LED_COL_YELLOW (LED_COL_RED | LED_COL_GREEN)
#define LED_COL_MAGENTA (LED_COL_RED | LED_COL_BLUE)
#define LED_COL_WHITE (LED_COL_RED | LED_COL_GREEN | LED_COL_BLUE)

#define LED_STATE_IDLE      0
#define LED_STATE_CONNECTED 1
#define LED_STATE_PRESS     2
#define LED_STATE_ERASE     3

#define LED_TASK_PRIO 5
#define LED_TASK_STK  256

// Must be called only without OS
void led_set(uint32_t col);

// Must be called only from OS
void led_set_state(uint8_t state);

uint32_t led_init();

#endif
