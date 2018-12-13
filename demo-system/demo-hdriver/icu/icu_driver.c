/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : irq.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "components.h"
#include "frtos_irq.h"

/**************************************************************************************
* FunctionName   : icu_ioctrl()
* Description    : ICU应用控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t icu_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    (void)idx;
    (void)cmd;
    (void)args;
    (void)len;
    return 0;
}

/*************************************************************************************
* FunctionName   : icu_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t __init icu_init(void)
{
    /* 1.初始化ICU*/
	icu_lld_init();

    return 0;
}

/*************************************************************************************
* Description    : 模块初始化
*************************************************************************************/
static __const struct driver spc_icu = {
    .idx   = DRIVER_ICU,
    .name  = "icu",
    .init  = icu_init,
    .ioctl = icu_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
CORE_INIT(spc_icu);
