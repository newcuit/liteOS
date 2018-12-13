/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : PCF85263.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __PCF85263_H__
#define __PCF85263_H__

#include "frtos_types.h"
#include "i2c_driver.h"
#include "frtos_ioctl.h"
#include "frtos_time.h"

/**************************************************************************************
* Description    : pcf85263 RTC寄存器
**************************************************************************************/
#define PCF85263_DEV_ADDR                0X51       // pcf85263设备地址

/**************************************************************************************
* Description    : pcf85263 RTC日期寄存器
**************************************************************************************/
#define PCF85263_100SECONDS              0x00
#define PCF85263_SECONDS                 0x01
#define PCF85263_MINUTES                 0x02
#define PCF85263_HOURS                   0x03
#define PCF85263_DAYS                    0x04
#define PCF85263_WEEKDAYS                0x05
#define PCF85263_MONTHS                  0x06
#define PCF85263_YEARS                   0x07

/**************************************************************************************
* Description    : pcf85263 RTC定时寄存器
**************************************************************************************/
#define PCF85263_SECOND_ALARM            0x08
#define PCF85263_MINUTE_ALARM            0x09
#define PCF85263_HOUR_ALARM              0x0A
#define PCF85263_DAY_ALARM               0x0B
#define PCF85263_MONTH_ALARM             0x0C

#define PCF85263_MINUTE_ALARM2           0x0D
#define PCF85263_HOUR_ALARM2             0x0E
#define PCF85263_WEEKDAY_ALARM2          0x0F

#define PCF85263_ALARM_ENABLE            0x10

/**************************************************************************************
* Description    : pcf85263 RTC控制寄存器
**************************************************************************************/
#define PCF85263_OFFSET                  0x24
#define PCF85263_OSCILLATOR              0x25
#define PCF85263_BATTERY_SW              0x26
#define PCF85263_PIN_IO                  0x27
#define PCF85263_CTRL_MODE               0x28
#define PCF85263_INTA_ENABLE             0x29
#define PCF85263_INTB_ENABLE             0x2A
#define PCF85263_FLAGS                   0x2B

#define PCF85263_RAM_BYTE                0x2C
#define PCF85263_WATCHDOG                0x2D
#define PCF85263_STOP_ENABLE             0x2E
#define PCF85263_RESETS                  0x2F

/**************************************************************************************
* Description    : 定义RTC对应寄存器值
**************************************************************************************/
#define PCF85263_SEC_A1E                 0x01
#define PCF85263_MIN_A1E                 0x02
#define PCF85263_HR_A1E                  0x04
#define PCF85263_DAY_A1E                 0x05
#define PCF85263_MON_A1E                 0x10
#define PCF85263_MIN_A2E                 0x20
#define PCF85263_HR_A2E                  0x40
#define PCF85263_WDAY_A2E                0x80

#define PCF85263_OSC_STOP                0x80
/**************************************************************************************
* Description    : 內核RTC日期时间数据类型
**************************************************************************************/
typedef time_date_t pcf85263_dtime_t;

/**************************************************************************************
* Description    : 內核RTC秒类型
**************************************************************************************/
typedef time_sys_t pcf85263_sec_t;

/**************************************************************************************
* MacroName      : pcf85263_i2c_read()
* Description    : I2C读寄存器
* EntryParameter : reg,寄存器, len,读数据长度, rxbuf,读缓冲
* ReturnValue    : 返回错误码
**************************************************************************************/
#define pcf85263_i2c_read(reg,len,rxbuf) \
    i2c_read_block_data(PCF85263_DEV_ADDR, reg, len, rxbuf)

/**************************************************************************************
* MacroName      : pcf85263_i2c_write()
* Description    : I2C写寄存器
* EntryParameter : reg,寄存器, len,写数据长度, txbuf,写缓冲
* ReturnValue    : 返回错误码
**************************************************************************************/
#define pcf85263_i2c_write(reg,len,txbuf) \
    i2c_write_block_data(PCF85263_DEV_ADDR, reg, len, txbuf)

#endif /* __PCF85263_H__ */

