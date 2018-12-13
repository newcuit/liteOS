/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : spcadc_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "spcadc_driver.h"

#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "frtos_time.h"
#include "frtos_log.h"
#include "config_driver.h"
#include "adc_lld_cfg.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define ADC_BUSY                                                      1 // 定义ADC忙碌状态
#define ADC_IDLE                                                      0 // 定义ADC空闲状态
#define ADC1_NUM                                                      16// 定义ADC1采样个数
#define ADC1_DEPTH                                                    1 // 定义ADC1采样次数
#define ADC0_NUM                                                      2 // 定义ADC0采样个数
#define ADC0_DEPTH                                                    1 // 定义ADC0采样次数
static uint8_t adc_flag = ADC_IDLE;

/**************************************************************************************
* FunctionName   : adc_callback()
* Description    : adc采样成功数据回调
* EntryParameter : *adc,当前ADC, sample,采样值， depth,采样次数
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
void adc_callback(ADCDriver *adc, adcsample_t *sample, size_t depth)
{
    static size_t adc_complete = 0;

    if(adc->samples == sample) adc_complete = depth;
    else adc_complete += depth;

    if(adc_complete >= adc->depth) adc_flag = ADC_IDLE;
}

/**************************************************************************************
* FunctionName   : adc_do_conversion()
* Description    : adc采样启动
* EntryParameter : *adc,当前ADC, sample,采样值， depth,采样次数
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static void adc_do_conversion(ADCDriver *adc,const ADCConversionGroup *grpp,
        adcsample_t *samples, size_t depth)
{
    adc_flag = ADC_BUSY;
    adc_lld_start_conversion(adc, grpp, samples, depth);
    while(adc_flag == ADC_BUSY);
}

/**************************************************************************************
* FunctionName   : adc_get_value()
* Description    : 求取adc多次采样的平均值
* EntryParameter : adc,adc数据结构信息， offset,起始通道偏移，  depth,采样次数
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static adcsample_t adc_get_value(adcsample_t *samples, uint8_t offset, uint8_t depth)
{
    uint8_t i = 0;
    adcsample_t value = 0;

    for (i = 0; i < depth; i++) {
        value += samples[offset + i];
    }
    return value/depth;
}

/**************************************************************************************
* FunctionName   : spcadc_read()
* Description    : 读
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回读取的字节数, 返回错误码
**************************************************************************************/
static int32_t spcadc_read(uint8_t idx, void *data, int32_t len)
{
    uint8_t i;
    uint16_t sample_mv = 0;
    struct spcadc_samp_s *samp = (struct spcadc_samp_s *)data;

    adcsample_t samples0[ADC0_NUM * ADC0_DEPTH] = {0}, samples1[ADC1_NUM * ADC1_DEPTH] = {0};

    // 1.开始ADC采集
    for(i = 0; i < len/sizeof(struct spcadc_samp_s); i++){
        if(unlikely(samp[i].adc == 0)) {
            switch(samp[i].ch){
            case 5:
            CONVERSION_ADC(0, 5);break;
            case 6:
            CONVERSION_ADC(0, 6);break;
            }
        } else {
            switch(samp[i].ch){
            case 0:
            CONVERSION_ADC(1, 0);break;
            case 1:
            CONVERSION_ADC(1, 1);break;
            case 2:
            CONVERSION_ADC(1, 2);break;
            case 3:
            CONVERSION_ADC(1, 3);break;
            case 4:
            CONVERSION_ADC(1, 4);break;
            case 5:
            CONVERSION_ADC(1, 5);break;
            case 6:
            CONVERSION_ADC(1, 6);break;
            case 7:
            CONVERSION_ADC(1, 7);break;
            case 8:
            CONVERSION_ADC(1, 8);break;
            case 9:
            CONVERSION_ADC(1, 9);break;
            case 10:
            CONVERSION_ADC(1, 10);break;
            case 11:
            CONVERSION_ADC(1, 11);break;
            case 12:
            CONVERSION_ADC(1, 12);break;
            case 13:
            CONVERSION_ADC(1, 13);break;
            case 14:
            CONVERSION_ADC(1, 14);break;
            case 15:
            CONVERSION_ADC(1, 15);break;
            }
        }
    }
    // 2.处理每个ADC信息
    for(i = 0; i < len/sizeof(struct spcadc_samp_s); i++){
        if(unlikely(samp[i].adc == 0)) {
            sample_mv = adc_get_value(samples0, samp[i].ch - 5, ADC0_DEPTH);
        } else {
            sample_mv = adc_get_value(samples1, samp[i].ch, ADC1_DEPTH);
        }

        // 3.转换
        samp[i].result = (sample_mv - samp[i].zero) * samp[i].scale;
    }

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : spcadc_suspend()
* Description    : ADC进入低功耗
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init spcadc_lowpower(void)
{
    // 关闭ADC
    adc_lld_stop(&ADCD1);
    adc_lld_stop(&ADCD2);

    return 0;
}

/**************************************************************************************
* FunctionName   : spcadc_wakeup()
* Description    : ADC从低功耗唤醒
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init spcadc_wakeup(void)
{
    // 重新初始化ADC
    adc_lld_init();

    // 重新启动ADC1和ADC0采样
    adc_lld_start(&ADCD1, NULL);
    adc_lld_start(&ADCD2, NULL);

    return 0;
}

/**************************************************************************************
* FunctionName   : spcadc_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init spcadc_init(void)
{
    // 初始化ADC
    adc_lld_init();

    // 启动ADC1和ADC0采样
    adc_lld_start(&ADCD1, NULL);
    adc_lld_start(&ADCD2, NULL);

    return 0;
}


static __const struct driver spc_adc = {
    .idx  = DRIVER_ADC,
    .name = "adc",
    .init = spcadc_init,
    .read = spcadc_read,
    .lowpower = spcadc_lowpower,
    .wakeup = spcadc_wakeup,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(spc_adc);
