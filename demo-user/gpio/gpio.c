/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : gpio.c
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
#include "frtos_gpio.h"
#include "frtos_irq.h"
#include "frtos_tasklet.h"
#include "data.pb-c.h"
#include "frtos_sys.h"
#include "frtos_utils.h"
#include "power_driver.h"
#include "frtos_log.h"
#include "gpio.h"

/**************************************************************************************
* Description    : 定义GPIO数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                        512
#define MAX_SUBPROTO_LEN                    50
#define MAX_GPIO_NUMBER                     3
static struct workqueue gpio_work[MAX_GPIO_NUMBER];

/**************************************************************************************
* FunctionName   : gpio_get()
* Description    : GPIO获取
* EntryParameter : gpiodata,gpio记录信息
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static __used int32_t gpio_get(Gpio *gdata, uint32_t gpio)
{
    struct gpio data;

    data.gpio = gpio;
    fdrive_read(DRIVER_GPIO, (void *)&data, sizeof(struct gpio));
    gdata->gpio = gpio;
    gdata->has_value = 1;
    gdata->value = data.value;

    return 0;
}

/**************************************************************************************
* FunctionName   : gpio_set()
* Description    : GPIO设置
* EntryParameter : gpiodata,gpio记录信息
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static __used int32_t gpio_set(uint32_t gpio, uint32_t value)
{
    struct gpio data;

    data.gpio = gpio;
    data.value = value;

    return fdrive_write(DRIVER_GPIO, (void *)&data, sizeof(struct gpio));
}

/**************************************************************************************
* FunctionName   : gpio_config()
* Description    : GPIO配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t gpio_config(uint8_t idx, void *data, int32_t len)
{
    int32_t i = 0;
    Subid *msg = NULL, reponse = SUBID__INIT;
    uint8_t buffer[MAX_DATA_LEN];
    struct piddata *pidata = (struct piddata *)buffer;

    // 1.解开gpio通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    // 2.填充应答protobuf 子ID信息
    reponse.id = IOC__DATA;
    reponse.n_subdata = 0;
    reponse.subdata = mem_malloc(msg->n_subdata * sizeof(ProtobufCBinaryData));

    // 3.解析gpio protobuf协议分组
    for (i = 0; i < (int32_t)msg->n_subdata; i++) {
        Gpio gdata = GPIO__INIT;
        Gpio *gpiodata = gpio__unpack(NULL, msg->subdata[i].len, \
                msg->subdata[i].data);

        if(unlikely(gpiodata == NULL)) break;


        switch(msg->id) {
        case IOC__SET: gpio_set(gpiodata->gpio, gpiodata->has_value?gpiodata->value:0); break;
        case IOC__GET: gpio_get(&gdata, gpiodata->gpio); break;
        }

        // 释放GPIO资源
        gpio__free_unpacked(gpiodata, NULL);

        // 如果id是GPIO__GET,则需要应答， 否则continue
        if(msg->id != IOC__GET) continue;

        // 打包需要发送的数据
        reponse.subdata[reponse.n_subdata].len = gpio__get_packed_size(&gdata);
        reponse.subdata[reponse.n_subdata].data = mem_malloc(reponse.subdata[reponse.n_subdata].len);
        gpio__pack(&gdata, reponse.subdata[reponse.n_subdata].data);

        reponse.n_subdata++;
    }

    // 4.如果需要应答，则到out
    subid__free_unpacked(msg, NULL);
    if(unlikely(reponse.n_subdata == 0)) goto out;

    // 6.数据发送给INIT_PID程序
    pidata->id = GPIO_PID;
    pidata->len = subid__get_packed_size(&reponse);
    subid__pack(&reponse, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

    // 7.内存释放
    for (i = 0; i < (int32_t)reponse.n_subdata; i++) {
        mem_free(reponse.subdata[i].data);
    }
out:
    mem_free(reponse.subdata);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : gpio_report()
* Description    : GPIO回数据给MPU
* EntryParameter : args, gpio结构信息
* ReturnValue    : None
**************************************************************************************/
static void gpio_report(void *gpio)
{
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t buffer[MAX_DATA_LEN];
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    struct piddata *pidata = (struct piddata *)buffer;

    Gpio gpiodata = GPIO__INIT;
    struct gpio io = {.gpio = (uint32_t)gpio, .value = 0};

    // 1.初始化protobuf具体数据信息
    if(fdrive_read(DRIVER_GPIO, (void *)&io, sizeof(struct gpio)) < 0) return;
    gpiodata.gpio = io.gpio;
    gpiodata.has_value = 1;
    gpiodata.value = io.value;

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = gpio__get_packed_size(&gpiodata);
    proto.data = bytedata;
    gpio__pack(&gpiodata, bytedata);

    // 3.准备发送protobuf打包
    pidata->id = GPIO_PID;
    pidata->len = subid__get_packed_size(&msg);
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));
}

/**************************************************************************************
* FunctionName   : gpio_isr()
* Description    : GPIO中断回调
* EntryParameter : irq,中断号
* ReturnValue    : None
**************************************************************************************/
static void gpio_isr(uint32_t irq)
{
    int gpio = 0;
    int8_t idx = -1;

    // 当前程序在中断函数当中， 延迟处理交给中断下半部分
    switch(irq) {
    case GPIO_SRS_IRQ:idx = 0;gpio = GPIO_SRS;break;
    case GPIO_IN_ON_IN_IRQ:idx = 1;gpio = GPIO_IN_ON_IN;break;
    }
    if(likely(idx >= 0)) {
        tasklet_schedule(&gpio_work[idx], (void *)gpio, 1);
    }
}

/**************************************************************************************
* FunctionName   : acc_isr()
* Description    : 中断服务函数
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
void acc_isr(int8_t irq, uint8_t status)
{
    (void)status;(void)irq;
    tasklet_schedule(&gpio_work[2], (void *)GPIO_ACC, 1);
}

/**************************************************************************************
* TypeName       : gpio_cmd()
* Description    : gpio控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t gpio_cmd(char *argv, int32_t argc)
{
    Gpio val1;
    int32_t val = 0;
    int32_t gpio = 0;

    //1.设置gpio 设置
    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("gpio get gpio"STRBR);
        stdio_printf("gpio set gpio"STRBR);
        return argc;
    }

    if (0 == memcmp(argv, "get", 3)) {
        if(mem_scannf(argv, argc, "get %d", &gpio) > 0) {
            gpio_get(&val1, gpio);
            stdio_printf("gpio%d=%d"STRBR, gpio, val1.value);
        } else {
            stdio_printf("Invalid gpio"STRBR);
        }
    } else if (0 == memcmp(argv, "set", 3)) {
        if(mem_scannf(argv, argc, "set %d %d", &gpio, &val) > 0) {
            gpio_set(gpio, val);
        } else {
            stdio_printf("Invalid gpio parameter"STRBR);
        }
    } else {
        stdio_printf("Invalid gpio parameter"STRBR);
    }
    return argc;
}

/**************************************************************************************
* FunctionName   : gpio_init()
* Description    : GPIO初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t gpio_init(void)
{
    uint8_t i = 0;
    struct irq_reg_s irq = {
        .trig = IRQ_TRIG_EDGE,
	    .handler = gpio_isr,
    };

    for (i = 0; i < MAX_GPIO_NUMBER; i++) {
        tasklet_init(&gpio_work[i], gpio_report);
    }

    irq.irq = GPIO_SRS_IRQ;
    fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));

    irq.irq = GPIO_IN_ON_IN_IRQ;
    fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));

    return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite gpio = {
    .idx   = GPIO_PID,
    .name  = "gpio",
    .init  = gpio_init,
    .set   = gpio_config,
    .debug = gpio_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(gpio);
