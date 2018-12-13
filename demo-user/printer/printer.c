/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : printer.c
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
#include "frtos_list.h"
#include "frtos_errno.h"
#include "frtos_delay.h"
#include "frtos_ioctl.h"
#include "gpio_driver.h"
#include "data.pb-c.h"
#include "frtos_sys.h"
#include "printer.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
struct tp_inode tp_data[BUFFER_MAX] = {
    [WAIT_BUFFER] = {
        .len = 0,
        .data = NULL,
    },
    [RUNNING_BUFFER] = {
        .len    = 0,
        .data   = NULL,
    }
};

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static mutex_lock_t printer_mutex = NULL;           // 打印机访问锁

/**************************************************************************************
* Description    : 定义局部栈使用大小
**************************************************************************************/
#define MAX_DATA_LEN                100
#define MAX_SUBPROTO_LEN            30

/**************************************************************************************
* FunctionName   : tp_ack_init()
* Description    : 初始化应答proto-buf报文
* EntryParameter : pidata，指向应答数据的proto格式报文， len,当前piddata空间最大长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t tp_ack_init(uint8_t type, uint8_t *buffer, uint16_t len)
{
    Ack ack = ACK__INIT;
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    struct piddata *pidata = (struct piddata *)buffer;

    // 1. 组装应答报文头部
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;

    // 2. 设置应答号，暂时都为1
    ack.has_rev1 = 0;
    ack.type = type;

    proto.len = ack__get_packed_size(&ack);
    proto.data = bytedata;
    ack__pack(&ack, bytedata);

    // 3. 准备发送protobuf打包
    pidata->id = PRINT_PID;
    pidata->len = subid__get_packed_size(&msg);

    if(pidata->len > len - sizeof(struct piddata)) return -ENOMEM;

    subid__pack(&msg, pidata->data);

    return pidata->len + sizeof(struct piddata);
}

/**************************************************************************************
* FunctionName   : tp_add()
* Description    : tp数据添加到队列
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t tp_add(uint8_t idx, void *data, int32_t len)
{
    uint32_t i = 0;
    int16_t buflen = 0;
    Subid *msg = NULL;
    Printer *pdata = NULL;
    uint8_t buffer[MAX_DATA_LEN];

    // 0.应答MPU，报文已收到
    buflen = tp_ack_init(1, buffer, MAX_DATA_LEN);
    fuser_data_set(INIT_PID, buffer, buflen);

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < msg->n_subdata; i++) {
        pdata = printer__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(pdata == NULL)) break;

        mutex_lock(printer_mutex);
        if(likely(msg->id == IOC__DATA && tp_data[WAIT_BUFFER].data == NULL)) {
            tp_data[WAIT_BUFFER].data = mem_malloc(pdata->print.len);
            if(unlikely(tp_data[WAIT_BUFFER].data == NULL)) goto print_free;

            tp_data[WAIT_BUFFER].len = pdata->print.len;
            memcpy(tp_data[WAIT_BUFFER].data, pdata->print.data, pdata->print.len);
        }
print_free:
        mutex_unlock(printer_mutex);
        printer__free_unpacked(pdata, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : tp_run()
* Description    : tp打印机打印数据
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static void tp_run(void *args)
{
    int16_t buflen = 0;
    uint8_t buffer[MAX_DATA_LEN];

    // 0.获取应答报文信息结构
    buflen = tp_ack_init(2, buffer, MAX_DATA_LEN);

    // 1.内存异常， 重启MCU
    if(unlikely(buflen < 0)) {
        fsystem_reboot(SYS_REBOOT);
    }

    // 2.创建互斥访问锁
    printer_mutex = mutex_lock_init();
    if(NULL == printer_mutex) {
        fsystem_reboot(SYS_REBOOT);
    }

    while (1) {
        mutex_lock(printer_mutex);
        tp_data[RUNNING_BUFFER].data = tp_data[WAIT_BUFFER].data;
        tp_data[RUNNING_BUFFER].len = tp_data[WAIT_BUFFER].len;
        tp_data[WAIT_BUFFER].data = NULL;
        tp_data[WAIT_BUFFER].len = 0;
        mutex_unlock(printer_mutex);

        // 3. 没有数据 继续循环
        if(likely(tp_data[RUNNING_BUFFER].data == NULL)) {
            continue;
        }

        // 4.应答MPU, 表示可以发送下一帧
        fuser_data_set(INIT_PID, buffer, buflen);

        // 5. 打印机打印数据
        fdrive_write(DRIVER_PRINT, tp_data[RUNNING_BUFFER].data, tp_data[RUNNING_BUFFER].len);
        mem_free(tp_data[RUNNING_BUFFER].data);
    }
    (void)args;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct task tp = {
    .idx   = PRINT_PID,
    .pro   = 3,
    .name  = "print",
    .set   = tp_add,
    .main  = tp_run,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
TASK_REGISTER(tp);
