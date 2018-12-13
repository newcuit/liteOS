/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : suspend.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "frtos_log.h"
#include "frtos_sys.h"
#include "data.pb-c.h"
#include "power_driver.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define sleep                0
#define suspend              1

/**************************************************************************************
* FunctionName   : wakeup_config()
* Description    : 设备休眠唤醒参数
* EntryParameter : mode 低功耗模式 sleep or suspend
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t wakeup_config(uint8_t mode)
{
    // 1.设置唤醒参数
    struct wkup_config acc_config = {
       .falling_edge = 1,
       .rising_edge = 0,
       .wkup_no = 10,
       .cb = NULL,
    };
    fdrive_ioctl(DRIVER_PM, POWER_CMD_CALLBACK, &acc_config, sizeof(acc_config));

    if(mode == suspend){
        struct wkup_config g4_config = {
           .falling_edge = 1,
           .rising_edge = 0,
           .wkup_no = 15,
           .cb = NULL,
        };
        fdrive_ioctl(DRIVER_PM, POWER_CMD_CALLBACK, &g4_config, sizeof(g4_config));
        struct wkup_config rtc_config = {
           .falling_edge = 0,
           .rising_edge = 1,
           .wkup_no = 19,
           .cb = NULL,
        };
        fdrive_ioctl(DRIVER_PM, POWER_CMD_CALLBACK, &rtc_config, sizeof(rtc_config));
        struct wkup_config can1_config = {
           .falling_edge = 0,
           .rising_edge = 1,
           .wkup_no = 5,
           .cb = NULL,
        };
        fdrive_ioctl(DRIVER_PM, POWER_CMD_CALLBACK, &can1_config, sizeof(can1_config));
        struct wkup_config can2_config = {
           .falling_edge = 1,
           .rising_edge = 0,
           .wkup_no = 7,
           .cb = NULL,
        };
        fdrive_ioctl(DRIVER_PM, POWER_CMD_CALLBACK, &can2_config, sizeof(can2_config));
        struct wkup_config can3_config = {
           .falling_edge = 1,
           .rising_edge = 0,
           .wkup_no = 4,
           .cb = NULL,
        };
        fdrive_ioctl(DRIVER_PM, POWER_CMD_CALLBACK, &can3_config, sizeof(can3_config));
        struct wkup_config can4_config = {
           .falling_edge = 1,
           .rising_edge = 0,
           .wkup_no = 6,
           .cb = NULL,
        };
        fdrive_ioctl(DRIVER_PM, POWER_CMD_CALLBACK, &can4_config, sizeof(can4_config));
    }

    return 0;
}
/**************************************************************************************
* FunctionName   : lowpower_system()
* Description    : 休眠设备
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t lowpower_system(uint8_t idx, void *data, int32_t len)
{
    int32_t control = 0;
    Subid *msg = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    control = msg->id;

    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    // 3.指令不是休眠返回
    if(unlikely(control != IOC__SUSPEND)){
        if(unlikely(control != IOC__SLEEP)) return -EFAULT;
        stdio_printf("system sleep now."STRBR);
        wakeup_config(sleep);
        goto SLEEP;
    }
    stdio_printf("system suspend now."STRBR);
    wakeup_config(suspend);
    goto SUSPEND;

    // 4.设备调用系统函数进入休眠或低功耗
SLEEP:
    system_lowpower(sleep);

SUSPEND:
    system_lowpower(suspend);

    // 5.无条件唤醒设备
    system_wakeup();

    (void)idx;
    return len;
}

/**************************************************************************************
* TypeName       : lowpower_cmd()
* Description    : 进入休眠控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lowpower_cmd(char *argv, int32_t argc)
{
#ifndef use_sleep
    // 0.设置唤醒参数
    wakeup_config(sleep);

    // 1.设备挂起
    stdio_printf("system sleep now."STRBR);

    system_lowpower(sleep);
#else
    // 0.设置唤醒参数
    wakeup_config(suspend);

    // 1.设备挂起
    stdio_printf("system suspend now."STRBR);

    system_lowpower(suspend);
#endif

    // 2.设备唤醒
    system_wakeup();

    (void)argv;
    return argc;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite lowpower = {
    .idx   = LOWPOWER_PID,
    .name  = "lowpower",
    .set   = lowpower_system,
    .debug = lowpower_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(lowpower);
