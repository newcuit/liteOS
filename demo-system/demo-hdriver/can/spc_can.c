/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : spc_can.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "spc_can.h"

/*************************************************************************************
* Description    : CAN信息数据定义
*************************************************************************************/
struct flex_can flexcan[SPC_MAX_CAN_COUNT];

/*************************************************************************************
* FunctionName   : spccan_phy_rxtimer()
* Description    : 物理接收定时器
* EntryParameter : timer,定时器
* ReturnValue    : None
*************************************************************************************/
static void spccan_phy_rxtimer(softimer_t timer)
{
    struct spccan_recv_s *recvbuf = NULL;
    struct flex_can *flx_can = sorftimer_get_privdata(timer);

    /* 1.检查是否有数据 */
    if(can_lld_receive(flx_can->can, 0, &flx_can->msg) != CAN_MSG_OK) {
        return;
    }

    /* 2.申请消息空间 */
    recvbuf = mem_malloc(sizeof(struct spccan_recv_s));
    if(NULL == recvbuf) return;

    /* 3.填充消息数据 */
    recvbuf->idx = flx_can - flexcan;
    if(flx_can->msg.IDE == 1U) {
        recvbuf->msg.msgid = flx_can->msg.EID;
    } else {
        recvbuf->msg.msgid = flx_can->msg.SID;
    }
    recvbuf->msg.len = flx_can->msg.LENGTH > 8? 8:flx_can->msg.LENGTH;

    memcpy(recvbuf->msg.data, flx_can->msg.data8, recvbuf->msg.len);

    /* 4.添加到队列 */
    if(0 != fqueue_push(flx_can->rxqueue, &recvbuf, 0)){
        mem_free(recvbuf);
    }

    (void)timer;
}

/*************************************************************************************
* FunctionName   : spccan_phy_send()
* Description    : 设备物理发送
* EntryParameter : msgid,消息ID, *data,发送数据指针, len,发送数据长度(<=8)
* ReturnValue    : 返回发送的数据长度
*************************************************************************************/
static int16_t spccan_phy_send(struct flex_can *flx_can, uint32_t msgid,
        uint8_t *data, int16_t len)
{
    int16_t err = len;
    CANTxFrame txmsg;

    /* 1.参数检查 */
    if(unlikely(NULL == data || len > 8 || len <= 0)) return -EINVAL;

    /* 2.消息体填充 */
    txmsg.EID = msgid;
    txmsg.IDE = CAN_IDE_EXT;
    txmsg.RTR = CAN_RTR_DATA;
    txmsg.LENGTH = (uint8_t)len;
    memcpy(txmsg.data8, data, len);

    /* 3.上锁 */
    mutex_lock(flx_can->mutex);

    /* 4.发送CAN数据 */
    if(can_lld_transmit(flx_can->can, 0, &txmsg) != CAN_MSG_OK) {
        err = -EREMOTEIO;
    }

    /* 5.发送延时 */
    frtos_delay_ms(SPCCAN_SEND_OVERTIME);

    /* 7.解锁 */
    mutex_unlock(flx_can->mutex);

    return err;
}

/*************************************************************************************
* FunctionName   : spccan_phy_setrxfilter()
* Description    : 设备物理屏蔽字,接受扩展帧or标准帧
* EntryParameter : mask,屏蔽字
* ReturnValue    : 返回错误码
*************************************************************************************/
static int8_t spccan_phy_setrxfilter(struct flex_can *flx_can,
        struct can_filter_s *filter_msg)
{
    /* 1.上锁 */
    mutex_lock(flx_can->mutex);

    /* 2.终止接收 */
    (void)filter_msg;
    /* 4.开始接收数据 */
    can_lld_receive(flx_can->can, 0, &flx_can->msg);

    /* 5.解锁 */
    mutex_unlock(flx_can->mutex);

    return 0;
}

/*************************************************************************************
* FunctionName   : spccan_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t spccan_write(uint8_t idx, void *data, int32_t len)
{
    uint16_t i = 0;
    struct can_msg_s *msgbuf = (struct can_msg_s *)data;

    if(unlikely(NULL == data || len <= 0 ||
        0 != (len % sizeof(struct can_msg_s)))){
        return -EINVAL;
    }

    /* 1.设备未注册 */
    if(unlikely(!flexcan[idx - DRIVER_CAN0].registered)) return -ENODEV;

    /* 2.循环发送数据 */
    for(i = 0; i < (len / sizeof(struct can_msg_s)); i++){
        if(msgbuf[i].len != spccan_phy_send(&flexcan[idx - DRIVER_CAN0],
            msgbuf[i].msgid, msgbuf[i].data, msgbuf[i].len)){
            return -EREMOTEIO;
        }
    }

    (void)idx;
    return len;
}

/*************************************************************************************
* FunctionName   : spccan_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t spccan_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len != 0) ||
        (NULL != args && 0 == len) || len < 0)){
        return -EINVAL;
    }

    /* 1.设备未注册 */
    if(unlikely(!flexcan[idx - DRIVER_CAN0].registered)) return -ENODEV;

    /* 2.执行命令 */
    switch(cmd){
    case _IOC_SET_CB:
        if(unlikely(NULL == args)) return -EINVAL;
        flexcan[idx - DRIVER_CAN0].rx_handler = (ioctl_cb_t)args;
        break;
    case _IOC_SET_FILTER:
        if(unlikely(NULL == args)) return -EINVAL;
        spccan_phy_setrxfilter(&flexcan[idx - DRIVER_CAN0], (struct can_filter_s *)args);
        break;
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : spccan_rx()
* Description    : 设备驱动任务
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t spccan_rx(void)
{
    uint8_t idx = 0;
    struct spccan_recv_s *recvbuf = NULL;

    /* 1.遍历所有串口 */
    for (idx = 0; idx < SPC_MAX_CAN_COUNT; idx++) {
        /* 2.CAN未注册，跳过 */
        if(unlikely(!flexcan[idx].registered)) continue;

        /* 3.处理接收数据 */
        if(0 == fqueue_pop(flexcan[idx].rxqueue, &recvbuf, 0, true)) {
            if(NULL != flexcan[idx].rx_handler){
                flexcan[idx].rx_handler(idx+DRIVER_CAN0, &recvbuf->msg, sizeof(struct can_msg_s));
            }
            mem_free(recvbuf);
        }
    }

    return 0;
}

/*************************************************************************************
* FunctionName   : spccan_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
static int32_t __init spccan_init(void)
{
    /* 1.初始化物理接口 */
    can_lld_init();

    return 0;
}

/**************************************************************************************
* Description    : 注册CAN驱动
**************************************************************************************/
REGISTER_CAN(1, DRIVER_CAN0, spccan_write, spccan_ioctrl, flexcan, spccan_phy_rxtimer);
REGISTER_CAN(2, DRIVER_CAN1, spccan_write, spccan_ioctrl, flexcan, spccan_phy_rxtimer);
REGISTER_CAN(3, DRIVER_CAN2, spccan_write, spccan_ioctrl, flexcan, spccan_phy_rxtimer);
REGISTER_CAN(6, DRIVER_CAN5, spccan_write, spccan_ioctrl, flexcan, spccan_phy_rxtimer);

/**************************************************************************************
* Description    : 定义CAN初始化模块
**************************************************************************************/
static __const struct driver spc_can = {
    .init  = spccan_init,
    .run   = spccan_rx,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(spc_can);
