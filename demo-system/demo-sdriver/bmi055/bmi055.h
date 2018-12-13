/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : bmi055.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __BMI055_H__
#define __BMI055_H__

#include "frtos_types.h"
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_delay.h"
#include "gpio_driver.h"
#include "i2c_driver.h"
#include "frtos_ioctl.h"
#include "frtos_irq.h"
#include "frtos_sys.h"
/**************************************************************************************
* Description    : 六轴I2C地址
**************************************************************************************/
#define BMI_G_ADDR 0x68
#define BMI_A_ADDR 0x18

/**************************************************************************************
* Description    : 六轴中断引脚
**************************************************************************************/
#define GYRO_INT            PTA,14
#define ACCEL_INT           PTE,2
#define ACCEL_INT_NUM       130                     //中断号

/**************************************************************************************
* Description    : 六轴陀螺仪寄存器
**************************************************************************************/
#define BMI_G_CHIP_ID               0x00
#define BMI_G_RATE_XL               0x02
#define BMI_G_RATE_XM               0x03
#define BMI_G_RATE_YL               0x04
#define BMI_G_RATE_YM               0x05
#define BMI_G_RATE_ZL               0x06
#define BMI_G_RATE_ZM               0x07
#define BMI_G_RANGE                 0x0F
#define BMI_G_BW                    0x10
#define BMI_G_LPW1                  0x11
#define BMI_G_LPW2                  0x12
#define BMI_G_RATE_RST              0x14

/**************************************************************************************
* Description    : 六轴陀螺仪寄存器配置
**************************************************************************************/
#define BMI_G_2000_SCALE            0x00
#define BMI_G_1000_SCALE            0x01
#define BMI_G_0500_SCALE            0x02
#define BMI_G_0250_SCALE            0x03
#define BMI_G_0125_SCALE            0x04
#define BMI_G_100_32HZ              0x87
#define BMI_G_200_64HZ              0x86
#define BMI_G_100_12HZ              0x85
#define BMI_G_200_23HZ              0x84
#define BMI_G_400_47HZ              0x83
#define BMI_G_1000_116HZ            0x82
#define BMI_G_2000_230HZ            0x81
#define BMI_G_2000_523HZ            0x80

/**************************************************************************************
* Description    : 六轴加速度寄存器
**************************************************************************************/
#define BMI_A_CHIP_ID               0x00
#define BMI_A_ACCD_XL               0x02
#define BMI_A_ACCD_XM               0x03
#define BMI_A_ACCD_YL               0x04
#define BMI_A_ACCD_YM               0x05
#define BMI_A_ACCD_ZL               0x06
#define BMI_A_ACCD_ZM               0x07
#define BMI_A_ACCD_TMP              0x08
#define BMI_A_PMU_RANGE             0x0F
#define BMI_A_PMU_BW                0x10
#define BMI_A_PMU_LPW               0x11
#define BMI_A_ACCD_RST              0x14
#define BMI_A_HIGHg_EN              0x17                            // HIGHg使能
#define BMI_A_INT_MP                0x19                            // 加速度中断映射
#define BMI_A_INT_DS                0x1E                            // 中断数据源
#define BMI_A_ELEC_BV               0x20                            // 引脚电气特性
#define BMI_A_INT_MD                0x21                            // 中断模式
#define BMI_A_INT_HY                0x24                            // hysteresis
#define BMI_A_HIGHg_DL              0x25                            // HIGHg延时
#define BMI_A_HIGHg_TH              0x26                            // HIGHg阈值

/**************************************************************************************
* Description    : 六轴加速度寄存器配置
**************************************************************************************/
#define BMI_A_2G_RANGE              0x03
#define BMI_A_4G_RANGE              0x06
#define BMI_A_8G_RANGE              0x08
#define BMI_A_16G_RANGE             0x0a
#define BMI_A_7_81_HZ               0x08
#define BMI_A_15_63_HZ              0x09
#define BMI_A_31_25_HZ              0x0A
#define BMI_A_62_5_HZ               0x0B
#define BMI_A_125_HZ                0x0C
#define BMI_A_250_HZ                0x0D
#define BMI_A_500_HZ                0x0E
#define BMI_A_1000_HZ               0x0F
#define BMI_A_NORMAL                0x00
#define BMI_A_DEEP_SUSPEND          0x10
#define BMI_A_LOW_POWER             0x20
#define BMI_A_SUSPEND               0x40
#define BMI_A_INT_MAP               0x02                    // INT1
#define BMI_A_HIGHg_ENABLE          0x03                    // axisX,Y
#define BMI_A_HIGHg_DELAY           0x0F                    // [high_dur<7:0> + 1]*2ms
#define BMI_A_INT_HYSTERESIS        0x01                    // 0*125mg
#define BMI_A_HIGHg_THRESHOLD       0XFF                    // X * 7.81mg
#define BMI_A_ELEC_BEHAV            0x01                    // 引脚推挽输出，中断高电平
#define BMI_A_INT_MODE              0x08                    // 中断模式non-latched
#define BMI_A_INT_DATSRC            0x00                    // filtered
/**************************************************************************************
* Description    : 定义加速度数据结构
**************************************************************************************/
typedef struct{
    int acc_x;
    int acc_y;
    int acc_z;
    bool collision;                                              //碰撞标记
}acc_data_s;

/**************************************************************************************
* Description    : 定义陀螺仪数据结构
**************************************************************************************/
typedef struct{
    int gyro_x;
    int gyro_y;
    int gyro_z;
}gyro_data_s;

/**************************************************************************************
* FunctionName   : bmi055_delay_ns()
* Description    : 纳秒延时函数
* EntryParameter : ns,延时纳秒
* ReturnValue    : None
**************************************************************************************/
static inline void bmi055_delay_ns(uint16_t ns)
{
    frtos_delay_ns(ns);
}

/**************************************************************************************
* Description    : 六轴定义读数据
**************************************************************************************/
#define bmi_read_acc_data(reg,length,buf)  \
    i2c_read_block_data(BMI_A_ADDR,reg,length,buf)

#define bmi_read_gyro_data(reg,length,buf)  \
    i2c_read_block_data(BMI_G_ADDR,reg,length,buf)

/**************************************************************************************
* Description    : 六轴定义写数据
**************************************************************************************/
#define bmi_write_acc_data(reg,length,buf)  \
    i2c_write_block_data(BMI_A_ADDR,reg,length,buf)

#define bmi_write_gyro_data(reg,length,buf)  \
    i2c_write_block_data(BMI_G_ADDR,reg,length,buf)

#endif /*__BMI055_H__ */

