/********************************** (C) COPYRIGHT  *******************************
* File Name          : debug.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file contains all the functions prototypes for UART
*                      Printf , Delay functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H

#include "stdio.h"
#include "ch32v30x.h"
#include "ch32v30x_rng.h"
#include "cfg.h"

#if CFG_DEBUG_ENABLE
// Debug enabled
#define DEBUG   DEBUG_UART1
#define dbg_printf(...) printf(__VA_ARGS__)
void dbg_init(uint32_t baudrate);
void dbg_print_array(char *title, uint8_t *arr, uint32_t len);
#else
// Debug disabled
#define DEBUG   DEBUG_OFF
#define dbg_printf(...)
#define dbg_init(a)
#define dbg_print_array(a, b, c)
#endif

#endif 
