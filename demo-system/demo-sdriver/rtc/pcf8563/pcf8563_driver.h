/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : PCF8563_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __PCF8563_DRIVER_H__
#define __PCF8563_DRIVER_H__

#include "frtos_types.h"
#include "rtc_driver.h"
#include "i2c_driver.h"
#include "frtos_ioctl.h"

/**************************************************************************************
* Description    : PCF8563 RTC寄存器
**************************************************************************************/
#define PCF8563_DEV_ADDR                0X51       // pcf8563设备地址
#define PCF8563_REG_ST1                 0x00       // 状态寄存器1
#define PCF8563_REG_ST2                 0x01       // 状态寄存器2
#define PCF8563_BIT_AIE                 (1 << 1)   // 定时器中断使能位
#define PCF8563_BIT_TIE                 (1 << 0)   // 计数器中断使能位
#define PCF8563_BIT_AF                  (1 << 3)   // 定时器标志位
#define PCF8563_BIT_TF                  (1 << 2)   // 计数器标志位
#define PCF8563_BITS_ST2_N              (7 << 5)
#define PCF8563_REG_SC                  0x02       // 秒寄存器
#define PCF8563_REG_MN                  0x03       // 分钟寄存器
#define PCF8563_REG_HR                  0x04       // 小时寄存器
#define PCF8563_REG_DM                  0x05       // 天寄存器
#define PCF8563_REG_DW                  0x06       // 周寄存器
#define PCF8563_REG_MO                  0x07       // 月寄存器
#define PCF8563_REG_YR                  0x08       // 年寄存器
#define PCF8563_REG_AMN                 0x09
#define PCF8563_REG_CLKO                0x0D
#define PCF8563_REG_TMRC                0x0E
#define PCF8563_WDALARM_MASK            0x80
#define PCF8563_TMRC_ENABLE             BIT(7)
#define PCF8563_TMRC_4096               0
#define PCF8563_TMRC_64                 1
#define PCF8563_TMRC_1                  2
#define PCF8563_TMRC_1_60               3
#define PCF8563_TMRC_TE                 (1 << 7)
#define PCF8563_TMRC_MASK               3
#define PCF8563_REG_TMR                 0x0F
#define PCF8563_SC_LV                   0x80       // 电压状态寄存器
#define PCF8563_MO_C                    0x80

/**************************************************************************************
* Description    : 內核RTC日期时间数据类型
**************************************************************************************/
typedef rtc_timedate_t pcf8563_dtime_t;

/**************************************************************************************
* Description    : 內核RTC秒中断参数设置
**************************************************************************************/
typedef rtc_seconds_int_config_t pcf8563_secirq_t;

/**************************************************************************************
* MacroName      : pcf8563_sys2date()
* Description    : 系统时间转日期时间
* EntryParameter : s,系统时间, d,日期时间
* ReturnValue    : 返回错误码
**************************************************************************************/
#define pcf8563_sys2date(s,d) RTC_DRV_ConvertSecondsToTimeDate(s, d)

/**************************************************************************************
* MacroName      : pcf8563_date2sys()
* Description    : 日期时间转系统时间
* EntryParameter : reg,寄存器, len,读数据长度, rxbuf,读缓冲
* ReturnValue    : 返回错误码
**************************************************************************************/
#define pcf8563_date2sys(d,s) RTC_DRV_ConvertTimeDateToSeconds(d, s)

/**************************************************************************************
* MacroName      : pcf8563_i2c_read()
* Description    : I2C读寄存器
* EntryParameter : reg,寄存器, len,读数据长度, rxbuf,读缓冲
* ReturnValue    : 返回错误码
**************************************************************************************/
#define pcf8563_i2c_read(reg,len,rxbuf) \
    i2c_read_block_data(PCF8563_DEV_ADDR,reg,len,rxbuf)

/**************************************************************************************
* MacroName      : pcf8563_i2c_write()
* Description    : I2C写寄存器
* EntryParameter : reg,寄存器, len,写数据长度, txbuf,写缓冲
* ReturnValue    : 返回错误码
**************************************************************************************/
#define pcf8563_i2c_write(reg,len,txbuf) \
    i2c_write_block_data(PCF8563_DEV_ADDR,reg,len,txbuf)

#endif /* __PCF8563_DRIVER_H__ */

