#ifndef __RTC_H__
#define __RTC_H__

#include "debug.h"
#include "ch32v30x.h"
#include "cfg.h"

// Time delta
#define RTC_DELTA (60 * 60 * 24) // 24h

#define RTC_CHECK 0x55AA
#define RTC_DEFAULT_TIME CFG_FACTORY_TIME

uint32_t rtc_init();
void rtc_set(int32_t tim);
int32_t rtc_get();

#endif
