/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : trace.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_drivers.h"
#include "frtos_trace.h"
#include "frtos_log.h"
#include "frtos_sys.h"
#include "interrupt_manager.h"
#include "system_S32K144.h"

/**************************************************************************************
* FunctionName   : trace_msg_send()
* Description    : 打印MCU异常信息
* EntryParameter : data，打印数据内容,len数据内容长度
* ReturnValue    : 返回None
**************************************************************************************/
void trace_msg_send(uint8_t *data, uint32_t len)
{
    stdio_output(data, len);
}

/**************************************************************************************
* FunctionName   : trace_finished()
* Description    : 实现设备重启或者其他功能
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
void trace_finished(void)
{
    fsystem_reboot(SYS_REBOOT);
}

/**************************************************************************************
* FunctionName   : backtrace_init()
* Description    : 注册系统异常中断
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __init trace_init(void)
{
    // 注册ARM-M系列不可屏蔽中断
    INT_SYS_InstallHandler(NonMaskableInt_IRQn, frtos_trace, (isr_t *)0);

    // 注册ARM-M系列硬件异常中断
    INT_SYS_InstallHandler(HardFault_IRQn, frtos_trace, (isr_t *)0);

    // 注册ARM-M系列内存异常中断
    INT_SYS_InstallHandler(MemoryManagement_IRQn, frtos_trace, (isr_t *)0);

    INT_SYS_InstallHandler(BusFault_IRQn, frtos_trace, (isr_t *)0);
    INT_SYS_InstallHandler(UsageFault_IRQn, frtos_trace, (isr_t *)0);

    return 0;
}

static __const struct driver s32k_trace = {
    .init = trace_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
ARCH_INIT(s32k_trace);
