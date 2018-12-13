/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : printer.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __PRINTER_H__
#define __PRINTER_H__

#include "S32K144.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
struct tp_inode {
    int32_t len;                                // 打印数据长度
    uint8_t *data;                              // 打印数据
};

enum {WAIT_BUFFER, RUNNING_BUFFER, BUFFER_MAX};

#endif /* __PRINTER_H__ */

