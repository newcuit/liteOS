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
#include "data.pb-c.h"
#include "frtos_sys.h"
#include "dflash_driver.h"
#include "frtos_ioctl.h"

/**************************************************************************************
* FunctionName   : dflash_send()
* Description    : dflash数据串
* EntryParameter : data，dflash数据， len，数据长度
* ReturnValue    : 返回处理的数据长度或者错误码
**************************************************************************************/
static void dflash_send(uint32_t offset, uint32_t size, uint8_t *data)
{
    uint32_t length = 0;
    Subid msg = SUBID__INIT;
    Flash dflash = FLASH__INIT;
    ProtobufCBinaryData proto;
    struct piddata *pidata = NULL;

    // 1.读取需要上报的信息
    if(likely(data == NULL)) dflash.has_data = 0;
    else {
        dflash.has_data = 1;
        dflash.data.len = size;
        dflash.data.data = data;
    }
    dflash.offset = offset;
    dflash.size = size;

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = flash__get_packed_size(&dflash);
    proto.data = (uint8_t *)mem_malloc(proto.len);
    flash__pack(&dflash, proto.data);

    // 3.准备发送protobuf打包
    length = subid__get_packed_size(&msg);
    pidata = (struct piddata *)mem_malloc(length+sizeof(struct piddata));

    pidata->id = DFLASH_PID;
    pidata->len = length;
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

    mem_free(pidata);
    mem_free(proto.data);
}

/**************************************************************************************
* FunctionName   : dflsah_ctrl()
* Description    : dflash配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t dflsah_ctrl(uint8_t idx, void *data, int32_t len)
{
    Subid *msg = NULL;
    Flash *flash = NULL;
    struct dflash dflash;
    int32_t i = 0, optlen = 0;
    uint8_t *buffer = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < (int32_t)msg->n_subdata; i++) {
        flash = flash__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(flash == NULL))  break;

        dflash.addr = flash->offset;
        dflash.length = flash->size;

        // 2.处理protobuf协议子ID
        if(unlikely(msg->id == IOC__SET)) {
            dflash.data = flash->data.data;
            optlen = fdrive_ioctl(DRIVER_DFLASH, _IOC_W, &dflash, sizeof(struct dflash));
        } else if(likely(msg->id == IOC__GET)) {
        	buffer = (uint8_t *)mem_malloc(flash->size);
        	dflash.data = buffer;
            optlen = fdrive_ioctl(DRIVER_DFLASH, _IOC_R, &dflash, sizeof(struct dflash));
        }

        // 3. 判断操作DFLASH的处理结果， optlen大于0表示成功， 小于或者等于0表示失败(失败发送data为NULL， 成功长度为0)
        if(likely(optlen > 0)) {
            dflash_send(flash->offset, flash->size, buffer);
        } else {
            dflash_send(flash->offset, 0, NULL);
        }

        flash__free_unpacked(flash, NULL);

        // 4. 如果使用了buffer, 就释放它（只有读dflash才使用）
        if(buffer) mem_free(buffer);
    }
    // 5.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite dflash = {
    .idx   = DFLASH_PID,
    .name  = "dflash",
    .set   = dflsah_ctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(dflash);
