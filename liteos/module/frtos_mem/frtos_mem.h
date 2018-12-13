/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_mem.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_MEM_H__
#define __FRTOS_MEM_H__

#include "frtos_types.h"

#ifdef USING_OS_FREERTOS
/**************************************************************************************
* MacroName      : mem_malloc()
* Description    : 申请内存空间
* EntryParameter : size,申请的字节数
* ReturnValue    : 返回内存指针, NULL,申请失败
**************************************************************************************/
#define mem_malloc(size)        pvPortMalloc(size)

/**************************************************************************************
* MacroName      : mem_free()
* Description    : 释放内存空间
* EntryParameter : pv,内存指针
* ReturnValue    : None
**************************************************************************************/
#define mem_free(pv)            do {vPortFree(pv); pv = NULL; }while(0)
#else
/**************************************************************************************
* MacroName      : mem_malloc()
* Description    : 申请内存空间
* EntryParameter : size,申请的字节数
* ReturnValue    : 返回内存指针, NULL,申请失败
**************************************************************************************/
static inline void *mem_malloc(uint32_t size)
{
    return malloc(size);
}

/**************************************************************************************
* MacroName      : mem_free()
* Description    : 释放内存空间
* EntryParameter : pv,内存指针
* ReturnValue    : None
**************************************************************************************/
static inline void mem_free(void *pv)
{
    free(pv);
}
#endif

#endif /*__FRTOS_MEM_H__ */

