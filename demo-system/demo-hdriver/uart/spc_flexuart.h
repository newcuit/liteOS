/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : spc_flexuart.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __SPC_FLEXUART_DRIVER_H__
#define __SPC_FLEXUART_DRIVER_H__

#include "frtos_types.h"
#include "frtos_ioctl.h"
#include "frtos_softimer.h"
#include "frtos_queue.h"
#include "frtos_lock.h"
#include "frtos_irq.h"
#include "serial_lld.h"
#include "frtos_log.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "frtos_mem.h"

/**************************************************************************************
* Description    : 设置FLEX UART配置
**************************************************************************************/
#define UART1_BAUD                      115200                          // 波特率
#define UART1_PARITY                    SD_MODE_8BITS_PARITY_NONE       // 奇偶校验
#define UART1_API_MODE                  SPC5_LIN_API_MODE_BUFFERED_IO   // Buffer io模式
#define UART1_RXBUF_LEN                 1024                            // 接收缓冲区长度
#define UART1_RXQUEUE_LEN               10                         	    // 接收队列个数

#define UART2_BAUD                      115200                          // 波特率
#define UART2_PARITY                    SD_MODE_8BITS_PARITY_NONE       // 奇偶校验
#define UART2_API_MODE                  SPC5_LIN_API_MODE_BUFFERED_IO   // Buffer io模式
#define UART2_RXBUF_LEN                 1024                            // 接收缓冲区长度
#define UART2_RXQUEUE_LEN               10                         	    // 接收队列个数

#define UART3_BAUD                      115200                          // 波特率
#define UART3_PARITY                    SD_MODE_8BITS_PARITY_NONE       // 奇偶校验
#define UART3_API_MODE                  SPC5_LIN_API_MODE_BUFFERED_IO   // Buffer io模式
#define UART3_RXBUF_LEN                 1024                            // 接收缓冲区长度
#define UART3_RXQUEUE_LEN               10                         	    // 接收队列个数

#define UART4_BAUD                      115200                          // 波特率
#define UART4_PARITY                    SD_MODE_8BITS_PARITY_NONE       // 奇偶校验
#define UART4_API_MODE                  SPC5_LIN_API_MODE_BUFFERED_IO   // Buffer io模式
#define UART4_RXBUF_LEN                 1024                            // 接收缓冲区长度
#define UART4_RXQUEUE_LEN               10                         	    // 接收队列个数

#define UART5_BAUD                      115200                          // 波特率
#define UART5_PARITY                    SD_MODE_8BITS_PARITY_NONE       // 奇偶校验
#define UART5_API_MODE                  SPC5_LIN_API_MODE_BUFFERED_IO   // Buffer io模式
#define UART5_RXBUF_LEN                 1024                            // 接收缓冲区长度
#define UART5_RXQUEUE_LEN               10                         	    // 接收队列个数

#define UART6_BAUD                      115200                          // 波特率
#define UART6_PARITY                    SD_MODE_8BITS_PARITY_NONE       // 奇偶校验
#define UART6_API_MODE                  SPC5_LIN_API_MODE_BUFFERED_IO   // Buffer io模式
#define UART6_RXBUF_LEN                 3072                            // 接收缓冲区长度
#define UART6_RXQUEUE_LEN               10                         	    // 接收队列个数

#define UART7_BAUD                      9600                            // 波特率
#define UART7_PARITY                    SD_MODE_8BITS_PARITY_NONE       // 奇偶校验
#define UART7_API_MODE                  SPC5_LIN_API_MODE_BUFFERED_IO   // Buffer io模式
#define UART7_RXBUF_LEN                 1024                            // 接收缓冲区长度
#define UART7_RXQUEUE_LEN               10                         	    // 接收队列个数

#define UART8_BAUD                      115200                          // 波特率
#define UART8_PARITY                    SD_MODE_8BITS_PARITY_NONE       // 奇偶校验
#define UART8_API_MODE                  SPC5_LIN_API_MODE_BUFFERED_IO   // Buffer io模式
#define UART8_RXBUF_LEN                 1024                            // 接收缓冲区长度
#define UART8_RXQUEUE_LEN               10                         	    // 接收队列个数

/**************************************************************************************
* Description    : 接收数据结构定义
**************************************************************************************/
struct flexuart_rx_s{
    uint16_t len;                               // 数据长度
    uint8_t data[0];                            // 数据内容
};

/**************************************************************************************
* Description    : 消息结构定义
**************************************************************************************/
struct uart_msg_s{
    uint32_t port;                              // 串口号
    uint16_t len;                               // 数据长度
    uint8_t data[0];                            // 数据内容
};

struct uart_config_s{
    uint32_t port;
    uint32_t baudrate;
    uint32_t parity;
    uint32_t databit;
    uint32_t stopbit;
};
/**************************************************************************************
* Description    : 串口结构定义
**************************************************************************************/
struct linflex_uart {
    uint8_t registered;                 /* 串口注册情况 */
    mutex_lock_t tx_mutex;              /* TX互斥锁 */
    uint8_t *rx_buf;                    /* RX缓冲区 */
    uint16_t rx_len;                    /* RX接收长度 */
    uint16_t max_len;                   /* 最大接收长度 */
    uint16_t record_len;                /* 记录上次处理的数据长度 */
    softimer_t rx_timer;                /* RX定时器 */
    fqueue_t rx_queue;                  /* RX接收队列 */
    ioctl_cb_t rx_handler;              /* RX应用层接收回调 */
    SerialDriver *sdp;                  /* 指向记录串口的结构 */
    SerialConfig *config;               /* 指向串口配置 */
};

#define MAX_FLEX_UART_NUM             8 /* 定义最大uart个数 */

/**************************************************************************************
* Description    : 串口注册结构定义
**************************************************************************************/
#define REGISTER_UART(no, id, tx, ctl, baud, parity, uinfo, callback, timecb)        \
    static uint8_t rx_##no##_buf[UART##no##_RXBUF_LEN] = {0};     /* RX缓冲区 */      \
                                                                                     \
    /* 创建串口配置信息 */                                                              \
    static SerialConfig uart_##no##_config = {                                       \
        .speed      = baud,                                                          \
        .mode       = parity,                                                        \
        .api_mode   = UART##no##_API_MODE,                                           \
        .tx_end_cb  = NULL,                                                          \
        .rx_end_cb  = NULL,                                                          \
        .dma_enable = FALSE,                                                         \
        .dma_err_cb = NULL,                                                          \
    };                                                                               \
                                                                                     \
    static int32_t __init flexuart##no##_init(void)                                  \
    {                                                                                \
        /* 1.初始化串口信息结构 */                                                      \
        uinfo[no - 1].sdp = &(SD##no);                                               \
        uinfo[no - 1].rx_buf = rx_##no##_buf;                                        \
        uinfo[no - 1].config = &uart_##no##_config;                                  \
        uinfo[no - 1].rx_len = 0;                                                    \
        uinfo[no - 1].max_len = UART##no##_RXBUF_LEN;                                \
        uinfo[no - 1].record_len = 0;                                                \
        uinfo[no - 1].rx_handler = NULL;                                             \
                                                                                     \
        /* 2.初始化串口，并设置串口私有数据 */                                         \
        sd_lld_start(uinfo[no - 1].sdp, uinfo[no - 1].config);                       \
        sd_lld_do_receive(uinfo[no - 1].sdp, callback);                              \
        sd_lld_set_priv(uinfo[no - 1].sdp, &uinfo[no - 1]);                          \
                                                                                     \
        /* 3.创建TX_MUTEX */                                                          \
        uinfo[no - 1].tx_mutex = mutex_lock_init();                                  \
        if(NULL == uinfo[no - 1].tx_mutex) return -EMEM;                             \
                                                                                     \
        /* 4.创建RX接收队列 */                                                         \
        uinfo[no - 1].rx_queue = fqueue_create(UART##no##_RXQUEUE_LEN,               \
                                       sizeof(struct flexuart_rx_s *));              \
        if(NULL == uinfo[no - 1].rx_queue) return -EMEM;                             \
                                                                                     \
        /* 5.创建数据接收定时器 */                                                      \
        uinfo[no - 1].rx_timer = softimer_create(timecb, SOFTIMER_RELOAD_ENABLE, 10);\
        if(NULL == uinfo[no - 1].rx_timer) return -EMEM;                             \
        /* 6.设置定时器私有数据*/                                                      \
        sorftimer_set_privdata(uinfo[no - 1].rx_timer, &uinfo[no - 1]);              \
                                                                                     \
        /* 7.启动定时器 */                                                             \
        softimer_start(uinfo[no - 1].rx_timer, 0);                                   \
                                                                                     \
        /* 8.设置为注册 */                                                             \
        uinfo[no - 1].registered = 1;                                                \
                                                                                     \
        return 0;                                                                    \
    }                                                                                \
                                                                                     \
    /* 初始化驱动结构信息 */                                                            \
    static __const struct driver spc_flexuart##no##_ = {                             \
        .idx   = id,                                                                 \
        .name  = "ttyS"#no,                                                          \
        .init  = flexuart##no##_init,                                                \
        .write = tx,                                                                 \
        .ioctl = ctl,                                                                \
    };                                                                               \
    /* 驱动结构注册 */                                                                 \
    CORE_INIT(spc_flexuart##no##_)

#endif /* __SPC_FLEXUART_DRIVER_H__ */

