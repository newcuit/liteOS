/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : tp701.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __TP701_H__
#define __TP701_H__

#include "frtos_types.h"
#include "frtos_delay.h"
#include "s32k144.h"
#include "gpio_driver.h"

/**************************************************************************************
* Description    : 打印机行列定义
**************************************************************************************/
#define TP701_ROW 24                                // 打印机字库数据行大小
#define TP701_COL 48                                // 打印机字库数据列大小
#define TP701_MSG_SIZE (TP701_ROW * TP701_COL)      // 打印机字库数据总尺寸

/**************************************************************************************
* Description    : 打印机设备管脚定义
**************************************************************************************/
#define TP701_HC373                       PTB,5
#define TP701_PRT_WE1                     PTD,8
#define TP701_STB12                       PTD,6
#define TP701_STB34                       PTD,7
#define TP701_STB56                       PTB,4
#define TP701_PDATA                       PTD,9       // print_d4
#define TP701_PCLK                        PTC,0       // print_d5
#define TP701_MTA_IN                      PTD,10      // print_d0
#define TP701_MTAIN                       PTD,11      // print_d1
#define TP701_MTB_IN                      PTD,12      // print_d2
#define TP701_MTBIN                       PTD,5       // print_d3
#define TP701_PLAT                        PTC,1       // print_d6

/**************************************************************************************
* FunctionName   : tp701_delay_ns()
* Description    : 纳秒延时函数
* EntryParameter : ns,延时纳秒
* ReturnValue    : None
**************************************************************************************/
static inline void tp701_delay_ns(uint16_t ns)
{
    frtos_delay_ns(ns);
}

/**************************************************************************************
* FunctionName   : tp701_delay_ms()
* Description    : 毫秒延时函数
* EntryParameter : ns,延时毫秒
* ReturnValue    : None
**************************************************************************************/
static inline void tp701_delay_ms(uint16_t ms)
{
    frtos_delay_ms(ms);
}

/**************************************************************************************
* FunctionName   : motor_relax()
* Description    : 步进电机延时函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline void motor_relax(void)
{
    tp701_delay_ns(1700);
}

/**************************************************************************************
* FunctionName   : trans_relax()
* Description    : 打印传输延时函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline void trans_relax(void)
{
    tp701_delay_ns(5100);
}
#endif
