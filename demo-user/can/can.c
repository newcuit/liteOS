/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : can.c
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
#include "data.pb-c.h"
#include "frtos_sys.h"
#include "spc_can.h"

/**************************************************************************************
* Description    : 定义ADC数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                 100
#define MAX_SUBPROTO_LEN             70
static uint8_t debug = 0;                           //can调试开关

/**************************************************************************************
* FunctionName   : can_recv()
* Description    : can数据串
* EntryParameter : data，CAN数据， len,gps数据长度
* ReturnValue    : 返回处理的数据长度或者错误码
**************************************************************************************/
static int32_t can_recv(int32_t idx, void *data, int32_t len)
{
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t buffer[MAX_DATA_LEN];
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    struct piddata *pidata = (struct piddata *)buffer;
    struct can_msg_s *canmsg = (struct can_msg_s *)data;

    Can can = CAN__INIT;

    if(unlikely(len != sizeof(struct can_msg_s))) return -EINVAL;

    // 1.读取需要上报的信息
    if(idx == DRIVER_CAN0) can.canid = 0;
    else if(idx == DRIVER_CAN1) can.canid = 1;
    else if(idx == DRIVER_CAN2) can.canid = 2;
    else can.canid = 3;

    can.id = canmsg->msgid;
    can.has_data = 1;
    can.data.data = canmsg->data;
    can.data.len = canmsg->len;

    if(unlikely(debug)) {
        stdio_printf("CAN%d recv %08X#%02X%02X%02X%02X%02X%02X%02X%02X"STRBR,can.canid,
                canmsg->msgid, canmsg->data[0],canmsg->data[1],canmsg->data[2],canmsg->data[3],
                canmsg->data[4],canmsg->data[5],canmsg->data[6],canmsg->data[7]);
    }

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = can__get_packed_size(&can);
    proto.data = bytedata;
    can__pack(&can, bytedata);

    // 3.准备发送protobuf打包
    pidata->id = CAN_PID;
    pidata->len = subid__get_packed_size(&msg);
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : can_send()
* Description    : CAN配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t can_send(uint8_t idx, void *data, int32_t len)
{
    Subid *msg = NULL;
    Can *can = NULL;
    uint8_t i = 0, canid = 0;
    struct can_msg_s msgbuf;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < msg->n_subdata; i++) {
        can = can__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(can == NULL))  break;

        if(can->canid == 0) canid = DRIVER_CAN0;
        else if(can->canid == 1) canid = DRIVER_CAN1;
        else if(can->canid == 2) canid = DRIVER_CAN2;
        else canid = DRIVER_CAN5;

        if(msg->id == IOC__SEND && can->has_data) {
            msgbuf.msgid = can->id;
            msgbuf.len = can->data.len;
            memcpy(msgbuf.data, can->data.data, can->data.len > 8? 8:can->data.len);
            if(unlikely(debug)) {
                stdio_printf("CAN%d send %08X#%08X"STRBR,can->canid,msgbuf.msgid, msgbuf.data);
            }
            if(unlikely(debug)) {
                stdio_printf("CAN%d send %08X#%02X%02X%02X%02X%02X%02X%02X%02X"STRBR,can->canid,
                        msgbuf.msgid, msgbuf.data[0],msgbuf.data[1],msgbuf.data[2],msgbuf.data[3],
                        msgbuf.data[4],msgbuf.data[5],msgbuf.data[6],msgbuf.data[7]);
            }
            fdrive_write(canid, &msgbuf, sizeof(struct can_msg_s));
        } else if(msg->id == IOC__FILTER) {
        }
        can__free_unpacked(can, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* TypeName       : can_cmd()
* Description    : can控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t can_cmd(char *argv, int32_t argc)
{
    //1.设置CAN的debug收发开关
    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("can debug on"STRBR);
        stdio_printf("can debug off"STRBR);
    } else if (argc == 8 && 0 == strcmp(argv, "debug on")) {
        debug = 1;
    } else if (argc == 9 && 0 == strcmp(argv, "debug off")) {
        debug = 0;
    }
    return argc;
}

/**************************************************************************************
* FunctionName   : can_init()
* Description    : CAN初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t can_init(void)
{
    fdrive_ioctl(DRIVER_CAN0, _IOC_SET_CB, can_recv, sizeof(can_recv));
    fdrive_ioctl(DRIVER_CAN1, _IOC_SET_CB, can_recv, sizeof(can_recv));
    fdrive_ioctl(DRIVER_CAN2, _IOC_SET_CB, can_recv, sizeof(can_recv));
    fdrive_ioctl(DRIVER_CAN5, _IOC_SET_CB, can_recv, sizeof(can_recv));

    return 0;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite can = {
    .idx   = CAN_PID,
    .name  = "can",
    .init  = can_init,
    .set   = can_send,
    .debug = can_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(can);
