/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : power_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_utils.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "frtos_delay.h"
#include "power_driver.h"
#include "gpio_driver.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define sleep                0
#define suspend              1

/**************************************************************************************
* Description    : 定义中断服务结构
**************************************************************************************/
static wkup_isr spc_wkup_isr[WKUP_IRQ_MAX];

/**************************************************************************************
* FunctionName   : spc_wkup_config()
* Description    : 配置唤醒脚中断
* EntryParameter : wkconfig,配置结构
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t spc_wkup_config(struct wkup_config *wkconfig)
{
    uint8_t wkup_no = wkconfig->wkup_no;

    if(unlikely(wkconfig->wkup_no >= WKUP_IRQ_MAX)) {
        return -EINVAL;
    }

    // 1.设置中断服务函数
    spc_wkup_isr[wkconfig->wkup_no] = wkconfig->cb;

    // 2.使能对应唤醒脚中断
    WKUP.WRER.R |= (0x1 << wkup_no);

    // 3.使能沿触发模式
    if(wkconfig->rising_edge) {
        WKUP.WIREER.R |= (0x1 << wkup_no);
    } else {
        WKUP.WIREER.R &= ~(0x1 << wkup_no);
    }
    if(wkconfig->falling_edge) {
        WKUP.WIFEER.R |= (0x1 << wkup_no);
    } else {
        WKUP.WIFEER.R &= ~(0x1 << wkup_no);
    }
    // 4.使能过滤器
    WKUP.WIFER.R |= (0x1 << wkup_no);

    // 5. 非唤醒引脚上拉
    WKUP.WIPUER.R &= ~(0x1 << wkup_no);

    // 6.清除中断标记
    WKUP.WISR.R = 0xFFFFFFFFU;

    return 0;
}

/**************************************************************************************
* FunctionName   : power_ioctrl()
* Description    : 控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t power_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    uint8_t value = *((uint8_t *)args);

    if(unlikely((NULL == args && len != 0) || \
        (NULL != args && 0 == len) || len < 0)){
        return -EINVAL;
    }

    // 1.执行命令序列
    switch(cmd){
    case POWER_CMD_CALLBACK:
        if(likely(sizeof(struct wkup_config) == len)) {
            return spc_wkup_config((struct wkup_config *)args);
        }
        break;
    case POWER_CMD_MPU_ON:
    case POWER_CMD_MPU_OFF:
        gpio_pin_set(POWER_PIN_MPU, value);
        break;
    case POWER_CMD_5V_ON:
    case POWER_CMD_5V_OFF:
        gpio_pin_set(POWER_PIN_5V, value);
        break;
    case POWER_CMD_WDG_ON:
    case POWER_CMD_WDG_OFF:
        gpio_pin_set(POWER_PIN_WDG, value);
        break;
    case POWER_CMD_4G_ON:
    case POWER_CMD_4G_OFF:
        gpio_pin_set(POWER_PIN_4G1, value);
        gpio_pin_set(POWER_PIN_4G2, value);
        break;
    case POWER_CMD_EXT_ON:
    case POWER_CMD_EXT_OFF:
        gpio_pin_set(POWER_PIN_EXT, value);
        break;
    case POWER_CMD_3V_ON:
    case POWER_CMD_3V_OFF:
        gpio_pin_set(POWER_PIN_3V, value);
        break;
    case POWER_CMD_VIDEO_ON:
    case POWER_CMD_VIDEO_OFF:
        gpio_pin_set(POWER_PIN_VIDEO3V, value);
        gpio_pin_set(POWER_PIN_VIDEO12V, value);
        break;
    default:
        return -EINVAL;
    }

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : board_power_wake()
* Description    : 打开所有电源开关
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void board_power_wake(void)
{
    gpio_pin_set(POWER_PIN_MPU, 1);
    gpio_pin_set(POWER_PIN_5V, 1);
    gpio_pin_set(POWER_PIN_WDG, 0);
    gpio_pin_set(POWER_PIN_4G1, 0);
    gpio_pin_set(POWER_PIN_4G2, 1);
    gpio_pin_set(POWER_PIN_EXT, 1);
    gpio_pin_set(POWER_PIN_3V, 1);
    gpio_pin_set(POWER_PIN_VIDEO3V, 0);
    gpio_pin_set(POWER_PIN_VIDEO12V, 0);
}

/**************************************************************************************
* FunctionName   : board_power_lowpower()
* Description    : 根据配置关闭相关电源
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void board_power_lowpower(uint8_t mode)
{
    gpio_pin_set(POWER_PIN_MPU, 0);
    gpio_pin_set(POWER_PIN_5V, 0);

    // 根据mode选择是否要关闭4G
    if(mode == suspend) gpio_pin_set(POWER_PIN_4G1, 0);
    else gpio_pin_set(POWER_PIN_4G1, 1);

    gpio_pin_set(POWER_PIN_4G2, 0);
    gpio_pin_set(POWER_PIN_EXT, 0);
    gpio_pin_set(POWER_PIN_3V, 0);
    gpio_pin_set(POWER_PIN_VIDEO3V, 0);
    gpio_pin_set(POWER_PIN_VIDEO12V, 0);
}

/**************************************************************************************
* TypeName       : power_lowpower()
* Description    : 驱动进入低功耗模式类型
* EntryParameter : mode
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t power_lowpower(uint8_t mode)
{
    board_power_lowpower(mode);
    return 0;
}

/**************************************************************************************
* TypeName       : power_wakeup()
* Description    : 驱动唤醒函数类型
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t power_wakeup(void)
{
    board_power_wake();
    return 0;
}

/**************************************************************************************
* TypeName       : spc_cpu_state()
* Description    : 获取CPU当前状态
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static uint8_t spc_cpu_state(void)
{
    uint8_t cpu_mode = MODE_RUNNING;

    switch(ME.GS.B.CURRENTMODE) {
    case SPC5_RUNMODE_SAFE:
    case SPC5_RUNMODE_DRUN:
    case SPC5_RUNMODE_RUN0:
    case SPC5_RUNMODE_RUN1:
    case SPC5_RUNMODE_RUN2:
    case SPC5_RUNMODE_RUN3:
        cpu_mode = MODE_RUNNING;
        break;
    case SPC5_RUNMODE_HALT:
    case SPC5_RUNMODE_STOP:
    case SPC5_RUNMODE_STANDBY:
        cpu_mode = MODE_SUSPEND;
        break;
    }

    return cpu_mode;
}

/**************************************************************************************
* TypeName       : spc_pm_irq()
* Description    : 唤醒/中断函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void spc_pm_irq(void)
{
    uint8_t idx = 0;
    uint32_t wisr = 0;

    IRQ_PROLOGUE();

    // 1. 读取中断状态，并清除中断标记
    osalEnterCriticalFromISR();
    wisr = WKUP.WISR.R;
    WKUP.WISR.R = wisr;
    osalExitCriticalFromISR();

    // 2. 调用对应的中断服务函数
    for(idx = 0; idx < WKUP_IRQ_MAX; idx++) {
        if(unlikely((wisr & (0x1<<idx)) && (spc_wkup_isr[idx] != NULL))) {
            spc_wkup_isr[idx](idx, spc_cpu_state());
        }
    }
    IRQ_EPILOGUE();
}

/**************************************************************************************
* FunctionName   : power_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init power_init(void)
{
    // 1.设置唤醒模块时钟配置
    SPCSetPeripheralClockMode(69,SPC5_ME_PCTL_RUN(1) | SPC5_ME_PCTL_LP(1));

    // 2.设置唤醒中断优先级
    INTC_PSR(46) = WKUP_IRQ_PRIO;
    INTC_PSR(47) = WKUP_IRQ_PRIO;
    INTC_PSR(48) = WKUP_IRQ_PRIO;
    INTC_PSR(49) = WKUP_IRQ_PRIO;

    // 3.打开所有电源开关
    board_power_wake();

    return 0;
}

static __const struct driver spc_power = {
    .idx     = DRIVER_PM,
    .name    = "pm",
    .init    = power_init,
    .ioctl   = power_ioctrl,
    .wakeup  = power_wakeup,
    .lowpower = power_lowpower,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
CORE_INIT(spc_power);

/**************************************************************************************
* Description    : 安装唤醒中断服务函数
**************************************************************************************/
SETUP_IRQ(46, spc_pm_irq);
SETUP_IRQ(47, spc_pm_irq);
SETUP_IRQ(48, spc_pm_irq);
SETUP_IRQ(49, spc_pm_irq);
