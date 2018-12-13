/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : muxpin_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __MUXPIN_DRIVER_H__
#define __MUXPIN_DRIVER_H__

#include "frtos_types.h"

static const spc_siu_init_t lowpower_siu_init[] = {
    {(int32_t)PCR_INDEX(PORT_A, PTA1), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, I2C0_SDA), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, SPI3_CLK), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, SPI3_IN), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, LIN6_TX), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, LIN6_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, LIN7_TX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, LIN7_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, I2C0_SCL), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, PTG14), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, PTG15), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, PTA14), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, CAN0_TXD), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, LIN0_TX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, LIN0_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, WKPU3), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, ADC1_P0), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, ADC1_P1), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, ADC1_P2), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, ADC1_P3), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, PTB12), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_B, PTB13), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, EIRQ13), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, EIRQ6), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, SPI1_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, SPI1_TX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, LIN1_TX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, LIN1_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, LIN2_TX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, LIN2_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, CAN1_TXD), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, PTC13), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, PTC14), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, PTC15), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, LIN4_TX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P6), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P7), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P8), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P9), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P10), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P11), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P12), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P13), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P14), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P15), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, LIN4_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC0_S5), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC0_S6), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, CAN5_TXD), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, PTE2), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, SPI1_CLK), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, SPI1_CS0), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, LIN3_TX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, CAN2_TXD), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, PTE12), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, EIRQ12), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, LIN3_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, PTF0), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, PTF1), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, PTF2), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, PTF3), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, PTF5), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, PTF6), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, WKPU22), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, LIN5_TX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, LIN5_RX), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, EIRQ14), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, SPI3_OUT), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_G, SPI3_CS0), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, WKPU9), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, PTF10), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P4), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_D, ADC1_P5), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, PTE15), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_F, WKPU15), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, PTE6), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_E, PTE7), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_C, PTC12), PAL_LOW,    (iomode_t)(PAL_MODE_INPUT)},
    {(int32_t)PCR_INDEX(PORT_A, PTA0), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},    // RTC
    {(int32_t)PCR_INDEX(PORT_E, WKPU14), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},  // AAC
    {(int32_t)PCR_INDEX(PORT_B, CAN0_RXD), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)},  // CAN3
    {(int32_t)PCR_INDEX(PORT_C, CAN1_RXD), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)},  // CAN1
    {(int32_t)PCR_INDEX(PORT_E, CAN2_RXD), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)},  // CAN4
    {(int32_t)PCR_INDEX(PORT_E, CAN5_RX), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)},   // CAN2
    {-1, 0, 0}
};

static const spc_siu_init_t sleep_siu_init[] = {
    // 唤醒引脚配置
    {(int32_t)PCR_INDEX(PORT_A, WKPU10), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)}, // ACC
    //电源保持引脚配置
    {(int32_t)PCR_INDEX(PORT_E, BAT_KEEPON), PAL_HIGH,   (iomode_t)(PAL_MODE_OUTPUT_PUSHPULL)},
    {(int32_t)PCR_INDEX(PORT_C, PTC2), PAL_HIGH,   (iomode_t)(PAL_MODE_OUTPUT_PUSHPULL)},
    //特殊操作关闭的电源
    {(int32_t)PCR_INDEX(PORT_F, PTF4), PAL_HIGH,   (iomode_t)(PAL_MODE_OUTPUT_PUSHPULL)}, // 4G1_3V3EN
    {(int32_t)PCR_INDEX(PORT_A, PTA13), PAL_HIGH,   (iomode_t)(PAL_MODE_OUTPUT_PUSHPULL)}, // LSM6DSL电源
    {-1, 0, 0}
};

static const spc_siu_init_t suspend_siu_init[] = {
    // 唤醒引脚配置
    {(int32_t)PCR_INDEX(PORT_A, WKPU10), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)}, // ACC
    {(int32_t)PCR_INDEX(PORT_A, PTA0), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},    // RTC
    {(int32_t)PCR_INDEX(PORT_E, WKPU14), PAL_LOW,   (iomode_t)(PAL_MODE_INPUT)},  // AAC
    {(int32_t)PCR_INDEX(PORT_B, CAN0_RXD), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)},  // CAN3
    {(int32_t)PCR_INDEX(PORT_C, CAN1_RXD), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)},  // CAN1
    {(int32_t)PCR_INDEX(PORT_E, CAN2_RXD), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)},  // CAN4
    {(int32_t)PCR_INDEX(PORT_E, CAN5_RX), PAL_HIGH,   (iomode_t)(PAL_MODE_INPUT)},   // CAN2
    //电源保持引脚配置
    {(int32_t)PCR_INDEX(PORT_E, BAT_KEEPON), PAL_HIGH,   (iomode_t)(PAL_MODE_OUTPUT_PUSHPULL)},
    {(int32_t)PCR_INDEX(PORT_C, PTC2), PAL_HIGH,   (iomode_t)(PAL_MODE_OUTPUT_PUSHPULL)},
    //4G模块进入低功耗模式（下降沿休眠，上升沿唤醒）
    {(int32_t)PCR_INDEX(PORT_B, PTB10), PAL_LOW,   (iomode_t)(PAL_MODE_OUTPUT_PUSHPULL)},
    {-1, 0, 0}
};


    /* Initialization array for the PSMI registers.*/
static const uint8_t spc_padsels_init[SPC5_SIUL_NUM_PADSELS] = {
    1,   0,   0,   0,   0,   0,   0,   1,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   1,   0,   0,   0,
};

#endif /* __MUXPIN_DRIVER_H__ */

