/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : upgrade.c
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
#include "version.h"

/**************************************************************************************
* FunctionName   : swapint32()
* Description    : 长整形大小端互换
* EntryParameter : 待处理数据
* ReturnValue    : 大小端互换后的数据
**************************************************************************************/
static uint32_t swapint32(uint32_t value)
{
	return(((value&0x000000FF)<<24)|((value&0x0000FF00)<<8)|((value&0x00FF0000)>>8)|((value&0xFF000000)>>24));
}
/**************************************************************************************
* FunctionName   : version_send()
* Description    : 版本信息发送给MPU
* EntryParameter : data，数据， len，数据长度
* ReturnValue    : 返回处理的数据长度或者错误码
**************************************************************************************/
static void version_send(struct img_hdr *header, uint32_t len)
{
    uint32_t length = 0;
    Subid msg = SUBID__INIT;
    Version version = VERSION__INIT;
    ProtobufCBinaryData proto;
    struct piddata *pidata = NULL;

    // 1.读取需要上报的信息
    version.btime = header->btime;
    version.os_name.data = (uint8_t *)header->os_name;
    version.os_name.len = sizeof(header->os_name);

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = version__get_packed_size(&version);
    proto.data = (uint8_t *)mem_malloc(proto.len);
    version__pack(&version, proto.data);

    // 3.准备发送protobuf打包
    length = subid__get_packed_size(&msg);
    pidata = (struct piddata *)mem_malloc(length+sizeof(struct piddata));

    pidata->id = VERSION_PID;
    pidata->len = length;
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

    mem_free(pidata);
    mem_free(proto.data);
    (void)len;
}

/**************************************************************************************
 * FunctionName   : version_get()
 * Description    : 获取设备信息
 * EntryParameter : data，指向发送的数据， len,指向发送数据长度
 * ReturnValue    : 返回发送状态或者长度
 **************************************************************************************/
static int32_t version_get(uint8_t idx, void *data, int32_t len)
{
    Subid *msg = NULL;
    struct img_hdr header;
    struct pflash pflash = {
        .addr = IMAGE_HDR_BASE,
	    .data = (uint8_t *)&header,
	    .length = sizeof(header),
    };

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL, len, data);
    if (unlikely(msg == NULL)) return -EFAULT;

    if (likely(msg->id == IOC__GET)){
        // 2.修改rom, 指定可以升级
        if(fdrive_ioctl(DRIVER_PFLASH, _IOC_R, &pflash, sizeof(struct pflash)) < 0){
            goto out;
        }
        /*调整字节顺序*/
        header.btime = swapint32(header.btime);
        version_send(&header, sizeof(header));
    }
out:
    // 3.释放内存
    subid__free_unpacked(msg, NULL);

    (void) idx;
    return len;
}

/**************************************************************************************
 * Description    : 定义通信任务结构
 **************************************************************************************/
static __const struct applite upgrade = {
    .idx = VERSION_PID,
    .name = "version",
    .set = version_get,
};

/**************************************************************************************
 * Description    : 模块初始化
 **************************************************************************************/
APP_REGISTER(upgrade);
