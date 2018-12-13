/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : spcadc_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __SPCADC_DRIVER_H__
#define __SPCADC_DRIVER_H__

#include "frtos_types.h"
#include "adc_lld_cfg.h"

/**************************************************************************************
* Description    : ADC采样数据读取结果
**************************************************************************************/
struct spcadc_samp_s{
    uint8_t adc;                    // ADC索引(0,1)
    uint8_t ch;                     // ADC通道索引
    int16_t zero;                   // 校零AD值
    float scale;                    // 最小刻度值(单位:mv)
    int32_t result;                 // 采样结果(单位：mv)
};

/**************************************************************************************
* Description    : ADC采样通道选择
**************************************************************************************/
#define CONVERSION_ADC(group, ch)                                                           \
    if(unlikely(group == 0)) {                                                              \
        adc_do_conversion(&ADCD1, &adc##group##_group_adc##group##_config_##ch, &samples0[ch-5],\
                ADC##group##_GROUP_ADC##group##_CONFIG_##ch##_BUF_DEPTH);\
    }else{                                                                                \
        adc_do_conversion(&ADCD2, &adc##group##_group_adc##group##_config_##ch, &samples1[ch], \
                ADC##group##_GROUP_ADC##group##_CONFIG_##ch##_BUF_DEPTH);\
    }
#endif /* __SPCADC_DRIVER_H__ */

