/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : rtc.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __RTC_H__
#define __RTC_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"
#include "clock.h"
#include "osal.h"
#include "irq.h"
#include "xpc560b.h"

/**************************************************************************************
* Description    : 定义RTC时钟
**************************************************************************************/
#define RTC_SXOSC_CLK                           0U
#define RTC_SIRC_CLK                            1U
#define RTC_FIRC_CLK                            2U

/**************************************************************************************
* Description    : 定义RTC时钟分频
**************************************************************************************/
#define RTC_512_DIVIDE                          1U
#define RTC_32_DIVIDE                           1U

#endif /* __RTC_H__ */

