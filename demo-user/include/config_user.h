/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : config_user.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __CONFIG_USER_H__
#define __CONFIG_USER_H__

/**************************************************************************************
* Description    : 设备驱动ID定义列表, ID必须大于0, 0作为保留字段
**************************************************************************************/
#define INIT_PID                     100
#define GPIO_PID                     1
#define ADC_PID                      2
#define GPS_PID                      3
#define CAN_PID                      4
#define ICCARD_PID                   5
#define LCM_PID                      6
#define PRINT_PID                    7
#define SENSOR_PID                   8
#define VEHIC_PID                    9
#define REBOOT_PID                   10
#define DFLASH_PID                   11
#define UPGRADE_PID                  12
#define VERSION_PID                  13
#define RTC_PID                      14
#define UART_PID                     15
#define LOWPOWER_PID                 16

struct piddata {
    uint8_t id;                           // id数据类型
    uint32_t len;                         // 数据长度
    uint8_t data[0];                      // 数据内容
} __packed;

#endif /* __CONFIG_USER_H__ */

