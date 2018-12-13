/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : vehic.c
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
#include "frtos_sys.h"
#include "frtos_gpio.h"
#include "frtos_time.h"
#include "frtos_irq.h"
#include "data.pb-c.h"
#include "frtos_log.h"
#include "vehic.h"

/**************************************************************************************
* Description    : 定义上报相关信息
**************************************************************************************/
static uint32_t base_time;                              // 存储上次上报的时间
static uint32_t interval = UPLOAD_DEFAULT_TICK;         // 上报间隔，默认200ms

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static struct vehic speedinfo[] = {
    {
        .irq = SPEED1_IRQ,
    },{
        .irq = SPEED2_IRQ,
    }
};

/**************************************************************************************
* FunctionName   : vehic_config()
* Description    : 车辆配置配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t vehic_config(uint8_t idx, void *data, int32_t len)
{
    uint32_t i = 0;
    Subid *msg = NULL;
    Vehic *vehic = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < msg->n_subdata; i++) {
        vehic = vehic__unpack(NULL, msg->subdata[0].len, msg->subdata[0].data);
        if(unlikely(vehic == NULL))  break;

        if(msg->id == IOC__SET) {
            interval = vehic->interval;
        }
        vehic__free_unpacked(vehic, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : vehic_report()
* Description    : 数据上报回数据给MPU
* EntryParameter : args, gpio结构信息
* ReturnValue    : None
**************************************************************************************/
static void vehic_report(struct gpio *gpioinfo)
{
    uint8_t idx = 0;
    uint16_t pidbuf_len = 0;
    Subid msg = SUBID__INIT;
    struct piddata *pidata = NULL;
    ProtobufCBinaryData proto[ARRAY_LEN(speedinfo)];

    Vehic vehic = VEHIC__INIT;

    // 1.获取ACC状态信息
    if(fdrive_read(DRIVER_GPIO, (void *)gpioinfo, sizeof(struct gpio)) < 0) {
        return;
    }

    // 2.初始化Subid头部信息
    msg.id = IOC__DATA;
    msg.subdata = proto;
    msg.n_subdata = sizeof(proto)/sizeof(ProtobufCBinaryData);

    // 3.初始化车辆速度共同信息
    vehic.has_acc = 1;
    vehic.has_pulse = 1;
    vehic.has_total = 1;
    vehic.interval = interval;
    vehic.acc = gpioinfo->value;

    // 4.填充不同速度的记录值， 并打包到protobuf子结构vehic信息中
    for (idx = 0; idx < ARRAY_LEN(proto); idx++) {
        vehic.pulse = speedinfo[idx].pulse_freq;
        vehic.total = speedinfo[idx].total_pulse;

        proto[idx].len = vehic__get_packed_size(&vehic);
        proto[idx].data = mem_malloc(proto[idx].len);
        vehic__pack(&vehic, proto[idx].data);
    }

    // 5.根据需要申请传输需要的内存大小
    pidbuf_len = subid__get_packed_size(&msg);
    pidata = mem_malloc(pidbuf_len + sizeof(struct piddata));

    // 6.打包protobuf结构到进程间通信的数据结构
    pidata->id = VEHIC_PID;
    pidata->len = pidbuf_len;
    subid__pack(&msg, pidata->data);

    // 7.发送数据到串口通信模块
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

    // 8.释放申请的内存
    for (idx = 0; idx < ARRAY_LEN(proto); idx++) {
        mem_free(proto[idx].data);
    }
    mem_free(pidata);
}

/**************************************************************************************
* FunctionName   : vehic_run()
* Description    : vehic周期任务
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t vehic_run(void)
{
    struct gpio gpioinfo = {
        .gpio = GPIO_ACC,
    };

    if(true == time_after(int32_t, time_gettick(), base_time + interval)) {
        vehic_report(&gpioinfo);
        base_time = time_gettick();
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : pulse_calculate()
* Description    : 频率计数定时器
* EntryParameter : timer,定时指针
* ReturnValue    : None
**************************************************************************************/
static void pulse_calculate(int32_t pit_no)
{
    uint32_t pulse = 0;
    uint8_t i = 0, idx = 0;

    for (idx = 0; idx < ARRAY_LEN(speedinfo); idx++) {
        pulse = speedinfo[idx].pulse_cnt;
        // 1.计数器清零
        speedinfo[idx].pulse_cnt = 0;
        speedinfo[idx].pulse_freq = 0;

        // 2.填充计数器
        speedinfo[idx].pulse_arry[speedinfo[idx].pulse_index % PULSE_MAX] = pulse;
        speedinfo[idx].pulse_index++;

        // 3.记录脉冲总数
        speedinfo[idx].total_pulse += pulse;

        // 4.计算频率,1S内的pulse个数
        for(i = 0; i < PULSE_MAX; i++) {
            speedinfo[idx].pulse_freq += speedinfo[idx].pulse_arry[i];
        }
    }

    (void)pit_no;
}

/**************************************************************************************
* FunctionName   : vehic_isr()
* Description    : 车辆中断回调
* EntryParameter : irq,中断号
* ReturnValue    : None
**************************************************************************************/
static void vehic_isr(uint32_t irq)
{
    uint8_t idx = 0;

    // 脉冲计数流程
    for (idx = 0; idx < ARRAY_LEN(speedinfo); idx++) {
        if(speedinfo[idx].irq == irq) speedinfo[idx].pulse_cnt++;
    }
    (void)irq;
}

/**************************************************************************************
* TypeName       : vehic_cmd()
* Description    : vehic控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t vehic_cmd(char *argv, int32_t argc)
{
    uint8_t idx = 0;
    struct gpio gpioinfo = {
        .gpio = GPIO_ACC,
    };

    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("vehic show"STRBR);
    } else if (argc == 4 && 0 == strcmp(argv, "show")) {
        // 1.获取ACC状态信息
        if(fdrive_read(DRIVER_GPIO, (void *)&gpioinfo, sizeof(struct gpio)) < 0) {
            stdio_printf("can't get acc info"STRBR);
            return argc;
        }
        stdio_printf("\t acc level:%d"STRBR, gpioinfo.value);
        for (idx = 0; idx < ARRAY_LEN(speedinfo); idx++) {
            stdio_printf("\t speed%d: freq %d, total %d"STRBR, idx,
                    speedinfo[idx].pulse_freq,speedinfo[idx].total_pulse);
        }
    }

    return argc;
}

/**************************************************************************************
* FunctionName   : vehic_init()
* Description    : 车辆相关初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static __init int32_t vehic_init(void)
{
    uint8_t idx = 0;
    struct irq_reg_s irq;

    // 1.初始化结构信息
    for (idx = 0; idx < sizeof(speedinfo)/sizeof(struct vehic); idx++) {
        irq.trig = IRQ_TRIG_UP;
        irq.handler = vehic_isr;
        irq.irq = speedinfo[idx].irq;

        // 2.设置速度采样为中断模式
        fdrive_ioctl(DRIVER_GPIOINT, 0, &irq, sizeof(struct irq_reg_s));
    }

    // 3.设置定时器
    fdrive_ioctl(DRIVER_PIT1, _IOC_SET_CB, pulse_calculate, sizeof(pulse_calculate));
    base_time = time_gettick();

    return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite vehic = {
    .idx   = VEHIC_PID,
    .name  = "vehic",
    .init  = vehic_init,
    .run   = vehic_run,
    .set   = vehic_config,
    .debug = vehic_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(vehic);
