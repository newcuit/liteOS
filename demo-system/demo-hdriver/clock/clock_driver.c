/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : clock_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "components.h"
#include "clock_driver.h"
#include "frtos_irq.h"

/**************************************************************************************
* MacroName      : arch_lowpower
* Description    : 系统CPU进入低功耗模式
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t arch_lowpower(void)
{
    // 0.关闭本地中断
    local_irq_disable();

    // 1.配置STOP模式的总线时钟状态
    ME.STOP0.R = 0x0005000F;

    // 2.配置STOP模式SUIL的状态
    ME.LPPC[0].B.STOP0 = 1;
    ME.PCTL[68].B.LPCFG = 0;

    // 3.配置电源域2和3
    PCU.PCONF[2].B.STOP0 = 1;
    PCU.PCONF[3].B.STOP0 = 1;
    ME.RUNPC[0].B.RUN0 = 1;
    ME.PCTL[69].B.RUNCFG = 0;

    // 4.清除标记
    WKUP.WISR.R = 0xFFFFFFFFU;

    // 5.进入STOP模式
    if (SPCSetRunMode(SPC5_RUNMODE_STOP) == CLOCK_FAILED) {
        SPC5_CLOCK_FAILURE_HOOK();
    }

    // 6.退出STOP模式，开启本地中断
    local_irq_enable();

    return 0;
}

/**************************************************************************************
* TypeName       : arch_wakeup()
* Description    : 驱动唤醒函数类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t arch_wakeup(void)
{
    clockInit();
    return 0;
}


/**************************************************************************************
* FunctionName   : clock_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init clock_init(void)
{
    // 1.初始系统时钟
    clockInit();

    return 0;
}

static __const struct driver spc_clock = {
    .name = "clock",
    .init = clock_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
ARCH_INIT(spc_clock);

