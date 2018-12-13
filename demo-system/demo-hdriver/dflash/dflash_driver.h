/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : pflash_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __DFLASH_DRIVER_H__
#define __DFLASH_DRIVER_H__

#include "frtos_types.h"
#include "S32K144.h"
#include "flash_driver.h"
#include "interrupt_manager.h"
#include "frtos_ioctl.h"

/**************************************************************************************
* Description    : Dflash的擦除扇区大小
**************************************************************************************/
#define DFLASH_SECTOR_SIZE FEATURE_FLS_DF_BLOCK_SECTOR_SIZE
#define DFLASH_BASE                                 0x10000000U
#define DFALSH_RAMBASE                              0x14000000U

/**************************************************************************************
* Description    : DFLASH控制参数结构
**************************************************************************************/
struct dflash {
    uint32_t addr;                             // DFLASH数据的地址 偏移
    uint32_t length;                           // DFLASH数据的读取长度
    uint8_t *data;                             // 指向存储数据的指针
};

#endif

