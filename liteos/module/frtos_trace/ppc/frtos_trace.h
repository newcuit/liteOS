/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_trace.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_TRACE_H__
#define __FRTOS_TRACE_H__

#include "frtos_types.h"

/**************************************************************************************
* FunctionName   : frtos_trace()
* Description    : 获取当前CPU工作模式，取对应SP传入给backtrace
* EntryParameter : None
* ReturnValue    : 返回None
**************************************************************************************/
void frtos_trace(void);

#endif /*__FRTOS_TRACE_H__ */

