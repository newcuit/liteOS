/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : gpio_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __GPIO_DRIVER_H__
#define __GPIO_DRIVER_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"
#include "components.h"
#include "pal.h"

/**************************************************************************************
* Description    : GPIO端口数据类型
**************************************************************************************/
typedef uint8_t gpio_port;
#define SPC_MIN_GPIO                0           // SPC560B54L5 最小GPIO号
#define SPC_MAX_GPIO                122         // SPC560B54L5 最大GPIO号

/**************************************************************************************
* Description    : GPIO控制参数结构
**************************************************************************************/
struct gpio_args_s{
    gpio_port port;                             // GPIO端口
    uint8_t pin;                                // GPIO端口引脚编号
    uint8_t val;                                // GPIO端口值
};

/**************************************************************************************
* FunctionName   : gpio_pin_set()
* Description    : 设置GPIO端口
* EntryParameter : port,GPIO端口, pin,端口引脚编号, val,值(0,1)
* ReturnValue    : None
**************************************************************************************/
static inline void gpio_pin_set(gpio_port port, uint8_t pin, uint8_t val)
{
    // 1.设置端口值
    pal_writepad(port, pin, val);
}

/**************************************************************************************
* FunctionName   : gpio_pin_set()
* Description    : 设置GPIO端口
* EntryParameter : port,GPIO端口, pin,端口引脚编号
* ReturnValue    : 返回端口值(0,1)
**************************************************************************************/
static inline uint8_t gpio_pin_get(gpio_port port, uint8_t pin)
{
    // 1.读取端口值
    return pal_readpad(port, pin) == 1 ? 1 : 0;
}

#endif

