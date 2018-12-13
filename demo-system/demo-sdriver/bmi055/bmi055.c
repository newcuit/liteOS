/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : bmi055.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "bmi055.h"
#include "frtos_log.h"
#include "frtos_tasklet.h"

/**************************************************************************************
 * Description    : 模块内部数据定义
 **************************************************************************************/
static bool bmiaccel_collision = false;
static struct workqueue event_clr;
#define INTERVAL 2000                           //清除碰撞预警标记时间间隔

/**************************************************************************************
* FunctionName   : bmiaccel_read_word()
* Description    : 获取一个字的数据
* EntryParameter : reg,基地址，word，填充写入的数据
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t bmiaccel_read_word(int8_t reg, int16_t *word)
{
    uint8_t buf[2] = {0};
    int8_t err = 0;

    err = bmi_read_acc_data(reg, 2, buf);
    if(unlikely(err != 0)) return err;

    *word = ((int16_t)buf[1]<<8) | ((int16_t)buf[0]);

    return 0;
}
/**************************************************************************************
* FunctionName   : bmigyro_read_word()
* Description    : 获取一个字的数据
* EntryParameter : reg,基地址，word，填充写入的数据
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t bmigyro_read_word(int8_t reg, int16_t *word)
{
    uint8_t buf[2] = {0};
    int8_t err = 0;

    err = bmi_read_gyro_data(reg, 2, buf);
    if(unlikely(err != 0)) return err;

    *word = ((int16_t)buf[1]<<8) | ((int16_t)buf[0]);

    return 0;
}

/**************************************************************************************
* FunctionName   : bmi_get_accelerometer()
* Description    : 获取加速度数据
* EntryParameter : 指向数据的指针
* ReturnValue    : 返回错误码
**************************************************************************************/
static int8_t bmi_get_accelerometer(acc_data_s *acc_data)
{
    int16_t axes_data;
    int8_t err = 0;

    /* 获取x轴加速度 */
    err = bmiaccel_read_word(BMI_A_ACCD_XL, &axes_data);
    if(unlikely(err != 0)) return err;
    axes_data = axes_data >> 4;
    acc_data->acc_x = axes_data;
    bmi055_delay_ns(4000);

    /* 获取y轴加速度 */
    err = bmiaccel_read_word(BMI_A_ACCD_YL, &axes_data);
    if(unlikely(err != 0)) return err;
    axes_data = axes_data >> 4;
    acc_data->acc_y = axes_data;
    bmi055_delay_ns(4000);

    /* 获取z轴加速度 */
    err = bmiaccel_read_word(BMI_A_ACCD_ZL, &axes_data);
    if(unlikely(err != 0)) return err;
    axes_data = axes_data >> 4;
    acc_data->acc_z = axes_data;
    bmi055_delay_ns(4000);

    /* 获取碰撞标记 */
    acc_data->collision = bmiaccel_collision;
    return 0;
}

/**************************************************************************************
* FunctionName   : bmi_get_gyroscope()
* Description    : 获取陀螺仪数据
* EntryParameter : 指向数据的指针
* ReturnValue    : 返回错误码
**************************************************************************************/
static int8_t bmi_get_gyroscope(gyro_data_s *gyro_data)
{
    int16_t axes_data;
    int8_t err = 0;

    /* 获取x轴 */
    err = bmigyro_read_word(BMI_G_RATE_XL, &axes_data);
    if(unlikely(err != 0)) return err;
    axes_data = axes_data >> 4;
    gyro_data->gyro_x = axes_data;
    bmi055_delay_ns(4000);

    /* 获取y轴 */
    err = bmigyro_read_word(BMI_G_RATE_YL, &axes_data);
    if(unlikely(err != 0)) return err;
    axes_data = axes_data >> 4;
    gyro_data->gyro_y = axes_data;
    bmi055_delay_ns(4000);

    /* 获取z轴*/
    err = bmigyro_read_word(BMI_G_RATE_ZL, &axes_data);
    if(unlikely(err != 0)) return err;
    axes_data = axes_data >> 4;
    gyro_data->gyro_z = axes_data;
    bmi055_delay_ns(4000);

    return 0;
}

/**************************************************************************************
* FunctionName   : bmi_run_reset()
* Description    : 模块软复位
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int8_t bmi_run_reset(void)
{
    uint8_t value;

    /* 复位模块 */
    value = 0xB6;
    bmiaccel_collision = false;
    bmi_write_acc_data(BMI_G_RATE_RST, 1, &value);
    bmi055_delay_ns(4000);
    bmi_write_acc_data(BMI_A_ACCD_RST, 1, &value);
    bmi055_delay_ns(4000);

    return 0;
}

/**************************************************************************************
* FunctionName   : bmi_config()
* Description    : 模块配置
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int8_t bmi_config(void)
{
    uint8_t value;
    /* 设置模块休眠模式 */
    value = BMI_A_PMU_LPW;
    bmi_write_acc_data(BMI_A_NORMAL, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置中断模式 */
    value = BMI_A_INT_MODE;
    bmi_write_acc_data(BMI_A_INT_MD, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置加速度中断MAP引脚 */
    value = BMI_A_INT_MAP;
    bmi_write_acc_data(BMI_A_INT_MP, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置中断引脚电气特性 */
    value = BMI_A_ELEC_BEHAV;
    bmi_write_acc_data(BMI_A_ELEC_BV, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置加速度RANGE为2G */
    value = BMI_A_2G_RANGE;
    bmi_write_acc_data(BMI_A_PMU_RANGE, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置加速度中断迟滞 */
    value = BMI_A_INT_HYSTERESIS;
    bmi_write_acc_data(BMI_A_INT_HY, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置延迟触发时间 */
    value = BMI_A_HIGHg_DELAY;
    bmi_write_acc_data(BMI_A_HIGHg_DL, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置加速度中断触发阈值 */
    value = BMI_A_HIGHg_THRESHOLD;
    bmi_write_acc_data(BMI_A_HIGHg_TH, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置加速度中断数据源 */
    value = BMI_A_INT_DATSRC;
    bmi_write_acc_data(BMI_A_INT_DS, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置加速度工作频率31.25HZ */
    value = BMI_A_31_25_HZ;
    bmi_write_acc_data(BMI_A_PMU_BW, 1, &value);
    bmi055_delay_ns(4500);

    /* 使能HIGHg的XYZ轴中断 */
    value = BMI_A_HIGHg_ENABLE;
    bmi_write_acc_data(BMI_A_HIGHg_EN, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置陀螺仪工作速率2000度每秒 */
    value = BMI_G_2000_SCALE;
    bmi_write_acc_data(BMI_G_RANGE, 1, &value);
    bmi055_delay_ns(4500);

    /* 设置陀螺仪工作带宽 */
    value = BMI_G_2000_523HZ;
    bmi_write_acc_data(BMI_G_BW, 1, &value);
    bmi055_delay_ns(4500);

    return 0;
}

/**************************************************************************************
* FunctionName   : bmiaccel_event_clr()
* Description    : 清除碰撞事件标记
* EntryParameter : none
* ReturnValue    : none
**************************************************************************************/
static void bmiaccel_event_clr(void *args)
{
    bmiaccel_collision = false;
    (void)args;
}

/**************************************************************************************
* FunctionName   : bmi_phy_init()
* Description    : 模块初始化控制寄存器函数
* EntryParameter : temp,获取温度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int8_t bmi_phy_init(void)
{
    uint8_t acc_id;
    uint8_t gyro_id;

    bmi_run_reset();

    // ACC的ID为0xFA
    bmi_read_acc_data(BMI_A_CHIP_ID, 1, &acc_id);
    bmi055_delay_ns(4500);

    /* GYRO的ID为0x0F */
    bmi_read_gyro_data(BMI_G_CHIP_ID, 1,&gyro_id);

    tasklet_init(&event_clr, bmiaccel_event_clr);
    return bmi_config();
}

/**************************************************************************************
 * FunctionName   : bmiaccel_irq()
 * Description    : 加速度中断
 * EntryParameter : irq,中断号
 * ReturnValue    : None
 **************************************************************************************/
static void bmiaccel_irq(uint32_t irq)
{
    bmiaccel_collision = true;
    tasklet_schedule(&event_clr, NULL, INTERVAL);
    (void)irq;
}

/**************************************************************************************
* FunctionName   : bmi_init()
* Description    : 模块初始化函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init bmi_init(void)
{
    /* 注册加速度中断 */
    struct irq_reg_s irq;
    irq.trig = IRQ_TRIG_UP;
    irq.handler = bmiaccel_irq;
    irq.irq = ACCEL_INT_NUM;
    fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));

    /* 初始化模块  */
    return bmi_phy_init();
}

/**************************************************************************************
* FunctionName   : s32bmi_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t s32bmi_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len > 0) || (NULL != args && len == 0))){
        return -EINVAL;
    }

    // 1.执行控制命令
    switch(cmd){
    case _IOC_GET_DATA1:
        return bmi_get_accelerometer(args);
    case _IOC_GET_DATA2:
        return bmi_get_gyroscope(args);
    case _IOC_BUS_RESET:
        return bmi_run_reset();
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

static __const struct driver s32k_bmi055 = {
    .idx  = DRIVER_SENSOR,
    .init = bmi_init,
    .ioctl = s32bmi_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(s32k_bmi055);
