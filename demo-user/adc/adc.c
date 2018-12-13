/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : adc.c
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
#include "data.pb-c.h"
#include "spcadc_driver.h"
#include "frtos_time.h"
#include "frtos_log.h"
#include "math.h"

/**************************************************************************************
* Description    : 定义ADC数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                       100  // 定义发送消息数据缓存长度
#define MAX_SUBPROTO_LEN                   50   // 定义发送消息数据缓存长度
#define ADC_MAX_COUNT                      18   // 定义最大ADC通道个数
#define ADC_UPLOAD_CHANGE                  100  // 定义默认变化电压上报值

/**************************************************************************************
* Description    : 定义上报相关信息
**************************************************************************************/
#ifdef UPLOAD_DELAY
static uint32_t base_time;                      // 存储上次上报的时间
static uint32_t interval = ADC_UPLOAD_CHANGE;   // 上报间隔，默认1000ms
#endif

/**************************************************************************************
* Description    : 定义ADC采样参数
**************************************************************************************/
static struct {
    uint32_t change;                            // 满足变化值，则上报, 单位v
    int32_t voltage;                            // 当前电压值(单位mv)
} adc_sample[ADC_MAX_COUNT];

/**************************************************************************************
* FunctionName   : adc_config()
* Description    : ADC配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t adc_config(uint8_t idx, void *data, int32_t len)
{
    uint32_t i = 0;
    Subid *msg = NULL;
    Voltage *adc = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < msg->n_subdata; i++) {
        adc = voltage__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(adc == NULL)) break;

        if(adc->id > 0 && adc->id <= ADC_MAX_COUNT) {
            if(msg->id == IOC__SET && adc->has_value) {
                adc_sample[adc->id - 1].change = adc->value;
            } else if(msg->id == IOC__GET) {
                // 这里设置成-ADC_UPLOAD_CHANGE， 后面周期运行任务里面会自动上报一次电压
                adc_sample[adc->id - 1].voltage = -ADC_UPLOAD_CHANGE;
            }
        }
        voltage__free_unpacked(adc, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : adc_run()
* Description    : adc周期任务
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t adc_run(void)
{
    static uint8_t idx = 0;
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t buffer[MAX_DATA_LEN];
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    struct piddata *pidata = (struct piddata *)buffer;

    Voltage vol = VOLTAGE__INIT;
    static struct spcadc_samp_s adc[] = {
            {1,  0,  15,    9, 0},
            {1,  1,  20, 0.81, 0},
            {1,  2,  20, 0.81, 0},
            {1,  3,  20, 0.81, 0},
            {1,  4,  20, 0.81, 0},
            {1,  5,  20, 0.81, 0},
            {1,  6,  20, 0.81, 0},
            {1,  7,  20, 0.81, 0},
            {1,  8,  20, 0.81, 0},
            {1,  9,  20, 0.81, 0},
            {1, 10,  20, 0.81, 0},
            {1, 11,  20, 0.81, 0},
            {1, 12,  20, 0.81, 0},
            {1, 13,  20, 0.81, 0},
            {1, 14, -15, 1.70, 0},
            {1, 15,  20, 0.81, 0},
            {0,  5, -10, 3.22, 0},
            {0,  6, -10, 3.22, 0},
    };

#ifdef UPLOAD_DELAY
    // 1.判断是否到上报时间
    if(false == time_after(int32_t, time_gettick(), base_time+interval)) return 0;
#endif
    // 2.读取需要上报的信息
    if(idx <= 15){
        if(fdrive_read(DRIVER_ADC, &adc[idx], sizeof(struct spcadc_samp_s)) < 0) return -EBUSY;
    }else{
        switch(idx){
        case 16:
            if(fdrive_read(DRIVER_ADC, &adc[idx], sizeof(struct spcadc_samp_s)) < 0) return -EBUSY;break;
        case 17:
            if(fdrive_read(DRIVER_ADC, &adc[idx], sizeof(struct spcadc_samp_s)) < 0) return -EBUSY;break;
        }
    }
    // 3.分析ADC数据， 变化值超过阈值则上报
        // 3.1.满足条件上报
    if(adc_sample[idx].change < fabs(adc[idx].result - adc_sample[idx].voltage)) {
        vol.value = adc[idx].result;
        vol.has_value = 1;
        // 3.2. 主电源ID是1， 电池是2, 对应的id为0， 1
        vol.id = idx + 1;
        adc_sample[idx].voltage = adc[idx].result;
        // 3.3.填充应答protobuf 子ID信息
        msg.id = IOC__DATA;
        msg.n_subdata = 1;
        msg.subdata = &proto;
        proto.len = voltage__get_packed_size(&vol);
        proto.data = bytedata;
        voltage__pack(&vol, bytedata);
        // 3.4.准备发送protobuf打包
        pidata->id = ADC_PID;
        pidata->len = subid__get_packed_size(&msg);
        subid__pack(&msg, pidata->data);
        fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));
    }
#ifdef UPLOAD_DELAY
    // 4.重新获取基准时间
    base_time = time_gettick();
#endif

    idx++;
    if(idx == 18) idx = 0;
    return 0;
}

/**************************************************************************************
* TypeName       : adc_cmd()
* Description    : adc控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t adc_cmd(char *argv, int32_t argc)
{
    uint8_t idx = 0;
    struct spcadc_samp_s adc[] = {
            {1,  0,  15,    9, 24},
            {1,  1,  20, 0.81, 0},
            {1,  2,  20, 0.81, 0},
            {1,  3,  20, 0.81, 0},
            {1,  4,  20, 0.81, 0},
            {1,  5,  20, 0.81, 0},
            {1,  6,  20, 0.81, 0},
            {1,  7,  20, 0.81, 0},
            {1,  8,  20, 0.81, 0},
            {1,  9,  20, 0.81, 0},
            {1, 10,  20, 0.81, 0},
            {1, 11,  20, 0.81, 0},
            {1, 12,  20, 0.81, 0},
            {1, 13,  20, 0.81, 0},
            {1, 14, -15, 1.70, 0},
            {1, 15,  20, 0.81, 0},
            {0,  5, -10, 3.22, 0},
            {0,  6, -10, 3.22, 0},
    };

    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("adc show"STRBR);
    } else if (argc == 4 && 0 == strcmp(argv, "show")) {
        // 1.读取需要上报的信息
        if(fdrive_read(DRIVER_ADC, adc, sizeof(adc)) < 0) {
            stdio_printf("adc can't read"STRBR);
        } else {
            // 2.分析ADC数据， 变化值超过一定 则上报
            for (idx = 0; idx < ARRAY_LEN(adc); idx++) {
                stdio_printf("adc%d ch%d sample %dmv"STRBR,adc[idx].adc, adc[idx].ch, adc[idx].result);
            }
        }
    }
    return argc;
}

/**************************************************************************************
* FunctionName   : adc_init()
* Description    : adc初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t adc_init(void)
{
    uint8_t idx = 0;

    // 1.设置ADC默认电压变化上报值
    for (idx = 0; idx < ADC_MAX_COUNT; idx++) {
        adc_sample[idx].change = ADC_UPLOAD_CHANGE;
    }

#ifdef UPLOAD_DELAY
    // 2.获取基准时间
    base_time = time_gettick();
#endif
    return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite adc = {
    .idx   = ADC_PID,
    .name  = "adc",
    .init  = adc_init,
    .run   = adc_run,
    .set   = adc_config,
    .debug = adc_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(adc);
