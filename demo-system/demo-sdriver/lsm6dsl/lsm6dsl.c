/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : lsm6dsl.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "frtos_log.h"
#include "frtos_tasklet.h"
#include "power_driver.h"
#include "lsm6dsl.h"
#include "frtos_log.h"

/**************************************************************************************
* Description    : 清除碰撞预警标记时间间隔
**************************************************************************************/
#define EVENT_CLR_INTERVAL                              2000

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define sleep                0
#define suspend              1
#define normal               3
static uint8_t lsm6dsl_flag = normal;                        //记录当前六轴的工作状态
static uint8_t lsm6dsl_collision = 0;
static struct workqueue event_clr;

/**************************************************************************************
* FunctionName   : lsm6dsl_write_reg()
* Description    : 写寄存器
* EntryParameter : reg,寄存器地址, val,值
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t lsm6dsl_write_reg(uint8_t reg, uint8_t val)
{
    return i2c_write_block_data(LSM6DSL_IIC_ADDR, reg, 1, &val);
}

/**************************************************************************************
* FunctionName   : lsm6dsl_read_reg()
* Description    : 读寄存器
* EntryParameter : reg,寄存器地址, *val,返回值
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t lsm6dsl_read_reg(uint8_t reg, uint8_t *val)
{
    return i2c_read_block_data(LSM6DSL_IIC_ADDR, reg, 1, val);
}

/**************************************************************************************
* FunctionName   : lsm6dsl_read_16reg()
* Description    : 读16位寄存器值
* EntryParameter : reg,寄存器, *r_val,返回值
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t lsm6dsl_read_16reg(uint8_t reg, int16_t *r_val)
{
    uint8_t val[2] = {0};

    if(0 != i2c_read_block_data(LSM6DSL_IIC_ADDR, reg, 2, val)){
        return -EBUSY;
    }
    *r_val = ((int16_t)val[1]<<8) | ((int16_t)val[0]);

    return 0;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_sensitivity_gyro()
* Description    : 获取陀螺仪灵敏度
* EntryParameter : *r_sensi,返回灵敏度
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t lsm6dsl_sensitivity_gyro(float *r_sensi)
{
    uint8_t ctrl2_g = 0;

    // 1.读陀螺仪控制寄存器
    if(0 != lsm6dsl_read_reg(LSM6DSL_REG_CTRL2_G, &ctrl2_g)){
        return -EBUSY;
    }

    // 2.获取灵敏度
    if(LSM6DSL_GYRO_FS_G_EN_125DPS == (ctrl2_g & LSM6DSL_GYRO_FS_G_EN_125DPS)){
        *r_sensi = 4.375;
        goto RETURN_1;
    }
    switch(ctrl2_g & LSM6DSL_MASK_GYRO_FS_G){
    case LSM6DSL_GYRO_FS_G_245DPS:
        *r_sensi = 8.75;
        break;
    case LSM6DSL_GYRO_FS_G_500DPS:
        *r_sensi = 17.50;
        break;
    case LSM6DSL_GYRO_FS_G_1000DPS:
        *r_sensi = 35;
        break;
    case LSM6DSL_GYRO_FS_G_2000DPS:
        *r_sensi = 70;
        break;
    default:
        return -EBUSY;
    }

    RETURN_1:

    return 0;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_sensitivity_acc()
* Description    : 获取加速度灵敏度
* EntryParameter : *r_sensi,返回灵敏度
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t lsm6dsl_sensitivity_acc(float *r_sensi)
{
    uint8_t ctrl1_xl = 0;

    // 1.读取加速度控制寄存器
    if(0 != lsm6dsl_read_reg(LSM6DSL_REG_CTRL1_XL, &ctrl1_xl)){
        return -EBUSY;
    }

    // 2.获取加速度精度
    switch(ctrl1_xl & LSM6DSL_MASK_ACC_FS_XL){
    case LSM6DSL_ACC_FS_XL_2G:
        *r_sensi = 0.061;
        break;
    case LSM6DSL_ACC_FS_XL_16G:
        *r_sensi = 0.122;
        break;
    case LSM6DSL_ACC_FS_XL_4G:
        *r_sensi = 0.244;
        break;
    case LSM6DSL_ACC_FS_XL_8G:
        *r_sensi = 0.488;
        break;
    default:
        return -EBUSY;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_read_acc()
* Description    : 读取加速度值
* EntryParameter : *acc,返回加速度值(x100倍)
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t lsm6dsl_read_acc(acc_data_s *acc)
{
    int16_t axes = 0;
    float sensi = 0.0;

    // 1.获取灵明度
    if(0 != lsm6dsl_sensitivity_acc(&sensi)){
        return -EBUSY;
    }

    // 2.获取x轴加速度
    if(0 != lsm6dsl_read_16reg(LSM6DSL_REG_OUTX_L_XL, &axes)){
        return -EBUSY;
    }
    acc->acc_x = (int16_t)((float)axes * sensi);

    // 3.获取y轴加速度
    if(0 != lsm6dsl_read_16reg(LSM6DSL_REG_OUTY_L_XL, &axes)){
        return -EBUSY;
    }
    acc->acc_y = (int16_t)((float)axes * sensi);

    // 4.获取z轴加速度
    if(0 != lsm6dsl_read_16reg(LSM6DSL_REG_OUTZ_L_XL, &axes)){
        return -EBUSY;
    }
    acc->acc_z = (int16_t)((float)axes * sensi);

    /* 5.获取碰撞标记 */
    acc->collision = lsm6dsl_collision;

    return 0;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_read_gyro()
* Description    : 获取陀螺仪数据
* EntryParameter : *gyro,返回加速度值(x100倍)
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t lsm6dsl_read_gyro(gyro_data_s *gyro)
{
    int16_t axes = 0;
    float sensi = 0.0;

    // 1.获取灵明度
    if(0 != lsm6dsl_sensitivity_gyro(&sensi)){
        return -EBUSY;
    }

    // 2.获取x轴
    if(0 != lsm6dsl_read_16reg(LSM6DSL_REG_OUTX_L_G, &axes)){
        return -EBUSY;
    }
    gyro->gyro_x = (int16_t)((float)axes * sensi);

    // 3.获取y轴
    if(0 != lsm6dsl_read_16reg(LSM6DSL_REG_OUTY_L_G, &axes)){
        return -EBUSY;
    }
    gyro->gyro_y = (int16_t)((float)axes * sensi);

    // 4.获取z轴
    if(0 != lsm6dsl_read_16reg(LSM6DSL_REG_OUTZ_L_G, &axes)){
        return -EBUSY;
    }
    gyro->gyro_z = (int16_t)((float)axes * sensi);

    return 0;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_event_en()
* Description    : 运动事件检测使能控制
* EntryParameter : level,运动检测等级(1~0x3f), en,使能控制(true,使能, false,禁止)
* ReturnValue    : None
**************************************************************************************/
void lsm6dsl_event_en(uint8_t level)
{
    // 使用高通滤波，中断自动复位
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_TAP_CFG, 0x90)){
        return;
    }
    // 没有duration
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_WAKE_UP_DUR, 0x00)){
        return;
    }
    // 设置唤醒阈值 = level*2g/2^6
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_WAKE_UP_THS, level)){
        return;
    }
    // 中断配置到INT1 pin
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_MD1_CFG, 0x20)){
        return;
    }
}

/**************************************************************************************
* FunctionName   : lsm6dsl_event_clr()
* Description    : 运动事件清除
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void lsm6dsl_event_clr(void *args)
{
    lsm6dsl_collision = 0;
    (void)args;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_irq1()
* Description    : 可编程中断1
* EntryParameter : irq,中断号
* ReturnValue    : None
**************************************************************************************/
static void lsm6dsl_irq1(int8_t irq, uint8_t status)
{
    lsm6dsl_collision = 1;
    tasklet_schedule(&event_clr, NULL, EVENT_CLR_INTERVAL);
    (void)irq;(void)status;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_dev_init()
* Description    : 模块初始化控制寄存器函数
* EntryParameter : void
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lsm6dsl_dev_init(void)
{
    uint8_t val = 0;

    // 0.读取芯片ID
//    while (val != 0x6A)
    lsm6dsl_read_reg(LSM6DSL_REG_WHOAMI, &val);

    // 1.软复位LSM6DSL_REG_WHOAMI
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL3_C, 0x01)){
        return -EIO;
    }

    // 2.设置FS_XL = ±2g
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL1_XL,   \
        LSM6DSL_GYRO_ODR_208HZ | LSM6DSL_ACC_FS_XL_2G)){
        return -EIO;
    }


    // 3.设置gyro采样频率208Hz
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL2_G,   \
        LSM6DSL_GYRO_ODR_416HZ | LSM6DSL_GYRO_FS_G_245DPS)){
        return -EIO;
    }

    // 4. 设置震动事件唤醒,阈值1000mg
    lsm6dsl_event_en(0x1F);

    return 0;
}


/**************************************************************************************
* FunctionName   : lsm6dsl_threshold_set()
* Description    : 配置碰撞阈值
* EntryParameter : data，待设置的阈值参数
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lsm6dsl_threshold_set(int32_t *data)
{
    if(NULL == data) return -EINVAL;
    uint32_t threshold = *data;
    uint8_t level;

    // 1. 将阈值转换成level,threshold单位是mg。
    level = threshold / 31; // 2g的测量范围，分辨率是31.25g
    lsm6dsl_event_en(level);

    return 0;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lsm6dsl_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len > 0) || (NULL != args && len == 0))){
        return -EINVAL;
    }

    // 1.执行控制命令
    switch(cmd){
    case _IOC_GET_DATA1:
        return lsm6dsl_read_acc(args);
    case _IOC_GET_DATA2:
        return lsm6dsl_read_gyro(args);
    case _IOC_SET:
    	return lsm6dsl_threshold_set(args);
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* TypeName       : lsm6dsl_lowpower()
* Description    : 驱动进入低功耗模式类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lsm6dsl_lowpower(uint8_t mode)
{
    // 0.该模式下需要开启震动唤醒
    if(mode == suspend) {

        // 设置加速度计的震动唤醒阈值250mg
        lsm6dsl_event_en(0x08);

        // 仅关闭陀螺仪
        if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL2_G, LSM6DSL_GYRO_ODR_PWRDOWN)){
            return -EIO;
        };

        lsm6dsl_flag = suspend;
        return 0;
    }

    lsm6dsl_flag = sleep;

    // 1.关闭加速度计
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL1_XL, LSM6DSL_ACC_ODR_PWRDOWN)){
        return -EIO;
    };

    // 2.关闭陀螺仪
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL2_G, LSM6DSL_GYRO_ODR_PWRDOWN)){
        return -EIO;
    };

    return 0;
}

/**************************************************************************************
* TypeName       : lsm6dsl_wakeup()
* Description    : 驱动唤醒函数类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lsm6dsl_wakeup(void)
{
    // 0.判断陀螺仪是否已下电
    if(lsm6dsl_flag == suspend) {
        // 设置加速度计的震动唤醒阈值1000mg
            lsm6dsl_event_en(0x1F);

        //唤醒陀螺仪
        if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL2_G,   \
            LSM6DSL_GYRO_ODR_416HZ | LSM6DSL_GYRO_FS_G_245DPS)){
            return -EIO;
        }

        lsm6dsl_flag = normal;
        return 0;
    }

    // 1.设置FS_XL = ±2g
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL1_XL,   \
        LSM6DSL_GYRO_ODR_208HZ | LSM6DSL_ACC_FS_XL_2G)){
        return -EIO;
    }

    // 2.设置gyro采样频率208Hz
    if(0 != lsm6dsl_write_reg(LSM6DSL_REG_CTRL2_G,   \
        LSM6DSL_GYRO_ODR_416HZ | LSM6DSL_GYRO_FS_G_245DPS)){
        return -EIO;
    }

    lsm6dsl_flag = normal;
    return 0;
}

/**************************************************************************************
* FunctionName   : lsm6dsl_init()
* Description    : 模块初始化函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init lsm6dsl_init(void)
{
    struct wkup_config config = {
        .falling_edge = 0,
        .rising_edge = 1,
        .wkup_no = 14, //PE11,pin17
        .cb = lsm6dsl_irq1,
    };

    // 1. 初始化tasklet任务
    tasklet_init(&event_clr, lsm6dsl_event_clr);

    // 2.设置陀螺仪中断
    fdrive_ioctl(DRIVER_PM, POWER_CMD_CALLBACK, &config, sizeof(config));

    // 3.给陀螺仪开电
    gpio_pin_set(LSM6DSL_IO_POWER, 0);

    // 4.初始化陀螺仪
    return lsm6dsl_dev_init();
}

static __const struct driver lsm6dsl = {
    .idx     = DRIVER_SENSOR,
    .init    = lsm6dsl_init,
    .ioctl   = lsm6dsl_ioctrl,
    .lowpower = lsm6dsl_lowpower,
    .wakeup  = lsm6dsl_wakeup,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(lsm6dsl);
