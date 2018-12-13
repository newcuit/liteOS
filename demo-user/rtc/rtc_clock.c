/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : rtc_clock.c
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
#include "frtos_sys.h"
#include "data.pb-c.h"
#include "frtos_log.h"
#include "frtos_utils.h"
#include "pcf85263.h"

/**************************************************************************************
* Description    : 模块定义消息数据缓存长度
**************************************************************************************/
#define MAX_RTC_MSG_LENGTH          100
#define MAX_PID_MSG_LENGTH          150

/**************************************************************************************
* FunctionName   : rtc_response_times()
* Description    : 时间信息发送给MPU
* EntryParameter : seconds,系统时间，单位s
* ReturnValue    : 返回处理的数据长度或者错误码
**************************************************************************************/
static int32_t rtc_response_times(uint32_t seconds)
{
    Rtc rtc = RTC__INIT;
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t rtc_msg_buf[MAX_RTC_MSG_LENGTH];
    uint8_t pid_msg_buf[MAX_PID_MSG_LENGTH];
    struct piddata *pidata = (struct piddata *)pid_msg_buf;

    // 1.读取需要上报的信息
    rtc.rtime = seconds;

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = rtc__get_packed_size(&rtc);
    proto.data = (uint8_t *)rtc_msg_buf;
    rtc__pack(&rtc, proto.data);

    // 3.准备发送protobuf打包
    pidata->id = RTC_PID;
    pidata->len = subid__get_packed_size(&msg);
    subid__pack(&msg, pidata->data);
    return fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));
}

/**************************************************************************************
 * FunctionName   : rtc_get()
 * Description    : 获取设备时间信息
 * EntryParameter : data，指向发送的数据， len,指向发送数据长度
 * ReturnValue    : 返回发送状态或者长度
 **************************************************************************************/
static int32_t rtc_get(uint8_t idx, void *data, int32_t len)
{
    uint8_t i = 0;
    Rtc *rtc = NULL;
    Subid *msg = NULL;
    time_sys_t seconds;
    pcf85263_dtime_t dtime;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL, len, data);
    if (unlikely(msg == NULL)) return -EFAULT;

    if (likely(msg->id == IOC__GET)) {
        fdrive_read(DRIVER_RTC,&seconds, sizeof(seconds));
        rtc_response_times(seconds);
        goto out;
    }

    for (i = 0; i < msg->n_subdata; i++) {
        rtc = rtc__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(rtc == NULL)) continue;

        if(msg->id == IOC__SET) {
            fdrive_write(DRIVER_RTC, &rtc->rtime, sizeof(rtc->rtime));
        } else if(msg->id == IOC__SETWAKE) {
            time_stm2dtm((time_sys_t)rtc->rtime, &dtime);
            fdrive_ioctl(DRIVER_RTC,_IOC_RTC_SETALARM,  &dtime, sizeof(dtime));
        }
        rtc__free_unpacked(rtc, NULL);
    }

out:
    // 3.释放内存
    subid__free_unpacked(msg, NULL);

    (void) idx;
    return len;
}

/**************************************************************************************
* TypeName       : rtc_cmd()
* Description    : rtc控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t rtc_cmd(char *argv, int32_t argc)
{
    int32_t ret = 0;
    time_sys_t seconds;
    pcf85263_dtime_t dtime;
    int year, mon, day, hour, min, sec;

    //1.设置rtc
    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("rtc set "STRBR);
        stdio_printf("rtc show"STRBR);
    } else if (0 == memcmp(argv, "set", 3)) {
        ret = mem_scannf(argv+4, argc-4, "%d-%d-%d %d:%d:%d",&year, &mon, &day, &hour, &min, &sec);
        if(ret <= 0) {
            stdio_printf("Invalid time format"STRBR);
            goto out;
        }

        dtime.mon = mon;
        dtime.day = day;
        dtime.hour = hour;
        dtime.min = min;
        dtime.sec = sec;
        dtime.year = year;
        time_dtm2stm(&dtime, &seconds);
        fdrive_write(DRIVER_RTC, &seconds, sizeof(seconds));
    } else if (argc == 4 && 0 == strcmp(argv, "show")) {
        fdrive_read(DRIVER_RTC,&seconds, sizeof(seconds));
        time_stm2dtm(seconds, &dtime);
        stdio_printf("%d-%d-%d %02d:%02d:%02d"STRBR, dtime.year, dtime.mon,
                dtime.day, dtime.hour, dtime.min, dtime.sec);
    }

out:
    return argc;
}

/**************************************************************************************
 * Description    : 定义通信任务结构
 **************************************************************************************/
static __const struct applite rtc = {
    .idx  = RTC_PID,
    .name = "rtc",
    .set  = rtc_get,
    .debug= rtc_cmd,
};

/**************************************************************************************
 * Description    : 模块初始化
 **************************************************************************************/
APP_REGISTER(rtc);
