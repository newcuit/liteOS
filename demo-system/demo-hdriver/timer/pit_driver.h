/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : pit_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __PIT_DRIVER_H__
#define __PIT_DRIVER_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"

/**************************************************************************************
* Description    : 定义PIT最大个数
**************************************************************************************/
#define MAX_PIT_COUNT                           SPC5_PIT_CHANNELS

/**************************************************************************************
* Description    : 串口注册结构定义
**************************************************************************************/
#define REGISTER_PIT(ch, id, freq, ctl)                                              \
                                                                                     \
    static int32_t __init pit##ch##_init(void)                                       \
    {                                                                                \
        /* 1. 设置PIT周期 */                                                           \
        pit_config[ch].frequency = freq;                                             \
        /* 启动PIT模块 */                                                              \
        pit_lld_channel_start(&PITD, ch);                                            \
        return 0;                                                                    \
    }                                                                                \
                                                                                     \
    /* 初始化驱动结构信息 */                                                            \
    static __const struct driver spc_pit##ch = {                                     \
        .idx   = id,                                                                 \
        .name  = "pit"#ch,                                                           \
        .ioctl = ctl,                                                                \
        .init  = pit##ch##_init,                                                     \
    };                                                                               \
    /* 驱动结构注册 */                                                                 \
    MODULE_INIT(spc_pit##ch)

#endif /* __PIT_DRIVER_H__ */
