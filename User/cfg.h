#ifndef __CFG_H__
#define __CFG_H__

#include <stdint.h>

// Enable debug messages to RS-485 (UART1). See debug.c/.h
#define CFG_DEBUG_ENABLE 0

#define CFG_WDT_ENABLE 1

// Factory settings
#define CFG_FACTORY_TIME INT32_MIN
#define CFG_FACTORY_NAME "keybox"
#define CFG_FACTORY_PASSKEY 123456

#endif
