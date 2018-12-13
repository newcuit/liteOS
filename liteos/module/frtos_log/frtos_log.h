/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_log.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_LOG_H__
#define __FRTOS_LOG_H__

#include "frtos_types.h"
#include "frtos_mem.h"
#include "mini-printf.h"

/**************************************************************************************
* Description    : 模块配置宏数据定义
**************************************************************************************/
#define STDIO_OUT_SIZE              256             // 标志输出缓冲区SIZE
#define STRBR                       "\r\n"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************
* FunctionName   : stdio_output()
* Description    : 标准输出函数,需要外部重写
* EntryParameter : data,数据指针, len,数据长度
* ReturnValue    : None
**************************************************************************************/
void stdio_output(uint8_t *dat, int16_t len);

/**************************************************************************************
* FunctionName   : stdio_strout()
* Description    : 标准字符串输出,需要外部重写
* EntryParameter : str,字符串指针
* ReturnValue    : None
**************************************************************************************/
void stdio_strout(const char *str);

/**************************************************************************************
* FunctionName   : stdio_printf()
* Description    : 格式化输出
* EntryParameter : fmt,格式化字符串, ...,可变参数
* ReturnValue    : None
**************************************************************************************/
void stdio_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /*__FRTOS_LOG_H__ */

