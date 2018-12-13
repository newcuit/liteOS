/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : sensor.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "frtos_sys.h"
#include "frtos_irq.h"
#include "frtos_time.h"
#include "frtos_log.h"
#include "frtos_ioctl.h"
#include "config_user.h"
#include "config_driver.h"
#include "gpio_driver.h"
#include "data.pb-c.h"
#include "lsm6dsl.h"

/**************************************************************************************
* Description    : 定义IC数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                 120
#define MAX_SUBPROTO_LEN             100
#define SENSOR_MAX                   5

/**************************************************************************************
* Description    : 定义IC采样参数
**************************************************************************************/
static struct {
    uint32_t base;                                // 上次上传时间
    uint32_t interval;                            // 上报间隔
} sample_sensor = {
    .interval = 500,                              // 上报间隔 默认500ms
};

/**************************************************************************************
* FunctionName   : sensor_config()
* Description    : 传感器配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t sensor_config(uint8_t idx, void *data, int32_t len)
{
    uint32_t i = 0;
    Subid *msg = NULL;
    Gsensor *sensor = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for(i = 0; i < msg->n_subdata; i++) {
        sensor = gsensor__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(sensor == NULL))  break;

        if(likely(msg->id == IOC__SET)) {
            sample_sensor.interval = sensor->interval;
            fdrive_ioctl(DRIVER_SENSOR, _IOC_SET, &(sensor->threshold), sizeof(int32_t));
        }
        gsensor__free_unpacked(sensor, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : sensor_run()
* Description    : sensor周期任务
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t sensor_run(void)
{
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t buffer[MAX_DATA_LEN];
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    static uint8_t sensor_failed = 0;
    struct piddata *pidata = (struct piddata *)buffer;

    Gsensor sensor = GSENSOR__INIT;
    Gsensor__Accel accel = GSENSOR__ACCEL__INIT;
    Gsensor__Gyro gyro = GSENSOR__GYRO__INIT;
    acc_data_s acc_data;
    gyro_data_s gyro_data;

    if(true != time_after(int32_t, time_gettick(),
            sample_sensor.base+sample_sensor.interval))
        goto out;

    // 0.填充本次上报的时间
    sample_sensor.base = time_gettick();

    // 1.初始化protobuf具体数据信息
    memset(&acc_data, 0, sizeof(acc_data_s));
    memset(&gyro_data, 0, sizeof(gyro_data_s));
    if(fdrive_ioctl(DRIVER_SENSOR, _IOC_GET_DATA1, &acc_data,
            sizeof(acc_data_s)) < 0) {
        sensor_failed++;
    }
    if(fdrive_ioctl(DRIVER_SENSOR, _IOC_GET_DATA2, &gyro_data,
            sizeof(gyro_data_s)) < 0){

        sensor_failed++;
    }

    if(unlikely(sensor_failed >= SENSOR_MAX)) {
        fdrive_ioctl(DRIVER_I2C, _IOC_BUS_RESET, NULL, 0);
        stdio_printf("Found i2c fault, reset it !!!!"STRBR);
        sensor_failed = 0;
    }

    accel.x = acc_data.acc_x;
    accel.y = acc_data.acc_y;
    accel.z = acc_data.acc_z;

    gyro.x = gyro_data.gyro_x;
    gyro.y = gyro_data.gyro_y;
    gyro.z = gyro_data.gyro_z;

    sensor.a = &accel;
    sensor.g = &gyro;

    sensor.has_event = 1;
    sensor.event = acc_data.collision;

    if(unlikely(acc_data.collision)) {
        stdio_printf("Collision detected !!!!"STRBR);
    }

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = gsensor__get_packed_size(&sensor);
    proto.data = bytedata;
    gsensor__pack(&sensor, bytedata);

    // 3.准备发送protobuf打包
    pidata->id = SENSOR_PID;
    pidata->len = subid__get_packed_size(&msg);
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

out:
    return 0;
}

/**************************************************************************************
* TypeName       : sensor_cmd()
* Description    : sensor控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t sensor_cmd(char *argv, int32_t argc)
{
    acc_data_s acc_data;
    gyro_data_s gyro_data;

    //1.设置sensor的debug收发开关
    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("sensor show"STRBR);
    } else if (argc == 4 && 0 == strcmp(argv, "show")) {
        fdrive_ioctl(DRIVER_SENSOR, _IOC_GET_DATA1, &acc_data,sizeof(acc_data_s));
        fdrive_ioctl(DRIVER_SENSOR, _IOC_GET_DATA2, &gyro_data,sizeof(gyro_data_s));

        stdio_printf("\t acc_x:%d, acc_y:%d, acc_z:%d"STRBR,
                acc_data.acc_x, acc_data.acc_y, acc_data.acc_z);
        stdio_printf("\t gyro_x:%d, gyro_y:%d, gyro_z:%d"STRBR,
                gyro_data.gyro_x, gyro_data.gyro_y, gyro_data.gyro_z);
    }
    return argc;
}

/**************************************************************************************
* FunctionName   : sensor_init()
* Description    : 传感器初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t sensor_init(void)
{
    sample_sensor.base = time_gettick();
    return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite sensor = {
    .idx   = SENSOR_PID,
    .name  = "sensor",
    .init  = sensor_init,
    .run   = sensor_run,
    .set   = sensor_config,
    .debug = sensor_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(sensor);
