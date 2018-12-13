/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : spc_can.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __SPC_CAN_H__
#define __SPC_CAN_H__

#include "can_lld.h"
#include "can_lld_cfg.h"

#include "frtos_drivers.h"
#include "frtos_errno.h"
#include "config_driver.h"
#include "frtos_softimer.h"
#include "frtos_mem.h"
#include "frtos_queue.h"
#include "frtos_lock.h"
#include "frtos_delay.h"
#include "frtos_log.h"
#include "gpio_driver.h"
#include "frtos_ioctl.h"

/**************************************************************************************
* Description    : 模块内部配置宏定义
**************************************************************************************/
#define SPCCAN_SEND_OVERTIME                           1   // CAN发送超时配置
#define SPC_MAX_CAN_COUNT                              6   //SPC最大支持CAN个数

/**************************************************************************************
* Description    : 设备端口配置
**************************************************************************************/
#define SPCCAN1_IO_STB                         PORT_E,PTE7    // STB模式使能
#define SPCCAN2_IO_STB                         PORT_E,PTE6    // STB模式使能
#define SPCCAN3_IO_STB                         PORT_C,PTC12   // STB模式使能
#define SPCCAN4_IO_STB                         PORT_C,PTC13   // STB模式 未使用
#define SPCCAN5_IO_STB                         PORT_C,PTC13   // STB模式 未使用
#define SPCCAN6_IO_STB                         PORT_C,PTC13   // STB模式使能

/**************************************************************************************
* Description    : CAN配置
**************************************************************************************/
#define CAN1_QUEUE_LEN                                 20    // can0接收队列长度
#define CAN2_QUEUE_LEN                                 20    // can1接收队列长度
#define CAN3_QUEUE_LEN                                 20    // can2接收队列长度
#define CAN4_QUEUE_LEN                                 20    // can3接收队列长度
#define CAN5_QUEUE_LEN                                 20    // can4接收队列长度
#define CAN6_QUEUE_LEN                                 20    // can5接收队列长度

/**************************************************************************************
* Description    : 消息结构定义
**************************************************************************************/
struct can_msg_s {
    uint32_t msgid;                                     // CAN消息ID
    uint8_t len;                                        // CAN消息长度
    uint8_t data[8];                                    // CAN消息数据
};

/**************************************************************************************
* Description    : 过滤表结构定义
**************************************************************************************/
struct can_filter_s{
	uint32_t mbid;                                      // 邮箱ID
	CANFilter RxFilter;                                 // 要屏蔽的ID类型和ID
};

/**************************************************************************************
* Description    : CAN驱动结构定义
**************************************************************************************/
struct flex_can {
    uint8_t registered;                                /* CAN注册情况 */
    CANRxFrame msg;                                    /* CAN接收缓冲 */
    CANDriver  *can;                                   /* 指向CAN寄存器结构 */
    mutex_lock_t mutex;                                /* CAN自旋锁 */
    softimer_t rxtimer;                                /* CAN定时器时钟 */
    fqueue_t rxqueue;                                  /* CAN数据接收队列 */
    ioctl_cb_t rx_handler;                             /* CAN数据回到函数 */
};

/**************************************************************************************
* Description    : 消息结构定义
**************************************************************************************/
struct spccan_recv_s{
    uint8_t idx;                                        // 设备索引
    struct can_msg_s msg;                               // 消息
};

/**************************************************************************************
* Description    : 串口注册结构定义
**************************************************************************************/
#define REGISTER_CAN(no, id, tx, ctl, flcan, timecb)                                 \
    static int32_t __init can##no##_init(void)                                       \
    {                                                                                \
        /* 1.初始化串口信息结构 */                                                      \
        flcan[no - 1].can = &CAND##no;                                               \
                                                                                     \
        /* 2.创建TX_MUTEX */                                                          \
        flcan[no - 1].mutex = mutex_lock_init();                                     \
        if(NULL == flcan[no - 1].mutex) return -EMEM;                                \
                                                                                     \
        /* 3.创建RX接收队列 */                                                         \
        flcan[no - 1].rxqueue = fqueue_create(CAN##no##_QUEUE_LEN,                   \
                sizeof(struct spccan_recv_s *));                                     \
        if(NULL == flcan[no - 1].rxqueue) return -EMEM;                              \
                                                                                     \
        /* 4.创建数据接收定时器 */                                                      \
        flcan[no - 1].rxtimer = softimer_create(timecb, SOFTIMER_RELOAD_ENABLE, 1);  \
        if(NULL == flcan[no - 1].rxtimer) return -EMEM;                              \
                                                                                     \
        /* 5.设置定时器私有数据, 并开启CAN*/                                            \
        sorftimer_set_privdata(flcan[no - 1].rxtimer, &flcan[no - 1]);               \
        gpio_pin_set(SPCCAN##no##_IO_STB, 0);                                        \
        can_lld_start(flcan[no - 1].can , &can_config_can##no##_config);             \
                                                                                     \
        /*6.启动接收定时器 */                                                          \
        softimer_start(flcan[no - 1].rxtimer, 0);                                    \
        flcan[no - 1].registered = 1;                                                \
                                                                                     \
        return 0;                                                                    \
    }                                                                                \
                                                                                     \
    /* 初始化驱动结构信息 */                                                            \
    static __const struct driver spc_can##no = {                                     \
        .idx   = id,                                                                 \
        .name = "can"#no,                                                            \
        .init  = can##no##_init,                                                     \
        .write = tx,                                                                 \
        .ioctl = ctl,                                                                \
    };                                                                               \
    /* 驱动结构注册 */                                                                 \
    CORE_INIT(spc_can##no)


#endif /* __SPC_CAN_H__ */

