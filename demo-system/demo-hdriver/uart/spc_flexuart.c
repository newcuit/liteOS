/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : spc_flexuart.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "spc_flexuart.h"
#include "frtos_time.h"

/**************************************************************************************
* Description    : 定义串口信息结构
**************************************************************************************/
static struct linflex_uart flexuart[MAX_FLEX_UART_NUM];
static inline int16_t spc_phy_tx(SerialDriver *sdp, const void *data, int16_t len);

/**************************************************************************************
* FunctionName   : stdio_output()
* Description    : 标准输出函数,需要外部重写
* EntryParameter : data,数据指针, len,数据长度
* ReturnValue    : None
**************************************************************************************/
void stdio_output(uint8_t *dat, int16_t len)
{
    struct linflex_uart *uart = sd_lld_get_priv(&SD1);

    if(uart!= NULL && uart->registered) spc_phy_tx(uart->sdp, dat, len);
}

/**************************************************************************************
* FunctionName   : stdio_echo()
* Description    : 输入回显函数
* EntryParameter : sdp,指向当前数据串口ID, data,数据指针, len,数据长度
* ReturnValue    : None
**************************************************************************************/
static void stdio_echo(SerialDriver *sdp, uint8_t *dat, int16_t len)
{
    if(unlikely(sdp == &SD1)) stdio_output(dat, len);
}

/*************************************************************************************
* FunctionName   : spc_phy_tx()
* Description    : UART物理发送
* EntryParameter : *data,数据指针, len,数据长度
* ReturnValue    : 返回发送的字节数, 返回错误码
*************************************************************************************/
static inline int16_t spc_phy_tx(SerialDriver *sdp, const void *data, int16_t len)
{
    uint32_t ret = SERIAL_MSG_WAIT;
    struct linflex_uart *uart = sd_lld_get_priv(sdp);

    /* 0.判断结构合法 */
    if(unlikely(uart == NULL)) return ret;

    /* 1.上锁 */
    mutex_lock(uart->tx_mutex);

    /* 2.发送数据 */
    do {
        ret = sd_lld_write(sdp, (uint8_t *)data, len);
    } while(unlikely(ret == SERIAL_MSG_WAIT));

    /* 3.解锁 */
    mutex_unlock(uart->tx_mutex);

    return ret;
}

/*************************************************************************************
* FunctionName   : spc_rx_callback()
* Description    : UART物理接收定
* EntryParameter : drv_state,状态, event,事件标记, *args,参数指针
* ReturnValue    : None
*************************************************************************************/
static void spc_rx_callback(SerialDriver *sdp, uint8_t data)
{
    struct linflex_uart *uart = sd_lld_get_priv(sdp);

    /* 0.判断结构合法 */
    if(unlikely(uart == NULL)) return;

    /* 1.移动偏移量，设置下一个接收点 */
    uart->rx_buf[uart->rx_len++] = data;

    /* 2.检查接收缓冲区满 */
    if(unlikely(uart->max_len == uart->rx_len)) uart->rx_len = 0;

    (void)sdp;
}

/*************************************************************************************
* FunctionName   : spc_timer_callback()
* Description    : UART物理监视定时器
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void spc_timer_callback(softimer_t timer)
{
    struct flexuart_rx_s *qdata = NULL;
    struct linflex_uart *uart = sorftimer_get_privdata(timer);

    /* 0.判断结构合法 */
    if(unlikely(uart == NULL)) return;

    /* 1. 禁用中断 */
    sd_lld_rx_disable(uart->sdp);

    /* 2.检查数据接收是否还在进行 */
    if(likely(uart->record_len != uart->rx_len)){
        uart->record_len = uart->rx_len;
        sd_lld_rx_enable(uart->sdp);
        return;
    }

    /* 3.检查是否接收到数据 */
    if(unlikely(0 == uart->record_len)) goto NEXT_RECV;

    /* 4.申请数据存储空间 */
    qdata = mem_malloc(sizeof(struct flexuart_rx_s) + uart->record_len);
    if(unlikely(NULL == qdata)) goto NEXT_RECV;

    /* 5.填充数据 */
    qdata->len = uart->record_len;
    memcpy(qdata->data, uart->rx_buf, uart->record_len);

    /* 6.将数据添加到队列 */
    if(0 == fqueue_push(uart->rx_queue, &qdata, 0)) goto NEXT_RECV;

    /* 7.释放入队列失败的数据 */
    mem_free(qdata);

    NEXT_RECV:

    /* 7.接收计数器清零 */
    uart->rx_len = 0;
    uart->record_len = 0;

    /* 8.使能中断 */
    sd_lld_rx_enable(uart->sdp);

    (void)timer;
}

/*************************************************************************************
* FunctionName   : spcflexuart_write()
* Description    : 写
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/
static int32_t spcflexuart_write(uint8_t idx, void *data, int32_t len)
{
    /* 1.设备未注册 */
    if(unlikely(!flexuart[idx - DRIVER_UART0].registered)) return -ENODEV;

    /* 2.写入数据 */
    return spc_phy_tx(flexuart[idx - DRIVER_UART0].sdp, data, len);
}

/*************************************************************************************
* FunctionName   : spcflexuart_config()
* Description    : 串口参数设置，这个函数必须在中断禁用的情况下调用
* EntryParameter : *sdp,串口配置参数
* ReturnValue    : 返回写入的字节数, 返回错误码
*************************************************************************************/

static void spcflexuart_config(uint8_t idx, void *data, int32_t len)
{
    struct uart_config_s *config = (struct uart_config_s *)data;

    static SerialConfig uart = {
        .api_mode = SPC5_LIN_API_MODE_BUFFERED_IO,
        .dma_enable = FALSE,
        .dma_err_cb = NULL,
        .rx_end_cb = NULL,
        .tx_end_cb = NULL,
    };
    uart.speed = config->baudrate;
    if(config->parity == 0) uart.mode = SD_MODE_8BITS_PARITY_NONE;
    else if(config->parity == 1) uart.mode = SD_MODE_8BITS_PARITY_ODD;
    else if(config->parity == 2) uart.mode = SD_MODE_8BITS_PARITY_EVEN;

    if(config->port == 2)
    {
        // 失能串口中断
        irq_disable(SPC5_LINFLEX2_RXI_NUMBER);
        irq_disable(SPC5_LINFLEX2_TXI_NUMBER);
        sd_lld_stop(&SD3);
        sd_lld_start(&SD3, &uart);
        // 使能串口中断
        irq_enable(SPC5_LINFLEX2_RXI_NUMBER);
        irq_enable(SPC5_LINFLEX2_TXI_NUMBER);
    }else if(config->port == 3) {
        // 失能串口中断
        irq_disable(SPC5_LINFLEX4_RXI_NUMBER);
        irq_disable(SPC5_LINFLEX4_TXI_NUMBER);
        sd_lld_stop(&SD5);
        sd_lld_start(&SD5, &uart);
        // 使能串口中断
        irq_enable(SPC5_LINFLEX4_RXI_NUMBER);
        irq_enable(SPC5_LINFLEX4_TXI_NUMBER);
    }else if(config->port == 4) {
        // 失能串口中断
        irq_disable(SPC5_LINFLEX7_RXI_NUMBER);
        irq_disable(SPC5_LINFLEX7_TXI_NUMBER);
        sd_lld_stop(&SD8);
        sd_lld_start(&SD8, &uart);
        // 使能串口中断
        irq_enable(SPC5_LINFLEX7_RXI_NUMBER);
        irq_enable(SPC5_LINFLEX7_TXI_NUMBER);
    }else {
        // 失能串口中断
        irq_disable(SPC5_LINFLEX3_RXI_NUMBER);
        irq_disable(SPC5_LINFLEX3_TXI_NUMBER);
        sd_lld_stop(&SD4);
        sd_lld_start(&SD4, &uart);
        // 使能串口中断
        irq_enable(SPC5_LINFLEX3_RXI_NUMBER);
        irq_enable(SPC5_LINFLEX3_TXI_NUMBER);
    }

    (void)idx;
    (void)len;
}
/*************************************************************************************
* FunctionName   : spcflexuart_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t spcflexuart_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    if(unlikely((NULL == args && len != 0) ||
        (NULL != args && 0 == len) || len < 0)){
        return -EINVAL;
    }

    /* 1.设备未注册 */
    if(unlikely(!flexuart[idx - DRIVER_UART0].registered)) return -ENODEV;

    /* 2.执行命令 */
    switch(cmd){
    case _IOC_SET_CB:
        if(unlikely(NULL == args)) return -EINVAL;
        flexuart[idx - DRIVER_UART0].rx_handler = (ioctl_cb_t)args;
        break;
    case _IOC_SET:
        if(unlikely(NULL == args)) return -EINVAL;
        spcflexuart_config(idx, args, len);
        break;
    default:
        return -ENODEV;
    }

    (void)idx;
    return 0;
}

/*************************************************************************************
* FunctionName   : spcflexuart_rx()
* Description    : UART驱动任务
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t spcflexuart_rx(void)
{
    uint8_t idx = 0;
    struct flexuart_rx_s *qdata = NULL;

    /* 1.遍历所有串口 */
    for (idx = 0; idx < MAX_FLEX_UART_NUM; idx++) {
        /* 2.串口未注册，跳过 */
        if(unlikely(!flexuart[idx].registered)) continue;

        /* 3.处理接收数据 */
        if(0 == fqueue_pop(flexuart[idx].rx_queue, &qdata, 0, true)) {
            stdio_echo(flexuart[idx].sdp, qdata->data, qdata->len);

            if(unlikely(NULL != flexuart[idx].rx_handler)) {
                flexuart[idx].rx_handler(idx + DRIVER_UART0, qdata->data, qdata->len);
            }

            mem_free(qdata);
        }
    }

    return 0;
}

/**************************************************************************************
* FunctionName   : spcflexuart_suspend()
* Description    : flexuart进入低功耗
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init spcflexuart_lowpower(uint8_t mode)
{
    // 关闭UART
    sd_lld_stop(&SD1);
    sd_lld_stop(&SD2);
    sd_lld_stop(&SD3);
    sd_lld_stop(&SD4);
    sd_lld_stop(&SD5);
    sd_lld_stop(&SD6);
    sd_lld_stop(&SD7);
    sd_lld_stop(&SD8);

    (void) mode;
    return 0;
}

/**************************************************************************************
* FunctionName   : spcflexuart_wakeup()
* Description    : flexuart从低功耗唤醒
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init spcflexuart_wakeup(void)
{
    /* 创建串口配置信息 */
    static SerialConfig uart_config = {
        .speed      = 115200,
        .mode       = SD_MODE_8BITS_PARITY_NONE,
        .api_mode   = SPC5_LIN_API_MODE_BUFFERED_IO,
        .tx_end_cb  = NULL,
        .rx_end_cb  = NULL,
        .dma_enable = FALSE,
        .dma_err_cb = NULL,
    };
    // 重新初始化UART
    sd_lld_start(&SD1, &uart_config);
    sd_lld_start(&SD2, &uart_config);
    sd_lld_start(&SD3, &uart_config);
    sd_lld_start(&SD4, &uart_config);
    sd_lld_start(&SD5, &uart_config);
    sd_lld_start(&SD6, &uart_config);
    sd_lld_start(&SD7, &uart_config);
    sd_lld_start(&SD8, &uart_config);

    return 0;
}

/*************************************************************************************
* FunctionName   : spcflexuart_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
*************************************************************************************/
static int32_t __init spcflexuart_init(void)
{
    /* 1.初始化物理接口 */
    sd_lld_init();
    return 0;
}

/**************************************************************************************
* Description    : 注册串口驱动
**************************************************************************************/
REGISTER_UART(1, DRIVER_UART0, spcflexuart_write, spcflexuart_ioctrl,
        UART1_BAUD, UART1_PARITY, flexuart, spc_rx_callback, spc_timer_callback);
REGISTER_UART(2, DRIVER_UART1, spcflexuart_write, spcflexuart_ioctrl,
        UART2_BAUD, UART2_PARITY, flexuart, spc_rx_callback, spc_timer_callback);
REGISTER_UART(3, DRIVER_UART2, spcflexuart_write, spcflexuart_ioctrl,
        UART3_BAUD, UART3_PARITY, flexuart, spc_rx_callback, spc_timer_callback);
REGISTER_UART(4, DRIVER_UART3, spcflexuart_write, spcflexuart_ioctrl,
        UART4_BAUD, UART4_PARITY, flexuart, spc_rx_callback, spc_timer_callback);
REGISTER_UART(5, DRIVER_UART4, spcflexuart_write, spcflexuart_ioctrl,
        UART5_BAUD, UART5_PARITY, flexuart, spc_rx_callback, spc_timer_callback);
REGISTER_UART(6, DRIVER_UART5, spcflexuart_write, spcflexuart_ioctrl,
        UART6_BAUD, UART6_PARITY, flexuart, spc_rx_callback, spc_timer_callback);
REGISTER_UART(7, DRIVER_UART6, spcflexuart_write, spcflexuart_ioctrl,
        UART7_BAUD, UART7_PARITY, flexuart, spc_rx_callback, spc_timer_callback);
REGISTER_UART(8, DRIVER_UART7, spcflexuart_write, spcflexuart_ioctrl,
        UART8_BAUD, UART8_PARITY, flexuart, spc_rx_callback, spc_timer_callback);


/**************************************************************************************
* Description    : 定义串口初始化模块
**************************************************************************************/
static __const struct driver spc_flexuart = {
    .init  = spcflexuart_init,
    .run   = spcflexuart_rx,
    .lowpower = spcflexuart_lowpower,
    .wakeup = spcflexuart_wakeup,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(spc_flexuart);
