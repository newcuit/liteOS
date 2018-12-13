/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_log.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_log.h"
#include "frtos_lock.h"

/**************************************************************************************
* FunctionName   : stdio_output()
* Description    : 标准输出函数,需要外部重写
* EntryParameter : data,数据指针, len,数据长度
* ReturnValue    : None
**************************************************************************************/
void __default stdio_output(uint8_t *dat, int16_t len)
{
    (void)dat;(void)len;
}

/**************************************************************************************
* FunctionName   : stdio_strout()
* Description    : 标准字符串输出,需要外部重写
* EntryParameter : str,字符串指针
* ReturnValue    : None
**************************************************************************************/
void __default stdio_strout(const char *str)
{
    stdio_output((uint8_t *)str, strlen(str));
}

/**************************************************************************************
* FunctionName   : stdio_printf()
* Description    : 格式化输出
* EntryParameter : fmt,格式化字符串, ...,可变参数
* ReturnValue    : None
**************************************************************************************/
void stdio_printf(const char *fmt, ...)
{
    va_list va;
    uint8_t buf[STDIO_OUT_SIZE] = {0};

    // 1.上锁
    vTaskSuspendAll();

    // 3.格式化数据
    va_start(va, fmt);
    vsnprintf((void *)buf, STDIO_OUT_SIZE, fmt, va);
    va_end(va);

    // 4.输出数据
    stdio_output(buf, strlen((void *)buf));

    // 6.解锁
    xTaskResumeAll();
}

