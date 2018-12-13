/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : rtc.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "frtos_time.h"
#include "rtc.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static time_sys_t rtc_seconds = 0;

/**************************************************************************************
* FunctionName   : rtc_getdtime()
* Description    : 获取物理设备RTC时间
* EntryParameter : *dtime,返回获取的日期时间
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t rtc_getdtime(time_date_t *dtime)
{
    return time_stm2dtm(rtc_seconds, dtime);
}

/**************************************************************************************
* FunctionName   : rtc_getdtime()
* Description    : 设置物理设备RTC时间
* EntryParameter : *dtime,带设置的日期时间指针
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t rtc_setdtime(time_date_t *dtime)
{
	return time_dtm2stm(dtime, &rtc_seconds);
}

/**************************************************************************************
* FunctionName   : rtc_setsecirq()
* Description    : 设置物理设备RTC秒中断参数
* EntryParameter : *secirq_args,秒中断参数指针
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t rtc_setsecirq(const uint32_t *sec)
{
    (void)sec;
    return 0;
}

/**************************************************************************************
* TypeName       : spc_rtc_irq()
* Description    : RTC中断函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void spc_rtc_irq(void)
{
    IRQ_PROLOGUE();

    // 1. 读取中断状态，并清除中断标记
    osalEnterCriticalFromISR();
    RTC.RTCC.B.CNTEN = 0U;

    if (RTC.RTCS.B.RTCF == 1U) {
        rtc_seconds++;
    }
    //RTC.RTCC.B.RTCVAL = (RTC.RTCC.B.RTCVAL + 1);
    RTC.RTCC.B.CNTEN = 1U;
    RTC.RTCS.B.RTCF = 1U;
    osalExitCriticalFromISR();
    IRQ_EPILOGUE();
}

/**************************************************************************************
* FunctionName   : rtc_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t rtc_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len > 0) || \
        (NULL != args && len > 0) || len < 0)){
        return -EINVAL;
    }

    // 1.执行控制命令
    switch(cmd){
    case _IOC_RTC_SETTIME:
        return rtc_setdtime(args);
    case _IOC_RTC_SETIRQ:
        return rtc_setsecirq(args);
    case _IOC_RTC_GETTIME:
        return rtc_getdtime(args);
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : rtc_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init rtc_init(void)
{
    // 1.设置RTC时钟和运行模式
    SPCSetPeripheralClockMode(91,SPC5_ME_PCTL_RUN(1) | SPC5_ME_PCTL_LP(2));

    // 2.复位RTC计数器
    RTC.RTCC.B.CNTEN = 0U;
    // 3.清除API中断标志
    RTC.RTCS.B.APIF = 1U;
    // 4.设置内部RTC时钟源和分频, 1s会产生一次RTCVAL匹配中断
    RTC.RTCC.B.CLKSEL = RTC_SXOSC_CLK;
    RTC.RTCC.B.DIV32EN= RTC_32_DIVIDE;
    RTC.RTCC.B.DIV512EN = !RTC_512_DIVIDE;
    RTC.RTCC.B.FRZEN = 1U;

    // 5.使能外部RTC时钟32.768KHZ
    CGM.SXOSC_CTL.B.OSCON = 1;

    // 6. 1s产生一次中断
    RTC.RTCC.B.RTCVAL = 1;
    RTC.RTCC.B.RTCIE = 1;

    // 7.开启计数器
    RTC.RTCC.B.CNTEN = 1U;

    return 0;
}

static __const struct driver spc_rtc = {
    .idx   = DRIVER_RTC,
    .name  = "rtc",
    .init  = rtc_init,
    .ioctl = rtc_ioctrl,
};

/**************************************************************************************
* Description    : 安装唤醒中断服务函数
**************************************************************************************/
SETUP_IRQ(38, spc_rtc_irq);

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(spc_rtc);
