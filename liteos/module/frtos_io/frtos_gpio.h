/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_gpio.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_GPIO_H__
#define __FRTOS_GPIO_H__

#include "frtos_types.h"

/**************************************************************************************
* Description    : 定义通用GPIO头部
**************************************************************************************/
struct gpio {
    uint32_t gpio;
    uint32_t value;
};

#endif /*__FRTOS_GPIO_H__ */

