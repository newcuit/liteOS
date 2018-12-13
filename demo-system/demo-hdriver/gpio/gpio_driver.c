/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : gpio_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "frtos_gpio.h"
#include "gpio_driver.h"

/**************************************************************************************
* FunctionName   : gpio_read()
* Description    : 读
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static int32_t gpio_read(uint8_t idx, void *data, int32_t len)
{
    uint16_t pin;
    gpio_port port = 0;
    struct gpio *gpio = (struct gpio *)data;

    // 1.校验结构数据
    if(unlikely(len < (int32_t)sizeof(struct gpio) || NULL == data)) return -EINVAL;

    // 2.校验gpio有效性
    if(gpio->gpio > SPC_MAX_GPIO) {
        return -EINVAL;
    }

    // 3.计算GPIO组
    pin = gpio->gpio % 16;
    port = (gpio_port)(0 + (gpio->gpio / 16));

    (void)idx;
    gpio->value = gpio_pin_get(port, pin);
    return gpio->value;
}

/**************************************************************************************
* FunctionName   : gpio_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
**************************************************************************************/
static int32_t gpio_write(uint8_t idx, void *data, int32_t len)
{
    uint16_t pin;
    gpio_port port = 0;
    struct gpio *gpio = (struct gpio *)data;

    // 1.校验结构数据
    if(unlikely(len < (int32_t)sizeof(struct gpio) || NULL == data)) return -EINVAL;

    // 2.校验gpio有效性
    if(gpio->gpio > SPC_MAX_GPIO) {
        return -EINVAL;
    }

    // 3.计算GPIO组
    pin = gpio->gpio % 16;
    port = (gpio_port)(0 + (gpio->gpio / 16));

    gpio_pin_set(port, pin, gpio->value);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : gpio_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t gpio_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    struct gpio_args_s *gpio_args = args;

    if(unlikely(NULL == args || len != sizeof(struct gpio_args_s))){
        return -EINVAL;
    }

    switch(cmd){
    case _IOC_SET_DATA:
        gpio_pin_set(gpio_args->port, gpio_args->pin, gpio_args->val);
        break;
    case _IOC_GET_DATA:
        gpio_args->val = gpio_pin_get(gpio_args->port, gpio_args->pin);
        break;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : gpio_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init gpio_init(void)
{
    return 0;
}

static __const struct driver spc_gpio = {
    .idx   = DRIVER_GPIO,
    .name  = "gpio",
    .init  = gpio_init,
    .read  = gpio_read,
    .write = gpio_write,
    .ioctl = gpio_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(spc_gpio);
