/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : pflash_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __PFLASH_DRIVER_H__
#define __PFLASH_DRIVER_H__

#include "frtos_types.h"
#include "ssd_c90fl.h"
#include "flashmap.h"
#include "frtos_ioctl.h"

/**************************************************************************************
* Description    : Pflash的擦除块大小
**************************************************************************************/
#define PFLASH_SECTOR_SIZE             0x3FFF

/**************************************************************************************
* Description    : PFLASH控制参数结构
**************************************************************************************/
struct pflash {
    uint32_t addr;                             // PFLASH数据的地址 偏移
    uint32_t length;                           // PFLASH数据的读取长度
    uint8_t *data;                             // 指向存储数据的指针
};

#endif

