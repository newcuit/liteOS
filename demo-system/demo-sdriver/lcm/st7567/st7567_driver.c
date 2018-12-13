/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : st7567_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_types.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "config_driver.h"
#include "frtos_lock.h"
#include "st7567_driver.h"
#include "st7567_font.h"

/**************************************************************************************
* Description    : 临界区定义
**************************************************************************************/
#define ST7567_ERITICAL_ENTER()     taskENTER_CRITICAL()
#define ST7567_ERITICAL_EXIT()      taskEXIT_CRITICAL()

/**************************************************************************************
* FunctionName   : st7567_trans()
* Description    : 发送时序
* EntryParameter : byte,要发送的字节, us,延时时序
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_trans(uint8_t byte, uint16_t us)
{
    uint8_t i = 0;

    st7567_gpio_val_set(ST7567_GPIO_CS, ST7567_HIGH);
    st7567_gpio_val_set(ST7567_GPIO_CLK, ST7567_HIGH);
    st7567_gpio_val_set(ST7567_GPIO_SID, ST7567_HIGH);
    st7567_delay_ns(us);

    st7567_gpio_val_set(ST7567_GPIO_CS, ST7567_LOW);
    st7567_delay_ns(us);
    for(i = 0; i < 8; i++){
        st7567_gpio_val_set(ST7567_GPIO_CLK, ST7567_LOW);
        st7567_delay_ns(us);
        if(byte & 0x80){
            st7567_gpio_val_set(ST7567_GPIO_SID, ST7567_HIGH);
        }else{
            st7567_gpio_val_set(ST7567_GPIO_SID, ST7567_LOW);
        }
        st7567_delay_ns(us);
        st7567_gpio_val_set(ST7567_GPIO_CLK, ST7567_HIGH);
        st7567_delay_ns(us);
        byte = (byte << 1);
    }
    st7567_gpio_val_set(ST7567_GPIO_CS, ST7567_HIGH);
}

/**************************************************************************************
* FunctionName   : st7567_trans_cmd()
* Description    : 发送命令
* EntryParameter : cmd,命令字
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_trans_cmd(uint8_t cmd)
{
    st7567_gpio_val_set(ST7567_GPIO_RS, ST7567_LOW);
    st7567_delay_ns(10);
    st7567_trans(cmd, 5);
    st7567_delay_ns(10);
}

/**************************************************************************************
* FunctionName   : st7567_trans_data()
* Description    : 发送数据
* EntryParameter : data,要发送的数据
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_trans_data(uint8_t data)
{
    st7567_gpio_val_set(ST7567_GPIO_RS, ST7567_HIGH);
    st7567_delay_ns(10);
    st7567_trans(data, 5);
    st7567_delay_ns(10);
}

/**************************************************************************************
* FunctionName   : st7567_addr_set()
* Description    : 设置显示地址
* EntryParameter : page,页地址(0~3), column,列地址(0~127)
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_addr_set(uint8_t page, uint8_t column)
{
    st7567_trans_cmd(0xB0 + page);
    st7567_trans_cmd(0x10 + ((column >> 4) & 0x0F));
    st7567_trans_cmd(column & 0x0F);
    st7567_delay_ns(10);
}

/**************************************************************************************
* FunctionName   : st7567_gpio_init()
* Description    : 硬件初始化函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_gpio_init(void)
{
    // 1.设置ST7567相关GPIO为输出
    st7567_gpio_dir_out(ST7567_GPIO_PWR, ST7567_HIGH);
    st7567_gpio_dir_out(ST7567_GPIO_VLED, ST7567_HIGH);
    st7567_gpio_dir_out(ST7567_GPIO_RST, ST7567_LOW);
    st7567_gpio_dir_out(ST7567_GPIO_CS, ST7567_LOW);
    st7567_gpio_dir_out(ST7567_GPIO_RS, ST7567_LOW);
    st7567_gpio_dir_out(ST7567_GPIO_CLK, ST7567_LOW);
    st7567_gpio_dir_out(ST7567_GPIO_SID, ST7567_LOW);

    // 2.设置按键相关GPIO为输入
    st7567_gpio_dir_in(ST7567_GPIO_K1);
    st7567_gpio_dir_in(ST7567_GPIO_K2);
    st7567_gpio_dir_in(ST7567_GPIO_K3);
    st7567_gpio_dir_in(ST7567_GPIO_K4);
}

/**************************************************************************************
* FunctionName   : st7567_para_init()
* Description    : 显示参数初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_para_init(void)
{
    #if 0
    st7567_trans_cmd(0xe2);        // 软件复位
    st7567_delay_ms(1);

    st7567_trans_cmd(0x2c);        // 打开内部升压
    st7567_trans_cmd(0x2e);        // 打开电压调整电路
    #endif

    st7567_trans_cmd(0x2f);        // 打开电压跟随器

    st7567_trans_cmd(0x24);        // 设置显示浓度(0x20 ~ 0x27)
    st7567_trans_cmd(0x81);        // 内部设置电压模式
    st7567_trans_cmd(0x1f);        // 设置电压值

    st7567_trans_cmd(0xa2);        // 设置偏压比
    st7567_trans_cmd(0xc8);        // 设置扫描顺序(从下到上)
    st7567_trans_cmd(0xa0);        // 显示列地址增减(从左到右)
    st7567_trans_cmd(0x40);        // 设置初始显示行
    st7567_trans_cmd(0xa6);        // 显示正/反显(正显)
    st7567_trans_cmd(0xaf);        // 打开显示开关
}

/**************************************************************************************
* FunctionName   : st7567_phy_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static inline void st7567_device_init(void)
{
    // 1.初始化GPIO
    st7567_gpio_init();

    // 2.打开电源
    st7567_ctrl_pwr(ST7567_ON);

    // 3.打开背光
    st7567_ctrl_led(ST7567_ON);

    // 4.设备复位
    st7567_reset();

    // 5.初始参数
    st7567_para_init();

    // 6.清屏
    st7567_screen_clear();
}

/**************************************************************************************
* FunctionName   : st7567_write_dot()
* Description    : 写点阵数据
* EntryParameter : dot,要显示的点阵数据
* ReturnValue    : 0,成功, -1,失败
**************************************************************************************/
int8_t st7567_write_dot(const struct st7567_dot_s *dot)
{
    uint8_t i = 0, j = 0;
    uint16_t n = 0;
    if(dot->count > ST7567_WIDE_MAX*ST7567_HIGH_MAX/8){
        return -1;
    }

    // 1.进入临界区
    ST7567_ERITICAL_ENTER();

    // 2.初始化显示参数
    st7567_para_init();

    // 3.写数据
    for(i = 0; i < 8; i++){
        st7567_addr_set(dot->addr.page + i, dot->addr.column);
        for(j = 0; j < dot->wide; j++){
            st7567_trans_data(dot->data[n++]);
            if(n >= dot->count){goto RETURN_1;}
        }
    }

    // 4.退出临界区
    RETURN_1:
    ST7567_ERITICAL_EXIT();

    return 0;
}

/**************************************************************************************
* FunctionName   : st7567_screen_clear()
* Description    : 显示清屏
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void st7567_screen_clear(void)
{
    uint8_t i = 0, j = 0;

    for(i = 0; i < 4; i++){
        st7567_addr_set(i, 0);
        for(j = 0; j < 128; j++){
            st7567_trans_data(0x00);
        }
    }
}

/**************************************************************************************
* FunctionName   : st7567_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
**************************************************************************************/
static int32_t st7567_write(uint8_t idx, void *data, int32_t len)
{
    if(unlikely(NULL != data && len != sizeof(struct st7567_dot_s))){
        return -EINVAL;
    }

    // 1.写数据
    if(0 != st7567_write_dot(data))return -EIO;

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : st7567_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t st7567_ioctrl(uint8_t idx,int32_t cmd, void *args,int32_t len)
{
    if(unlikely((NULL == args && len > 0) || \
        (NULL != args && len > 0) || len < 0)){
        return -EINVAL;
    }

    // 1.执行控制命令
    switch(cmd){
    case _IOC_PWR_ON:
        st7567_ctrl_pwr(ST7567_ON);
        break;
    case _IOC_PWR_OFF:
        st7567_ctrl_pwr(ST7567_OFF);
        break;
    case _IOC_LED_ON:
        st7567_ctrl_led(ST7567_ON);
        break;
    case _IOC_LED_OFF:
        st7567_ctrl_led(ST7567_OFF);
        break;
    case _IOC_BUS_INIT:
        st7567_device_init();
        break;
    case _IOC_BUS_RESET:
        st7567_reset();
        break;
    case _IOC_CLEAR:
        st7567_screen_clear();
        break;
    case _IOC_SET_CB:
        break;
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : st7567_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init st7567_init(void)
{
    struct st7567_addr_s addr = {1, 0};

    // 1.初始化设备
    st7567_device_init();
    //st7567_font_write_utf8_str(&addr, (uint8_t *)__TIME__);
    //st7567_delay_ms(3000);

    st7567_screen_clear();
    st7567_font_write_utf8_str(&addr, (uint8_t *)"系统启动中...");

    return 0;
}

static __const struct driver s32k_st7567 = {
    .idx   = DRIVER_LCD,
    .name  = "lcd",
    .init  = st7567_init,
    .write = st7567_write,
    .ioctl = st7567_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
CORE_INIT(s32k_st7567);
