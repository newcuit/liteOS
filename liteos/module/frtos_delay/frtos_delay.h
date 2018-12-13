/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_delay.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_DELAY_H__
#define __FRTOS_DELAY_H__

#include "frtos_types.h"
#include "frtos_delay.h"

/**************************************************************************************
* FunctionName   : frtos_delay_ns()
* Description    : 纳秒延时函数
* EntryParameter : ns,延时纳秒
* ReturnValue    : None
**************************************************************************************/
static inline void frtos_delay_ns(uint16_t ns)
{
    uint16_t i = 0;

    for(i = 0; i < ns; i++) __barrier();
}

#ifdef USING_OS_FREERTOS
/**************************************************************************************
* FunctionName   : frtos_delay_ms()
* Description    : 毫秒延时函数
* EntryParameter : ms,延时毫秒
* ReturnValue    : None
**************************************************************************************/
static inline void frtos_delay_ms(uint16_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/**************************************************************************************
* FunctionName   : frtos_get_tick()
* Description    : 获取当前tick数据
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline uint32_t frtos_get_tick(void)
{
    return xTaskGetTickCount();
}
#else
/**************************************************************************************
* FunctionName   : frtos_delay_ms()
* Description    : 毫秒延时函数
* EntryParameter : ms,延时毫秒
* ReturnValue    : None
**************************************************************************************/
static inline void frtos_delay_ms(uint16_t ms)
{
    frtos_delay_ns(1000*ms);
}

/**************************************************************************************
* FunctionName   : frtos_get_tick()
* Description    : 获取当前tick数据
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline uint32_t frtos_get_tick(void)
{
    return 0;
}
#endif

#endif /*__FRTOS_DELAY_H__ */

