/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : config_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __CONFIG_DRIVER_H__
#define __CONFIG_DRIVER_H__

/**************************************************************************************
* Description    : 设备驱动ID定义列表, ID必须大于0, 0作为保留字段
**************************************************************************************/
#define DRIVER_ADC                  1
#define DRIVER_PRINT                2
#define DRIVER_LCD                  3
#define DRIVER_I2C                  4
#define DRIVER_GPIO                 5
#define DRIVER_SENSOR               6
#define DRIVER_RTC                  7
#define DRIVER_PM                   8
#define DRIVER_UART0                9
#define DRIVER_UART1                10
#define DRIVER_UART2                11
#define DRIVER_UART3                12
#define DRIVER_UART4                13
#define DRIVER_UART5                14
#define DRIVER_UART6                15
#define DRIVER_UART7                16
#define DRIVER_CAN0                 17
#define DRIVER_CAN1                 18
#define DRIVER_CAN2                 19
#define DRIVER_CAN3                 20
#define DRIVER_CAN4                 21
#define DRIVER_CAN5                 22
#define DRIVER_CAN6                 23
#define DRIVER_PIT0                 24
#define DRIVER_PIT1                 25
#define DRIVER_PIT2                 26
#define DRIVER_GPIOINT              27
#define DRIVER_PFLASH               28
#define DRIVER_DFLASH               29
#define DRIVER_ICU                  30

#define IRQ_FUNCTIONS(id) void id(void)

/**************************************************************************************
* Description    : 定义安装中断服务函数
**************************************************************************************/
#define SETUP_IRQ(irq, callback) IRQ_FUNCTIONS(vector##irq) \
{\
    callback();\
}

/**************************************************************************************
* Description    : 定义安装CORE中断服务函数
**************************************************************************************/
#define SETUP_CORE_IRQ(irq, callback) IRQ_FUNCTIONS(_IVOR##irq) \
{\
    callback();\
}

#endif /* __CONFIG_DRIVER_H__ */

