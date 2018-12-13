/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : st7567_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __ST7567_DRIVER_H__
#define __ST7567_DRIVER_H__

#include "frtos_types.h"
#include "s32k144.h"
#include "frtos_delay.h"
#include "gpio_driver.h"
#include "frtos_ioctl.h"

/**************************************************************************************
* Description    : 设备信息配置
**************************************************************************************/
#define ST7567_WIDE_MAX         128                          // 点阵最大宽度
#define ST7567_HIGH_MAX         32                           // 点阵最大高度
#define ST7567_PAGE_MAX         8                            // 最大页地址
#define ST7567_BUF_MAX          ((ST7567_WIDE_MAX)*(ST7567_HIGH_MAX)/8)

/**************************************************************************************
* Description    : 设备IO端口配置
**************************************************************************************/
#define ST7567_GPIO_PWR         PTC,15                      // 电源IO
#define ST7567_GPIO_VLED        PTC,14                      // 背光IO
#define ST7567_GPIO_RST         PTC,12                      // 复位IO
#define ST7567_GPIO_CS          PTC,13                      // 片选IO
#define ST7567_GPIO_RS          PTC,11                      // 发送命令/数据选择IO
#define ST7567_GPIO_CLK         PTC,10                      // 时钟IO
#define ST7567_GPIO_SID         PTC,9                       // 数据IO
#define ST7567_GPIO_K1          PTE,7                       // 按键1
#define ST7567_GPIO_K2          PTA,6                       // 按键2
#define ST7567_GPIO_K3          PTA,7                       // 按键3
#define ST7567_GPIO_K4          PTC,8                       // 按键4

/**************************************************************************************
* Description    : 设备宏数据定义
**************************************************************************************/
#define ST7567_HIGH             1                           // 高电平
#define ST7567_LOW              0                           // 低电平
#define ST7567_ON               ST7567_HIGH                 // 开
#define ST7567_OFF              ST7567_LOW                  // 关

/**************************************************************************************
* Description    : 设备地址数据结构
**************************************************************************************/
struct st7567_addr_s{
    uint8_t page;                                           // 页地址
    uint8_t column;                                         // 行地址
};

/**************************************************************************************
* Description    : 点阵数据结构
**************************************************************************************/
struct st7567_dot_s{
    struct st7567_addr_s addr;                              // 地址
    uint8_t wide;                                           // 宽度
    uint8_t high;                                           // 高度
    uint16_t count;                                         // 字节数
    uint8_t data[ST7567_BUF_MAX];                           // 数据
}__attribute__((packed));

/**************************************************************************************
* FunctionName   : st7567_delay_ns()
* Description    : 纳秒延时函数
* EntryParameter : ns,延时纳秒
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_delay_ns(uint16_t ns)
{
    frtos_delay_ns(ns);
}

/**************************************************************************************
* FunctionName   : st7567_delay_ns()
* Description    : 毫秒延时函数
* EntryParameter : ns,延时毫秒
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_delay_ms(uint16_t ms)
{
    frtos_delay_ms(ms);
}

/**************************************************************************************
* FunctionName   : st7567_gpio_dir_in()
* Description    : 设置GPIO输入
* EntryParameter : port,GPIO端口, pin,管脚
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_gpio_dir_in(gpio_port port, uint8_t pin)
{
    GPIO_HAL_SetPinDirection(port, pin, 0);
}

/**************************************************************************************
* FunctionName   : st7567_gpio_dir_out()
* Description    : 设置GPIO输出
* EntryParameter : port,GPIO端口, pin,管脚, val值
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_gpio_dir_out(gpio_port port, uint8_t pin, uint8_t val)
{
    GPIO_HAL_SetPinDirection(port, pin, 1);
    GPIO_HAL_WritePin(port, pin, val);
}

/**************************************************************************************
* FunctionName   : st7567_gpio_val_set()
* Description    : 设置GPIO值
* EntryParameter : port,GPIO端口, pin,管脚, val值
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_gpio_val_set(gpio_port port, uint8_t pin, uint8_t val)
{
    GPIO_HAL_WritePin(port, pin, val);
}

/**************************************************************************************
* FunctionName   : st7567_ctrl_pwr()
* Description    : 电源控制
* EntryParameter : ctrl,控制码(ST7567_ON, ST7567_OFF)
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_ctrl_pwr(uint8_t ctrl)
{
    if(ST7567_ON != ctrl && ST7567_OFF != ctrl)return;
    st7567_gpio_val_set(ST7567_GPIO_PWR, ctrl);
}

/**************************************************************************************
* FunctionName   : st7567_ctrl_led()
* Description    : 背光控制
* EntryParameter : ctrl,控制码(ST7567_ON, ST7567_OFF)
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_ctrl_led(uint8_t ctrl)
{
    if(ST7567_ON != ctrl && ST7567_OFF != ctrl)return;
    st7567_gpio_val_set(ST7567_GPIO_VLED, ctrl);
}

/**************************************************************************************
* FunctionName   : st7567_reset()
* Description    : 显示器复位
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_reset(void)
{
    st7567_gpio_val_set(ST7567_GPIO_RST, ST7567_LOW);
    st7567_delay_ms(1);
    st7567_gpio_val_set(ST7567_GPIO_RST, ST7567_HIGH);
    st7567_delay_ms(1);
}

/**************************************************************************************
* FunctionName   : st7567_write_dot()
* Description    : 写点阵数据
* EntryParameter : dot,要显示的点阵数据
* ReturnValue    : 0,成功, -1,失败
**************************************************************************************/
int8_t st7567_write_dot(const struct st7567_dot_s *dot);

/**************************************************************************************
* FunctionName   : st7567_screen_clear()
* Description    : 显示清屏
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void st7567_screen_clear(void);

#endif /* __ST7567_DRIVER_H__ */

