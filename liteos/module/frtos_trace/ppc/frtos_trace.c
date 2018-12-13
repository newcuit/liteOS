/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.
 * File Name     : frtos_trace.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "string.h"
#include "frtos_sys.h"
#include "frtos_mem.h"
#include "frtos_log.h"
#include "mini-printf.h"

/**************************************************************************************
* FunctionName   : trace_msg_send()
* Description    : 打印MCU异常信息，需要外部函数实现
* EntryParameter : data，打印数据内容,len数据内容长度
* ReturnValue    : 返回None
**************************************************************************************/
void __default trace_msg_send(uint8_t *data, uint32_t len)
{
    stdio_output(data, len);
}

/**************************************************************************************
* FunctionName   : trace_finished()
* Description    : 外部函数，外部实现，实现设备重启或者其他功能
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
void __default trace_finished(void)
{
    fsystem_reboot(SYS_REBOOT);
}

/**************************************************************************************
* FunctionName   : backtrace()
* Description    : 打印MCU异常的时候，程序状态值
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
static void __used backtrace(void)
{
    while(1);
}

/**************************************************************************************
* FunctionName   : frtos_trace()
* Description    : 获取当前CPU工作模式，取对应SP传入给backtrace
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
void frtos_trace(void)
{
}
