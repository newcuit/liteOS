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
#include "spc_flexuart.h"

/**************************************************************************************
* Description    : 定义ADC数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                 256
#define MAX_SUBPROTO_LEN             128
static uint8_t debug = 0;                           //uart调试开关

/**************************************************************************************
* FunctionName   : uart_recv()
* Description    : uart数据串
* EntryParameter : data，uart数据， len,gps数据长度
* ReturnValue    : 返回处理的数据长度或者错误码
**************************************************************************************/
static int32_t uart_recv(int32_t idx, void *data, int32_t len)
{
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t buffer[MAX_DATA_LEN];
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    struct piddata *pidata = (struct piddata *)buffer;

    if(unlikely(data == NULL)) return -EINVAL;

    SerialData serial = SERIAL__DATA__INIT;

    // 1.读取需要上报的信息
    if(idx == DRIVER_UART2) serial.port = 2;
    else if(idx == DRIVER_UART4) serial.port = 3;
    else if(idx == DRIVER_UART7) serial.port = 4;
    else serial.port = 485;

    serial.data.data = data;
    serial.data.len = len;

    if(unlikely(debug)) {
        uint16_t i;
        stdio_printf("UART%d recv :"STRBR, serial.port);
        for(i = 0; i < serial.data.len; i++)
        {
            stdio_printf("%02x", serial.data.data[i]);
        }
        stdio_printf(STRBR);
    }

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = serial__data__get_packed_size(&serial);
    proto.data = bytedata;
    serial__data__pack(&serial, bytedata);

    // 3.准备发送protobuf打包
    pidata->id = UART_PID;
    pidata->len = subid__get_packed_size(&msg);
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : uart_set()
* Description    : 配置uart
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t uart_set(uint8_t idx, void *data, int32_t len)
{
    Subid *msg = NULL;
    SerialSet *serial = NULL;
    uint8_t i = 0, uartid = 0;
    struct uart_config_s msgbuf;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < msg->n_subdata; i++) {
        serial = serial__set__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(serial == NULL))  break;

        if(serial->port == 2) uartid = DRIVER_UART2;
        else if(serial->port == 3) uartid = DRIVER_UART4;
        else if(serial->port == 4) uartid = DRIVER_UART7;
        else uartid = DRIVER_UART3;

        msgbuf.port = serial->port;
        msgbuf.baudrate = serial->baudrate;
        msgbuf.parity = serial->parity;
        msgbuf.stopbit = serial->stopbit;
        msgbuf.databit = serial->databit;

        if(unlikely(debug)) {
            stdio_printf("UART%d set: baudrate %d, parity %d, databit %d, stopbit %d"STRBR,\
                serial->port, serial->baudrate, serial->parity, serial->databit, serial->stopbit);
        }
        fdrive_ioctl(uartid, _IOC_SET, &msgbuf, sizeof(struct uart_config_s));
        fdrive_ioctl(uartid, _IOC_SET_CB, uart_recv, sizeof(uart_recv));

        serial__set__free_unpacked(serial, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : uart_send()
* Description    : uart发送数据
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t uart_send(uint8_t idx, void *data, int32_t len)
{
    Subid *msg = NULL;
    SerialData *serial = NULL;
    uint8_t i = 0, uartid = 0;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < msg->n_subdata; i++) {
        serial = serial__data__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(serial == NULL)) break;

        if(serial->port == 2) uartid = DRIVER_UART2;
        else if(serial->port == 3) uartid = DRIVER_UART4;
        else if(serial->port == 4) uartid = DRIVER_UART7;
        else uartid = DRIVER_UART3;

        if(unlikely(debug)) {
            uint16_t i;
            stdio_printf("UART%x send :"STRBR, serial->port);
            for(i = 0; i < serial->data.len; i++)
            {
                stdio_printf("%02x",serial->data.data[i]);
            }
            stdio_printf(STRBR);
        }

        fdrive_write(uartid, serial->data.data, serial->data.len);

        serial__data__free_unpacked(serial, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}


/**************************************************************************************
* FunctionName   : uart_ctrl()
* Description    : uart分发数据
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t uart_ctrl(uint8_t idx, void *data, int32_t len)
{
    Subid *msg = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    if(msg->id == IOC__SET) uart_set(idx, data, len);
    else if(msg->id == IOC__SEND) uart_send(idx, data, len);

    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    return len;
}

/**************************************************************************************
* TypeName       : can_cmd()
* Description    : can控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t uart_cmd(char *argv, int32_t argc)
{
    //1.设置UART的debug收发开关
    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("uart debug on"STRBR);
        stdio_printf("uart debug off"STRBR);
    } else if (argc == 8 && 0 == strcmp(argv, "debug on")) {
        debug = 1;
    } else if (argc == 9 && 0 == strcmp(argv, "debug off")) {
        debug = 0;
    }
    return argc;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite uart = {
    .idx    = UART_PID,
    .name   = "uart",
    .set    = uart_ctrl,
    .debug  = uart_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(uart);
