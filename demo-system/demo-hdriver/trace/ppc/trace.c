/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : trace.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_log.h"
#include "frtos_sys.h"
#include "frtos_trace.h"
#include "config_driver.h"
#include "frtos_drivers.h"

/**************************************************************************************
* FunctionName   : trace_msg_send()
* Description    : 打印MCU异常信息，需要外部函数实现
* EntryParameter : data，打印数据内容,len数据内容长度
* ReturnValue    : 返回None
**************************************************************************************/
void trace_msg_send(uint8_t *data, uint32_t len)
{
    stdio_output(data, len);
}

/**************************************************************************************
* FunctionName   : trace_finished()
* Description    : 外部函数，外部实现，实现设备重启或者其他功能
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
void trace_finished(void)
{
    fsystem_reboot(SYS_REBOOT);
}

/**************************************************************************************
* Description    : 注册bracktrace异常函数
**************************************************************************************/
SETUP_CORE_IRQ(1, frtos_trace);// machine check irq
