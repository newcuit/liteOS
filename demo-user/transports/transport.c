/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : transport.c
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
#include "frtos_libpack.h"
#include "frtos_list.h"
#include "frtos_sys.h"
#include "frtos_log.h"
#include "transport.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define TRANS_UART                       DRIVER_UART5
#define BUFFER_FIFO_SIZE                 3096              // 缓存暂时没有收全的protobuf数据包
static uint8_t debug = 0;                                  // debug开关

/**************************************************************************************
* FunctionName   : chksum_xor()
* Description    : 计算校验
* EntryParameter : data，指向待校验数据， len,送数据长度
* ReturnValue    : 返回校验码
**************************************************************************************/
static uint8_t chksum_xor(uint8_t *data, int32_t len)
{
    int32_t i = 1;
    uint8_t csum = data[0];

    for (i = 1; i < len; i++) {
        csum ^= data[i];
    }
    return csum;
}

/**************************************************************************************
* FunctionName   : trans_data_dump()
* Description    : 打印传输数据
* EntryParameter : data，指向打印数据， len,送数据长度
* ReturnValue    : 返回0
**************************************************************************************/
static uint8_t trans_data_dump(char *name, uint8_t *data, int32_t len)
{
    int32_t i = 0;

    stdio_printf("%s(%d):", name, len);
    for (i = 0; i < len; i++) {
        stdio_printf("%02X ", data[i]);
        if(i%16 == 15) stdio_printf(STRBR"     ");
    }
    stdio_printf(STRBR STRBR);
    return 0;
}

/**************************************************************************************
* FunctionName   : trans_recv()
* Description    : MCU接收到MPU的数据
* EntryParameter : data，指向MPU发送的数据， len,指向MPU发送数据长度
* ReturnValue    : 返回None
**************************************************************************************/
static int32_t trans_recv(int32_t idx, void *data, int32_t len)
{
    int32_t pos = 0;
    uint8_t id = 0;
    uint32_t magic = 0, length = 0;
    struct transport *tdata = NULL;
    static uint16_t data_left = 0;
    static uint8_t data_space[BUFFER_FIFO_SIZE];

    if(unlikely(data == NULL)) return -EINVAL;

    if(unlikely(debug)) trans_data_dump("recv", data, len);

    // 0.如果上次有数据未处理完， 这里添加到头部重新处理
    if(data_left + len > BUFFER_FIFO_SIZE) data_left = 0;

    memcpy(data_space + data_left, data, len);
    data_left = data_left + len;

    // 1.遍历整个数据区，寻找数据头部
    while (likely((data_left - pos) > (int32_t)sizeof(struct transport))) {
        tdata = (struct transport *)((uint8_t *)data_space + pos);

        // 头部数据，解析数据长度和ID
        unpack_be8(tdata->id, &id);
        unpack_be32(tdata->magic, &magic);
        unpack_be32(tdata->length, &length);

        // 1.1 判断是否为头部
        if(likely(magic != TRANS_MAGIC)) {
            pos++;continue;
        }

        // 检测数据长度有效性, 长度异常, 跳出循环
        if(unlikely(length > (data_left - pos - sizeof(struct transport)))) {
            break;
        }
        // 数据校验可靠性检测， 错误就重新尝试
        if(tdata->csum == chksum_xor((uint8_t *)tdata->data, length)) {
            fuser_data_set(id, tdata->data, length);
            pos = pos + length;         //去掉数据部分长度
        } else {
            stdio_printf("application[%d] recv data(%d) chksum fail !!!"STRBR, id, length);
        }

        // 数据去掉头部数据
        pos += sizeof(struct transport);
    }

    data_left = data_left - pos;
    memcpy(data_space,  data_space + pos, data_left);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : trans_send()
* Description    : MCU发送数据到MPU
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t trans_send(uint8_t idx, void *data, int32_t len)
{
    int32_t tdata_len = 0;
    struct transport *tdata = NULL;
    struct piddata *pdata = (struct piddata *)data;

    if(pdata->len + sizeof(struct piddata) != (uint32_t)len || data == NULL) {
        return -EINVAL;
    }
    tdata_len = sizeof(struct transport) + pdata->len;

    tdata = mem_malloc(tdata_len);
    if(unlikely(tdata == NULL)) return -ENOMEM;

    pack_be8(pdata->id, &tdata->id);
    pack_be32(pdata->len, &tdata->length);
    pack_be32(TRANS_MAGIC, &tdata->magic);

    memcpy(tdata->data, pdata->data, pdata->len);
    tdata->csum = chksum_xor((uint8_t *)tdata->data, pdata->len);

    if(unlikely(debug)) trans_data_dump("send", (uint8_t *)tdata, tdata_len);
    if (fdrive_write(TRANS_UART, tdata, tdata_len) <= 0) {
        mem_free(tdata);
        return -EREMOTEIO;
    }
    mem_free(tdata);

    (void)idx;
    return len;
}

/**************************************************************************************
* TypeName       : trans_cmd()
* Description    : trans控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t trans_cmd(char *argv, int32_t argc)
{
    //1.设置串口传输的debug收发开关
    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("init debug on"STRBR);
        stdio_printf("init debug off"STRBR);
    } else if (argc == 8 && 0 == strcmp(argv, "debug on")) {
        debug = 1;
    } else if (argc == 9 && 0 == strcmp(argv, "debug off")) {
        debug = 0;
    }
    return argc;
}

/**************************************************************************************
* FunctionName   : trans_init()
* Description    : MCU通信接口初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t trans_init(void)
{
    return fdrive_ioctl(TRANS_UART, _IOC_SET_CB, trans_recv, sizeof(trans_recv));
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite app_transport = {
    .idx   = INIT_PID,
    .name  = "init",
    .init  = trans_init,
    .set   = trans_send,
    .debug = trans_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(app_transport);
