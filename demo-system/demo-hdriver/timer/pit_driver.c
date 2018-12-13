/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : pit_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "pit_driver.h"

#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "pit_lld_cfg.h"
#include "pit_lld.h"

/**************************************************************************************
* Description    : 定时器回调设置
**************************************************************************************/
static struct {
    ioctl_cb1_t timer_cb;
} pit_info[MAX_PIT_COUNT];

/**************************************************************************************
* FunctionName   : spc_pit_callback()
* Description    : PIT中断回调函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void spc_pit_callback(void)
{
    uint8_t idx = 0;

    // 1.遍历PIT结构， 找到产生中断的PIT模块，然后调用对应回调
    for (idx = 0; likely(idx < MAX_PIT_COUNT); idx++) {
        if(unlikely(PITD.pit_tagp->CH[idx].TFLG.B.TIF && pit_info[idx].timer_cb)) {
            pit_info[idx].timer_cb(idx);
        }
    }
}

/**************************************************************************************
* FunctionName   : pit_ioctrl()
* Description    : PIT应用控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t pit_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely(NULL == args || len <= 0)){
        return -EINVAL;
    }

    // 1.执行命令序列
    switch(cmd){
    case _IOC_SET_CB:
    	pit_info[idx - DRIVER_PIT0].timer_cb = (ioctl_cb1_t)args;
        break;
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : pit_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __init pit_init(void)
{
    // 1.初始化PIT模块
    pit_lld_init();
    pit_lld_start(&PITD, pit_config);

    return 0;
}

/**************************************************************************************
* Description    : 注册PIT驱动
**************************************************************************************/
REGISTER_PIT(1, DRIVER_PIT1, 10, pit_ioctrl);  // 10HZ的定时频率
REGISTER_PIT(2, DRIVER_PIT2, 1, pit_ioctrl);   // 1HZ的定时频率

/**************************************************************************************
* Description    : 定义PIT初始化模块
**************************************************************************************/
static __const struct driver spc_pit = {
    .init  = pit_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
CORE_INIT(spc_pit);
