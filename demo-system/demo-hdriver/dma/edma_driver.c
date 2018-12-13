/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : edma_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "edma_driver.h"
#include "spc5_edma.h"

/**************************************************************************************
* FunctionName   : edma_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init edma_init(void)
{
    // 1.初始化dma
    edmaInit();

    return 0;
}

static __const struct driver spc_edma = {
    .name  = "edma",
    .init  = edma_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(spc_edma);
