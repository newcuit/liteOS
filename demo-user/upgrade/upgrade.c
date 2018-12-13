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
#include "upgrade.h"

/**************************************************************************************
* Description    : 模块内部全局数据定义
**************************************************************************************/
static struct img_hdr header;

/**************************************************************************************
 * FunctionName   : upgrade_config()
 * Description    : 重启设备
 * EntryParameter : data，指向发送的数据， len,指向发送数据长度
 * ReturnValue    : 返回发送状态或者长度
 **************************************************************************************/
static int32_t upgrade_config(uint8_t idx, void *data, int32_t len)
{
    Subid *msg = NULL;
    struct pflash pflash = {
        .addr = IMAGE_HDR_BASE,
    };

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL, len, data);
    if (unlikely(msg == NULL)) return -EFAULT;

    if (likely(msg->id == IOC__REBOOT)){
        // 2.修改rom, 指定可以升级
        pflash.data = (uint8_t *)&header;
        pflash.length = sizeof(header);
        // 3.读取已经存取的数据
        if(fdrive_ioctl(DRIVER_PFLASH, _IOC_R, &pflash, sizeof(struct pflash)) < 0){
            goto out;
        }
        header.refresh = IMAGE_UPGRADE;

        pflash.data = NULL;
        pflash.length = PFLASH_SECTOR_SIZE;
        // 4.擦书该数据段所在的块
        if(fdrive_ioctl(DRIVER_PFLASH, _IOC_E, &pflash, sizeof(struct pflash)) < 0){
            goto out;
        }

        pflash.data = (uint8_t *)&header;
        pflash.length = sizeof(header);
        // 5.将数据重新写到该块
        if(fdrive_ioctl(DRIVER_PFLASH, _IOC_W, &pflash, sizeof(struct pflash)) < 0){
            goto out;
        }
        fsystem_reboot(SYS_REBOOT);
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
    .idx = UPGRADE_PID,
    .name = "upgrade",
    .set = upgrade_config,
};

/**************************************************************************************
 * Description    : 模块初始化
 **************************************************************************************/
APP_REGISTER(upgrade);
