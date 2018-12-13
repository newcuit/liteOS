/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : lcm.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "gpio_driver.h"
#include "st7567_driver.h"
#include "frtos_irq.h"
#include "data.pb-c.h"
#include "frtos_gpio.h"
#include "frtos_tasklet.h"
#include "frtos_sys.h"

/**************************************************************************************
* Description   : 定义lcm key键的中断号
**************************************************************************************/
#define ST7567_K1_IRQ           135
#define ST7567_K2_IRQ           6
#define ST7567_K3_IRQ           7
#define ST7567_K4_IRQ           72

/**************************************************************************************
* Description    : 定义LCM上报数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                        200
#define MAX_SUBPROTO_LEN                    50
static struct workqueue lcm_work[4];

/**************************************************************************************
* FunctionName   : lcd_config()
* Description    : LCD显示
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t lcd_config(uint8_t idx, void *data, int32_t len)
{
    uint32_t i = 0;
    Subid *msg = NULL;
    Lcd *lcd = NULL;
    struct st7567_dot_s dot;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    switch(msg->id) {
    case IOC__POWERON: fdrive_ioctl(DRIVER_LCD, _IOC_PWR_ON, NULL, 0); goto out;
    case IOC__LEDON: fdrive_ioctl(DRIVER_LCD, _IOC_LED_ON, NULL, 0);goto out;
    case IOC__PWROFF: fdrive_ioctl(DRIVER_LCD, _IOC_PWR_OFF, NULL, 0); goto out;
    case IOC__LEDOFF: fdrive_ioctl(DRIVER_LCD, _IOC_LED_OFF, NULL, 0);goto out;
    case IOC__INIT: fdrive_ioctl(DRIVER_LCD, _IOC_BUS_INIT, NULL, 0);goto out;
    case IOC__CLEAR: fdrive_ioctl(DRIVER_LCD, _IOC_CLEAR, NULL, 0);goto out;
    default:break;
    }

    for (i = 0; i < msg->n_subdata; i++) {
        lcd = lcd__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(lcd == NULL)) break;

        if(likely(msg->id == IOC__DATA && lcd->type)) {
            dot.addr.column = lcd->data->column;
            dot.addr.page = lcd->data->page;
            dot.count = lcd->data->screen.len;
            dot.high = lcd->data->high;
            dot.wide = lcd->data->wide;
            memcpy(dot.data, lcd->data->screen.data, lcd->data->screen.len);

            fdrive_write(DRIVER_LCD, &dot, sizeof(struct st7567_dot_s));
        }
        lcd__free_unpacked(lcd, NULL);
    }
out:
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : lcm_report()
* Description    : lcm回数据给MPU
* EntryParameter : args, gpio结构信息
* ReturnValue    : None
**************************************************************************************/
static void lcm_report(void *gpio)
{
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t buffer[MAX_DATA_LEN];
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    struct piddata *pidata = (struct piddata *)buffer;

    Lcd lcd = LCD__INIT;
    Lcd__Button key = LCD__BUTTON__INIT;
    struct gpio io = {.gpio = (uint32_t)gpio, .value = 0};

    // 1.初始化protobuf具体数据信息
    if(fdrive_read(DRIVER_GPIO, (void *)&io, sizeof(struct gpio)) < 0) return;

    key.id = io.gpio;
    key.value = !io.value;
    lcd.key = &key;
    lcd.type = 0;

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = lcd__get_packed_size(&lcd);
    proto.data = bytedata;
    lcd__pack(&lcd, bytedata);

    // 3.准备发送protobuf打包
    pidata->id = LCM_PID;
    pidata->len = subid__get_packed_size(&msg);
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));
}

/**************************************************************************************
* FunctionName   : lcm_isr()
* Description    : lcm按钮中断回调
* EntryParameter : irq,中断号
* ReturnValue    : None
**************************************************************************************/
static void lcm_isr(uint32_t irq)
{
    int8_t idx = -1;

    switch(irq) {
    case ST7567_K1_IRQ:idx = 0;break;
    case ST7567_K2_IRQ:idx = 1;break;
    case ST7567_K3_IRQ:idx = 2;break;
    case ST7567_K4_IRQ:idx = 3;break;
    }
    // 当前程序在中断函数当中， 延迟处理交给中断下半部分
    if(likely(idx >= 0)) {
        tasklet_schedule(&lcm_work[idx], (void *)irq, 5);
    }
}

/**************************************************************************************
* FunctionName   : lcm_init()
* Description    : LCD初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t lcm_init(void)
{
    uint8_t i = 0;
    struct irq_reg_s irq;

    irq.trig = IRQ_TRIG_EDGE | IRQ_TRIG_SHARED;
    irq.handler = lcm_isr;

    for (i = 0; i < 4; i++) {
        tasklet_init(&lcm_work[i], lcm_report);
    }

    irq.irq = ST7567_K1_IRQ;
    fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));

    irq.irq = ST7567_K2_IRQ;
    fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));

    irq.irq = ST7567_K3_IRQ;
    fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));

    irq.irq = ST7567_K4_IRQ;
    fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));

    return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite lcm = {
    .idx   = LCM_PID,
    .name  = "lcd",
    .init  = lcm_init,
    .set   = lcd_config,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(lcm);
