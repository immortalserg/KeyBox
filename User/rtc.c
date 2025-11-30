#include "rtc.h"

// Get time. 32bit UNIX time
int32_t rtc_get() {
    return RTC_GetCounter();
}

// Set time. 32bit UNIX time
void rtc_set(int32_t tim) {
    RTC_WaitForLastTask();
    RTC_SetCounter(tim);
    
    BKP_WriteBackupRegister(BKP_DR1, RTC_CHECK);

    dbg_printf("rtc set %d\n", tim);
}

// Init rtc.
// Check if BKP magic is set. If not then set RTC to 0
uint32_t rtc_init() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    if (BKP_ReadBackupRegister(BKP_DR1) == RTC_CHECK) {
        return 1;
    }

    // Why DeInit?
    BKP_DeInit();
    RCC_LSEConfig(RCC_LSE_ON);

    while (!RCC_GetFlagStatus(RCC_FLAG_LSERDY));

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);

    RTC_WaitForLastTask();
    RTC_SetPrescaler(32767);

    rtc_set(RTC_DEFAULT_TIME);

    dbg_printf("rtc default init\n");

    return 0;
}
