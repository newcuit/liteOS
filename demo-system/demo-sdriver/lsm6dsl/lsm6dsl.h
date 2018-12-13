/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : lsm6dsl.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __LSM6DSL_H__
#define __LSM6DSL_H__

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
* Description    : 模块参数定义
**************************************************************************************/
#define LSM6DSL_IIC_ADDR                0x6A        // IIC地址

/**************************************************************************************
* Description    : 设备IO定义
**************************************************************************************/
#define LSM6DSL_IO_POWER                PORT_A,PTA13      // 电源

/**************************************************************************************
* Description    : 寄存器定义
**************************************************************************************/
#define LSM6DSL_REG_FUNC_CFG            0x01        // 嵌入式寄存器使能寄存器
#define LSM6DSL_REG_INT1_CTRL           0x0D        // INT1管脚中断配置寄存器
#define LSM6DSL_REG_WHOAMI              0x0F        // 读取芯片ID
#define LSM6DSL_REG_CTRL1_XL            0x10        // 加速度控制寄存器
#define LSM6DSL_REG_CTRL2_G             0x11        // 陀螺仪寄存器
#define LSM6DSL_REG_CTRL3_C             0x12        // 控制寄存器
#define LSM6DSL_REG_CTRL4_C             0x13        // 控制寄存器

#define LSM6DSL_REG_CTRL10_C            0x19

#define LSM6DSL_REG_TAP_CFG             0x58        // 采样模式
#define LSM6DSL_REG_WAKE_UP_DUR         0x5C        // 加速度超阈值持续时间
#define LSM6DSL_REG_WAKE_UP_THS         0x5B        // 加速度唤醒阈值寄存器
#define LSM6DSL_REG_MD1_CFG             0x5E        // 中断routing配置寄存器

#define LSM6DSL_REG_TEMP_L              0x20        // 温度寄存器低8位
#define LSM6DSL_REG_TEMP_H              0x21        // 温度寄存器高8位
#define LSM6DSL_REG_OUTX_L_G            0x22        // 陀螺仪寄存器
#define LSM6DSL_REG_OUTX_H_G            0x23
#define LSM6DSL_REG_OUTY_L_G            0x24
#define LSM6DSL_REG_OUTY_H_G            0x25
#define LSM6DSL_REG_OUTZ_L_G            0x26
#define LSM6DSL_REG_OUTZ_H_G            0x27
#define LSM6DSL_REG_OUTX_L_XL           0x28        //加速度寄存器
#define LSM6DSL_REG_OUTX_H_XL           0x29
#define LSM6DSL_REG_OUTY_L_XL           0x2A
#define LSM6DSL_REG_OUTY_H_XL           0x2B
#define LSM6DSL_REG_OUTZ_L_XL           0x2C
#define LSM6DSL_REG_OUTZ_H_XL           0x2D

/**************************************************************************************
* Description    : 嵌入式寄存器定义
**************************************************************************************/
#define LSM6DSL_REG_EM_SM_THS           0x13        // 重要的运动配置寄存器

/**************************************************************************************
* Description    : 加速度采样配置定义
**************************************************************************************/
#define LSM6DSL_ACC_ODR_PWRDOWN         0x00        // 关机
#define LSM6DSL_ACC_ODR_12HZ            0x10        // 12.5Hz
#define LSM6DSL_ACC_ODR_26HZ            0x20        // 26Hz
#define LSM6DSL_ACC_ODR_52HZ            0x30        // 52Hz
#define LSM6DSL_ACC_ODR_104HZ           0x40        // 104Hz
#define LSM6DSL_ACC_ODR_208HZ           0x50        // 208Hz
#define LSM6DSL_ACC_ODR_416HZ           0x60        // 416Hz
#define LSM6DSL_ACC_ODR_833HZ           0x70        // 833Hz
#define LSM6DSL_ACC_ODR_1660HZ          0x80        // 1660Hz
#define LSM6DSL_ACC_ODR_3330HZ          0x90        // 3330Hz
#define LSM6DSL_ACC_ODR_6660HZ          0xa0        // 6660Hz

/**************************************************************************************
* Description    : 加速度计满量程选择定义
**************************************************************************************/
#define LSM6DSL_ACC_FS_XL_2G            0x00        // 2G
#define LSM6DSL_ACC_FS_XL_4G            0x08        // 4G
#define LSM6DSL_ACC_FS_XL_8G            0x0C        // 8G
#define LSM6DSL_ACC_FS_XL_16G           0x04        // 16G

/**************************************************************************************
* Description    : 抗混叠滤波器带宽选择定义
**************************************************************************************/
#define LSM6DSL_ACC_BW_XL_400HZ         0x00        // 400Hz
#define LSM6DSL_ACC_BW_XL_200HZ         0x01        // 200Hz
#define LSM6DSL_ACC_BW_XL_100HZ         0x02        // 100Hz
#define LSM6DSL_ACC_BW_XL_50HZ          0x03        // 50Hz

/**************************************************************************************
* Description    : 定义六轴陀螺仪传感器配置定义
**************************************************************************************/
#define LSM6DSL_GYRO_ODR_PWRDOWN        0x00        // 关机
#define LSM6DSL_GYRO_ODR_12HZ           0x10        // 12.5Hz
#define LSM6DSL_GYRO_ODR_26HZ           0x20        // 26Hz
#define LSM6DSL_GYRO_ODR_52HZ           0x30        // 52Hz
#define LSM6DSL_GYRO_ODR_104HZ          0x40        // 104Hz
#define LSM6DSL_GYRO_ODR_208HZ          0x50        // 208Hz
#define LSM6DSL_GYRO_ODR_416HZ          0x60        // 416Hz
#define LSM6DSL_GYRO_ODR_833HZ          0x70        // 833Hz
#define LSM6DSL_GYRO_ODR_1660HZ         0x80        // 1660Hz

/**************************************************************************************
* Description    : 陀螺仪满量程选择定义
**************************************************************************************/
#define LSM6DSL_GYRO_FS_G_245DPS        0x00        // 245dps
#define LSM6DSL_GYRO_FS_G_500DPS        0x04        // 500dps
#define LSM6DSL_GYRO_FS_G_1000DPS       0x08        // 1000dps
#define LSM6DSL_GYRO_FS_G_2000DPS       0x0C        // 2000dps

/**************************************************************************************
* Description    : 加速度及陀螺仪满量程掩码定义
**************************************************************************************/
#define LSM6DSL_MASK_ACC_FS_XL          0x0C        // 加速度满量程屏蔽码
#define LSM6DSL_MASK_GYRO_FS_G          0x0C        // 陀螺仪满量程屏蔽码

/**************************************************************************************
* Description    : 陀螺仪满量程125dps使能
**************************************************************************************/
#define LSM6DSL_GYRO_FS_G_EN_125DPS     0x02        // 125dps使能

/**************************************************************************************
* Description    : 定义加速度数据结构
**************************************************************************************/
typedef struct{
    int acc_x;
    int acc_y;
    int acc_z;
    uint8_t collision;                                              //碰撞标记
}acc_data_s;

/**************************************************************************************
* Description    : 定义陀螺仪数据结构
**************************************************************************************/
typedef struct{
    int gyro_x;
    int gyro_y;
    int gyro_z;
}gyro_data_s;

#endif /*__LSM6DSL_H__ */

