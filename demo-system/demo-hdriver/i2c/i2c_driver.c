/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : i2c_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "i2c_driver.h"
#include "frtos_delay.h"
#include "frtos_irq.h"
#include "frtos_log.h"
#include "i2c_lld.h"
#include "i2c_lld_cfg.h"
#include "pal_lld.h"

/**************************************************************************************
* Description    : I2C设备初始化配置
**************************************************************************************/

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static mutex_lock_t i2c_mutex = NULL;               // I2C访问锁

/**************************************************************************************
* FunctionName   : i2c_clk_9pulse()
* Description    : CLK线拉9次脉冲
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void i2c_clk_9pulse(void)
{
    uint8_t i = 0;
    uint32_t j = 0;

    // 1.将CLK复用成GPIO并设置成推挽输出
    static const spc_siu_init_t i2c_pal_config = {
        (int32_t)PCR_INDEX(PORT_A, I2C0_SCL), PAL_LOW, (iomode_t)(PAL_MODE_OUTPUT_PUSHPULL),
    };

    SIU.PCR[i2c_pal_config.pcr_index].R  = i2c_pal_config.pcr_value;


    // 3.拉至少9次时钟脉冲
    for(i = 0; i < 10; i++){
        // 4.CLK低
        pal_lld_clearpad(PORT_A, I2C0_SCL);
        for(j = 0; j < 500; j++)__barrier();

        // 5.CLK高
        pal_lld_setpad(PORT_A, I2C0_SCL);
        for(j = 0; j < 500; j++)__barrier();
    }

    // 6.恢复CLK复用类型
    SIU.PCR[i2c_pal_config.pcr_index].R  = (iomode_t)(PAL_MODE_OUTPUT_ALTERNATE(2));
}

/**************************************************************************************
* FunctionName   : i2c_transfer()
* Description    : I2C设备阻塞式收发数据
* EntryParameter : msgs,消息数组, cnt,传输Msgs个数
* ReturnValue    : 返回发送成功的个数
**************************************************************************************/
int16_t i2c_transfer(struct i2c_msg_s msgs[], int16_t cnt)
{
    int8_t i = 0;
    int8_t trans_cnt = 0;

    // 1.上锁
    mutex_lock(i2c_mutex);
    for (i = 0; i < cnt; i++){
        if(msgs[i].flags & I2C_M_RD){
            if(I2C_NO_ERROR == i2c_lld_read(msgs[i].addr << 1, msgs[i].reg, msgs[i].data, msgs[i].len))
            	trans_cnt++;
        }else{
            if(I2C_NO_ERROR == i2c_lld_write(msgs[i].addr << 1, msgs[i].reg, msgs[i].data, msgs[i].len))
            	trans_cnt++;
        }
    }
    // 2.解锁
    mutex_unlock(i2c_mutex);
    frtos_delay_ns(20000);

    return trans_cnt;
}

/**************************************************************************************
* FunctionName   : i2c_read_block_data()
* Description    : 读取寄存器内容
* EntryParameter : addr,芯片地址, reg,寄存器地址, length,缓存区长度, buf,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_read_block_data(uint8_t addr, uint8_t reg, int16_t len, uint8_t *buf)
{
    struct i2c_msg_s msgs[] = {{addr, reg, buf, len, I2C_M_RD}};

    if (1 != i2c_transfer(msgs, 1))return -EIO;

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_write_block_data()
* Description    : 写寄存器内容
* EntryParameter : addr,芯片地址, reg,寄存器地址, len,缓存区长度，buf,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_write_block_data(uint8_t addr, uint8_t reg, int16_t len, uint8_t *buf)
{
    struct i2c_msg_s msgs[] = {{addr, reg, buf, len, I2C_M_W}};

    if (1 != i2c_transfer(msgs, 1))return -EIO;

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_smbus_write_data()
* Description    : i2c写数据
* EntryParameter : addr,芯片地址, len,缓存区长度，data,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_smbus_write_data(uint8_t addr, int16_t len, uint8_t *buf,uint8_t flag)
{
    struct i2c_msg_s msgs[] = {{addr, 0, buf, len, I2C_M_W|flag}};

    if ((i2c_transfer(msgs, 1)) != 1) {
        return -EIO;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_smbus_read_data()
* Description    : 写寄存器内容
* EntryParameter : addr,芯片地址, len,缓存区长度，data,缓冲区
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t i2c_smbus_read_data(uint8_t addr, int16_t len, uint8_t *data,uint8_t flag)
{
    struct i2c_msg_s msgs[] = {
        [0] = {addr, 0, data, len, I2C_M_RD|flag},
    };

    if ((i2c_transfer(msgs, 1)) != 1) {
        return -EIO;
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_stop()
* Description    : 停用I2C设备
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init i2c_stop(void)
{
    // 1.终止数据传输
    i2c_lld_stop();

    // 2.取消I2C初始化
    i2c_lld_deinit();

    // 3.CLK线拉9次脉冲
    i2c_clk_9pulse();//需要修改实现

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_phy_init()
* Description    : I2C物理接口初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline void i2c_phy_init(void)
{
    // 1.初始化I2C, Master
    i2c_lld_init();

    /* 2.开启I2C */
    i2c_lld_start(0);
}

/**************************************************************************************
* FunctionName   : i2c_ioctrl()
* Description    : I2C应用控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t i2c_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len > 0 ) || (NULL != args && len <= 0))){
        return -EINVAL;
    }

    // 1.执行命令序列
    switch(cmd){
    case _IOC_BUS_INIT:
        i2c_phy_init();
        break;
    case _IOC_BUS_TRANSPORTS:
        if((int16_t )(len / sizeof(struct i2c_msg_s)) != \
            i2c_transfer((void *)args, len / sizeof(struct i2c_msg_s))){
            return -EIO;
        }
        frtos_delay_ms(2);
        break;
    case _IOC_BUS_RESET:
        i2c_stop();
        i2c_phy_init();
        break;
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_suspend()
* Description    : i2c进入低功耗
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init i2c_lowpower(void)
{
    // 1.终止数据传输
    i2c_lld_stop();

    // 2.取消I2C初始化
    i2c_lld_deinit();

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_wakeup()
* Description    : i2c从低功耗唤醒
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init i2c_wakeup(void)
{
    // 1.初始化I2C, Master
    i2c_lld_init();

    /* 2.开启I2C */
    i2c_lld_start(0);

    return 0;
}

/**************************************************************************************
* FunctionName   : i2c_init()
* Description    : I2C设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init i2c_init(void)
{
    // 1.初始化物理接口
    i2c_phy_init();

    // 2.创建互斥访问锁
    i2c_mutex = mutex_lock_init();
    if(NULL == i2c_mutex) return -EPERM;

    return 0;
}

static __const struct driver spc_i2c = {
    .idx   = DRIVER_I2C,
    .name  = "i2c0",
    .init  = i2c_init,
    .ioctl = i2c_ioctrl,
    .stop  = i2c_stop,
    .lowpower = i2c_lowpower,
    .wakeup = i2c_wakeup,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(spc_i2c);
