/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : version.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __VERSION_H__
#define __VERSION_H__

#include "stdio.h"
#include "frtos_types.h"
#include "string.h"
#include <stdio.h>
#include <string.h>
#include "pflash_driver.h"

/**************************************************************************************
* Description    : 定义镜像起始位置和结束位置以及每次操作读取的大小
**************************************************************************************/
#define IMAGE_HDR_BASE                          0x00008000
#define IMAGE_UPGRADE                           1

/**************************************************************************************
* Description    : 定义头部数据结构
**************************************************************************************/
struct img_hdr {
    uint16_t refresh;
    uint16_t crc16;
    int32_t len;
    uint32_t btime;
    char os_name[10];
    int16_t reserved;
};

#endif /* __VERSION_H__ */
