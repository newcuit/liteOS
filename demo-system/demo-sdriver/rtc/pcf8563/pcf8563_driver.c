/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : pcf8563_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "frtos_utils.h"
#include "frtos_mem.h"
#include "pcf8563_driver.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static mutex_lock_t pcf8563_mutex = NULL;    // 内部访问锁

/**************************************************************************************
* FunctionName   : pcf8563_phy_time_get()
* Description    : 获取物理设备RTC时间
* EntryParameter : *dtime,返回获取的日期时间
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf8563_phy_time_get(pcf8563_dtime_t *dtime)
{
    int8_t err = 0;
    uint8_t buf[9] = {0};

    // 2.获取当前时间
    err = pcf8563_i2c_read(PCF8563_REG_ST1, 9, buf);

    // 3.低电压检查
    if(err == 0 && !(buf[PCF8563_REG_SC] & PCF8563_SC_LV)){
        dtime->seconds = utils_bcd2bin(buf[PCF8563_REG_SC] & 0x7F);
        dtime->minutes = utils_bcd2bin(buf[PCF8563_REG_MN] & 0x7F);
        dtime->hour = utils_bcd2bin(buf[PCF8563_REG_HR] & 0x3F);
        dtime->day = utils_bcd2bin(buf[PCF8563_REG_DM] & 0x3F);
        dtime->month = utils_bcd2bin(buf[PCF8563_REG_MO] & 0x1F) - 1;
        dtime->year = utils_bcd2bin(buf[PCF8563_REG_YR]);
        dtime->year += buf[PCF8563_REG_MO] & PCF8563_MO_C?100:0;
        dtime->year += 1970;
    }else{
        err = -EFAULT;
    }

    return err;
}

/**************************************************************************************
* FunctionName   : pcf8563_phy_time_set()
* Description    : 设置物理设备RTC时间
* EntryParameter : *dtime,带设置的日期时间指针
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf8563_phy_time_set(const pcf8563_dtime_t *dtime)
{
    uint8_t buf[9] = {0};

    // 1.编码转换
    buf[PCF8563_REG_SC] = utils_bin2bcd(dtime->seconds);
    buf[PCF8563_REG_MN] = utils_bin2bcd(dtime->minutes);
    buf[PCF8563_REG_HR] = utils_bin2bcd(dtime->hour);
    buf[PCF8563_REG_DM] = utils_bin2bcd(dtime->day);
    buf[PCF8563_REG_DW] = 0x00;
    buf[PCF8563_REG_MO] = utils_bin2bcd(dtime->month + 1);
    buf[PCF8563_REG_YR] = utils_bin2bcd((dtime->year - 1970)%100);
    if (dtime->year - 1970 >= 100)buf[PCF8563_REG_MO] |= PCF8563_MO_C;

    // 2.设置RTC
    return pcf8563_i2c_write(PCF8563_REG_SC, \
        9 - PCF8563_REG_SC, buf + PCF8563_REG_SC);
}

/**************************************************************************************
* FunctionName   : pcf8563_phy_secirq_set()
* Description    : 设置物理设备RTC秒中断参数
* EntryParameter : *secirq_args,秒中断参数指针
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf8563_phy_secirq_set(const pcf8563_secirq_t *secirq_args)
{
    // TODO:实现秒中断
    (void)secirq_args;

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf8563_phy_alarm_mode_set()
* Description    : 设置定时器模式开关
* EntryParameter : on_off,RTC开关(true,开, false,关)
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf8563_phy_alarm_mode_set(bool on_off)
{
    uint8_t buf = 0;

    // 1.读寄存器
    if(0 != pcf8563_i2c_read(PCF8563_REG_ST2, 1, &buf)){
        return -EBUSY;
    }

    // 2.设置寄存器
    if(true == on_off){
        buf |= PCF8563_BIT_AIE;
    }else{
        buf &= ~PCF8563_BIT_AIE;
    }
    buf &= ~(PCF8563_BIT_AF | PCF8563_BITS_ST2_N);

    // 3.写寄存器
    if(0 != pcf8563_i2c_write(PCF8563_REG_ST2, 1, &buf)){
        return -EBUSY;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf8563_phy_alarm_mode_get()
* Description    : 获取定时器模式状态
* EntryParameter : *en,使能状态, *pen,定时器状态
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf8563_phy_alarm_mode_get(uint8_t *en, uint8_t *pen)
{
    uint8_t buf = 0;

    // 1.读寄存器
    if(0 != pcf8563_i2c_read(PCF8563_REG_ST2, 1, &buf)){
        return -EBUSY;
    }

    // 2.返回状态值
    if(NULL != en ) *en  = !!(buf & PCF8563_BIT_AIE);
    if(NULL != pen) *pen = !!(buf & PCF8563_BIT_AF);

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf8563_phy_alarm_set()
* Description    : 设置报警定时器
* EntryParameter : dtime,RTC定时时间
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf8563_phy_alarm_set(pcf8563_dtime_t *dtime)
{
    uint8_t buf[4] = {0};

    buf[0] = utils_bin2bcd(dtime->minutes);
    buf[1] = utils_bin2bcd(dtime->hour);
    buf[2] = utils_bin2bcd(dtime->day);
    buf[3] = PCF8563_WDALARM_MASK;

    // 1.设置RTC定时时间
    if(0 != pcf8563_i2c_write(PCF8563_REG_AMN, 4, buf)){
        if(0 != pcf8563_phy_alarm_mode_set(true)){
            return -EBUSY;
        }
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf8563_phy_te_mode_set()
* Description    : 设置计数器模式开关
* EntryParameter : RTC开关(true,开, false,关)
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf8563_phy_te_mode_set(bool on_off)
{
    uint8_t buf = 0;

    // 1.读寄存器
    if(0 != pcf8563_i2c_read(PCF8563_REG_ST2, 1, &buf)){
        return -EBUSY;
    }

    // 2.设置寄存器
    if(true == on_off){
        buf |= PCF8563_BIT_TIE;
    }else{
        buf &= ~PCF8563_BIT_TIE;
    }
    buf &= ~(PCF8563_BIT_TF | PCF8563_BITS_ST2_N);

    // 3.写寄存器
    if(0 != pcf8563_i2c_write(PCF8563_REG_ST2, 1, &buf)){
        return -EBUSY;
    }

    // 4.读寄存器
    if(0 != pcf8563_i2c_read(PCF8563_REG_TMRC, 1, &buf)){
        return -EBUSY;
    }

    // 5.设置寄存器
    if(true == on_off){
        buf |= PCF8563_TMRC_TE;
    }else{
        buf &= ~PCF8563_TMRC_TE;
    }

    // 6.写寄存器
    if(0 != pcf8563_i2c_write(PCF8563_REG_TMRC, 1, &buf)){
        return -EBUSY;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf8563_phy_intpin_set()
* Description    : 设置中断引脚脚电平
* EntryParameter : level,电平(1,高电平, 0,低电平)
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf8563_phy_intpin_set(uint8_t level)
{
    uint8_t count = 1;

    // 1.设置中断引脚电平
    if(1 == level){
        return pcf8563_phy_te_mode_set(false);
    }

    pcf8563_i2c_write(PCF8563_REG_TMR, 1, &count);
    return pcf8563_phy_te_mode_set(true);
}

/**************************************************************************************
* FunctionName   : pcf8563_phy_init()
* Description    : 设备phy初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf8563_phy_init(void)
{
    uint8_t buf = PCF8563_TMRC_4096;

    // 1.设置精度
    if(0 != pcf8563_i2c_write(PCF8563_REG_TMRC, 1, &buf)){
        return -EBUSY;
    }

    // 2.清除报警标志
    if(0 !=  pcf8563_phy_alarm_mode_get(NULL,&buf)){
        return -EBUSY;
    }
    if(0 != buf){
        return pcf8563_phy_alarm_mode_set(false);
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf8563_read()
* Description    : 读
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static int32_t pcf8563_read(uint8_t idx, void *data, int32_t len)
{
    int16_t err = 0;

    pcf8563_dtime_t dtime;

    if(unlikely(NULL == data || sizeof(uint32_t) != len)){
        return -EINVAL;
    }

    // 1.上锁
    mutex_lock(pcf8563_mutex);

    // 2.获取RTC时间
    err = pcf8563_phy_time_get(&dtime);
    if(0 == err)err = len;

    // 3.时间转换
    pcf8563_date2sys(&dtime, data);

    // 4.解锁
    mutex_unlock(pcf8563_mutex);

    (void)idx;
    return err;
}

/**************************************************************************************
* FunctionName   : pcf8563_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
**************************************************************************************/
static int32_t pcf8563_write(uint8_t idx, void *data, int32_t len)
{
    int32_t err = 0;
    pcf8563_dtime_t dtime;

    if(unlikely(NULL == data || sizeof(uint32_t) != len)){
        return -EINVAL;
    }

    // 1.上锁
    mutex_lock(pcf8563_mutex);

    // 2.时间转换
    pcf8563_sys2date(data, &dtime);

    // 3.设置RTC时间
    err = pcf8563_phy_time_set(&dtime);
    if(0 == err)err = len;

    // 4.解锁
    mutex_unlock(pcf8563_mutex);

    (void)idx;
    return err;
}

/**************************************************************************************
* FunctionName   : pcf8563_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf8563_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    int16_t err = 0;

    if(unlikely((NULL == args && len > 0) || (NULL != args && len == 0))){
        return -EINVAL;
    }

    // 1.上锁
    mutex_lock(pcf8563_mutex);

    // 2.执行控制命令
    switch(cmd){
    case _IOC_RTC_SETTIME:
        err = pcf8563_phy_time_set(args);
        break;
    case _IOC_RTC_SETIRQ:
        err = pcf8563_phy_secirq_set(args);
        break;
    case _IOC_RTC_SETALARM:
        err = pcf8563_phy_alarm_set(args);
        break;
    case _IOC_RTC_SETINT:
        err = pcf8563_phy_intpin_set(*(uint8_t *)args);
        break;
    default:
        err = -EINVAL;
    }

    // 3.解锁
    mutex_unlock(pcf8563_mutex);

    (void)idx;
    return err;
}

/**************************************************************************************
* FunctionName   : pcf8563_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init pcf8563_init(void)
{
    pcf8563_mutex = mutex_lock_init();
    if(NULL == pcf8563_mutex) return -EPERM;

    return pcf8563_phy_init();
}

static __const struct driver pcf8563_drive = {
    .idx   = DRIVER_RTC,
    .name    = "rtc",
    .init  = pcf8563_init,
    .read  = pcf8563_read,
    .write = pcf8563_write,
    .ioctl = pcf8563_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(pcf8563_drive);
