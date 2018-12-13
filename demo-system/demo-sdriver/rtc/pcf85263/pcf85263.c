/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : pcf85263.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "pcf85263.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "frtos_utils.h"
#include "frtos_mem.h"
#include "power_driver.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static mutex_lock_t pcf85263_mutex = NULL;    // 内部访问锁

/**************************************************************************************
* FunctionName   : pcf85263_phy_time_get()
* Description    : 获取物理设备RTC时间
* EntryParameter : *dtime,返回获取的日期时间
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf85263_phy_time_get(pcf85263_dtime_t *dtime)
{
    int8_t err = 0;
    uint8_t buf[8] = {0};

    // 2.获取当前时间
    err = pcf85263_i2c_read(PCF85263_100SECONDS, 8, buf);
    if(err != 0) err = -EFAULT;

    // 3.低电压检查
    dtime->sec = utils_bcd2bin(buf[PCF85263_SECONDS] & 0x7F);
    dtime->min = utils_bcd2bin(buf[PCF85263_MINUTES] & 0x7F);
    dtime->hour = utils_bcd2bin(buf[PCF85263_HOURS] & 0x3F);
    dtime->day = utils_bcd2bin(buf[PCF85263_DAYS] & 0x3F);
    dtime->mon = utils_bcd2bin(buf[PCF85263_MONTHS] & 0x1F);
    dtime->year = utils_bcd2bin(buf[PCF85263_YEARS]);
    dtime->year += 2000;

    return err;
}

/**************************************************************************************
* FunctionName   : pcf85263_phy_time_set()
* Description    : 设置物理设备RTC时间
* EntryParameter : *dtime,带设置的日期时间指针
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf85263_phy_time_set(const pcf85263_dtime_t *dtime)
{
    uint8_t buf[8] = {0};

    // 1.停止RTC时钟
    buf[0] = 0x01;
    pcf85263_i2c_write(PCF85263_STOP_ENABLE, 1, buf);

    // 2.复位RTC内部状态
    buf[0] = 0xA4;
    pcf85263_i2c_write(PCF85263_RESETS, 1, buf);

    // 3.编码转换
    buf[PCF85263_SECONDS] = utils_bin2bcd(dtime->sec);
    buf[PCF85263_MINUTES] = utils_bin2bcd(dtime->min);
    buf[PCF85263_HOURS] = utils_bin2bcd(dtime->hour);
    buf[PCF85263_DAYS] = utils_bin2bcd(dtime->day);
    buf[PCF85263_WEEKDAYS] = 0x00;
    buf[PCF85263_MONTHS] = utils_bin2bcd(dtime->mon);
    buf[PCF85263_YEARS] = utils_bin2bcd(dtime->year - 2000);

    // 4.设置RTC
    pcf85263_i2c_write(PCF85263_SECONDS, 8-PCF85263_SECONDS, buf+PCF85263_SECONDS);

    // 5.使能RTC时钟
    buf[0] = 0x00;
    pcf85263_i2c_write(PCF85263_STOP_ENABLE, 1, buf);

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf85263_phy_alarm_mode_set()
* Description    : 设置定时器模式开关
* EntryParameter : on_off,RTC开关(true,开, false,关)
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int32_t pcf85263_phy_alarm_mode_set(uint8_t on)
{
    uint8_t buf = 0;

    // 1.设置定时器1  秒， 分， 小时，天定时中断
    if(on) {
        buf = PCF85263_SEC_A1E | PCF85263_MIN_A1E | PCF85263_HR_A1E| PCF85263_DAY_A1E;
    }

    // 2.写寄存器
    if(0 != pcf85263_i2c_write(PCF85263_ALARM_ENABLE, 1, &buf)){
        return -EBUSY;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf85263_phy_alarm_set()
* Description    : 设置报警定时器
* EntryParameter : dtime,RTC定时时间
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf85263_phy_alarm_set(pcf85263_dtime_t *dtime)
{
    uint8_t buf[5] = {0};

    buf[0] = utils_bin2bcd(dtime->sec);
    buf[1] = utils_bin2bcd(dtime->min);
    buf[2] = utils_bin2bcd(dtime->hour);
    buf[3] = utils_bin2bcd(dtime->day);
    buf[4] = utils_bin2bcd(dtime->mon);

    // 1.设置RTC定时时间
    if(0 != pcf85263_i2c_write(PCF85263_SECOND_ALARM, 5, buf)){
        if(0 != pcf85263_phy_alarm_mode_set(1)){
            return -EBUSY;
        }
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : pcf85263_read()
* Description    : 读
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static int32_t pcf85263_read(uint8_t idx, void *data, int32_t len)
{
    int16_t err = 0;

    pcf85263_dtime_t dtime;

    if(unlikely(NULL == data || sizeof(uint32_t) != len)){
        return -EINVAL;
    }

    // 1.上锁
    mutex_lock(pcf85263_mutex);

    // 2.获取RTC时间
    err = pcf85263_phy_time_get(&dtime);
    if(0 == err)err = len;

    // 3.时间转换
    time_dtm2stm(&dtime, data);

    // 4.解锁
    mutex_unlock(pcf85263_mutex);

    (void)idx;
    return err;
}

/**************************************************************************************
* FunctionName   : pcf85263_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
**************************************************************************************/
static int32_t pcf85263_write(uint8_t idx, void *data, int32_t len)
{
    int32_t err = 0;
    pcf85263_dtime_t dtime;

    if(unlikely(NULL == data || sizeof(uint32_t) != len)){
        return -EINVAL;
    }

    // 1.上锁
    mutex_lock(pcf85263_mutex);

    // 2.时间转换
    time_stm2dtm(*((time_sys_t *)data), &dtime);

    // 3.设置RTC时间
    err = pcf85263_phy_time_set(&dtime);
    if(0 == err) err = len;

    // 4.解锁
    mutex_unlock(pcf85263_mutex);

    (void)idx;
    return err;
}

/**************************************************************************************
* FunctionName   : pcf85263_phy_init()
* Description    : 设备phy初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf85263_phy_init(void)
{
    return 0;
}

/**************************************************************************************
* TypeName       : pcf85623_lowpower()
* Description    : 驱动进入低功耗模式类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf85623_lowpower(void)
{
    return 0;
}

/**************************************************************************************
* TypeName       : pcf85623_wakeup()
* Description    : 驱动唤醒函数类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf85623_wakeup(void)
{
    return 0;
}

/**************************************************************************************
* FunctionName   : pcf85263_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pcf85263_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    int16_t err = 0;

    if(unlikely((NULL == args && len > 0) || (NULL != args && len == 0))){
        return -EINVAL;
    }

    // 1.上锁
    mutex_lock(pcf85263_mutex);

    // 2.执行控制命令
    switch(cmd){
    case _IOC_RTC_SETTIME:
        err = pcf85263_phy_time_set(args);
        break;
    case _IOC_RTC_SETALARM:
        err = pcf85263_phy_alarm_set(args);
        break;
    default:
        err = -EINVAL;
    }

    // 3.解锁
    mutex_unlock(pcf85263_mutex);

    (void)idx;
    return err;
}

/**************************************************************************************
* FunctionName   : pcf85263_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init pcf85263_init(void)
{
    pcf85263_mutex = mutex_lock_init();
    if(NULL == pcf85263_mutex) return -EPERM;

    return pcf85263_phy_init();
}

static __const struct driver pcf85263_drive = {
    .idx     = DRIVER_RTC,
    .name    = "rtc",
    .init    = pcf85263_init,
    .read    = pcf85263_read,
    .write   = pcf85263_write,
    .ioctl   = pcf85263_ioctrl,
    .lowpower = pcf85623_lowpower,
    .wakeup  = pcf85623_wakeup,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(pcf85263_drive);
