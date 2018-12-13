/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : muxpin_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "components.h"
#include "muxpin_driver.h"
#include "frtos_delay.h"
#include "muxpin_driver.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define sleep                0
#define suspend              1

/**************************************************************************************
* TypeName       : muxpin_suspend()
* Description    : 驱动进入低功耗模式类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t muxpin_lowpower(uint8_t mode)
{
    uint16_t i = 0;

    // 0.重新配置与唤醒无关的引脚状态
    while (lowpower_siu_init[i].pcr_index != -1) {
        SIU.PCR[lowpower_siu_init[i].pcr_index].R  = PAL_MODE_INPUT;
        i++;
    }
    // 1.根据不同的休眠模式选择引脚的配置方案
    if(mode == suspend) {
        i = 0;
        while (suspend_siu_init[i].pcr_index != -1) {
            SIU.GPDO[suspend_siu_init[i].pcr_index].R = suspend_siu_init[i].gpdo_value;
            SIU.PCR[suspend_siu_init[i].pcr_index].R  = suspend_siu_init[i].pcr_value;
            i++;
        }
    } else {
         i = 0;
         while (sleep_siu_init[i].pcr_index != -1) {
             SIU.GPDO[sleep_siu_init[i].pcr_index].R = sleep_siu_init[i].gpdo_value;
             SIU.PCR[sleep_siu_init[i].pcr_index].R  = sleep_siu_init[i].pcr_value;
             i++;
         }
    }
    return 0;
}

/**************************************************************************************
* TypeName       : muxpin_wakeup()
* Description    : 驱动唤醒函数类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t muxpin_wakeup(void)
{
    // 1.初始化PIN
    boardInit();

    return 0;
}

/**************************************************************************************
* FunctionName   : muxpin_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init muxpin_init(void)
{
    // 1.初始化PIN
    boardInit();

    return 0;
}

static __const struct driver spc_muxpin = {
    .name    = "muxpin",
    .init    = muxpin_init,
    .lowpower = muxpin_lowpower,
    .wakeup  = muxpin_wakeup,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
ARCH_INIT(spc_muxpin);

