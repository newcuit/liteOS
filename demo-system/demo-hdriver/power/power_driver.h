/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : power_driver.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __POWER_DRIVER_H__
#define __POWER_DRIVER_H__

#include "frtos_types.h"
#include "components.h"
#include "spc560b_registry.h"

/**************************************************************************************
* Description    : 电源控制命令定义
**************************************************************************************/
#define POWER_CMD_MPU_ON              0                 // MPU电源开
#define POWER_CMD_MPU_OFF             1                 // MPU电源关
#define POWER_CMD_5V_ON               2                 // 5V电源开
#define POWER_CMD_5V_OFF              3                 // 5V电源关
#define POWER_CMD_WDG_ON              4                 // WDG电源开
#define POWER_CMD_WDG_OFF             5                 // WDG电源关
#define POWER_CMD_4G_ON               6                 // 4G电源开
#define POWER_CMD_4G_OFF              7                 // 4G电源关
#define POWER_CMD_EXT_ON              8                 // 外部电源开
#define POWER_CMD_EXT_OFF             9                 // 外部电源关
#define POWER_CMD_3V_ON               10                // 3V电源开
#define POWER_CMD_3V_OFF              11                // 3V电源关
#define POWER_CMD_VIDEO_ON            12                // VIDEO电源开
#define POWER_CMD_VIDEO_OFF           13                // VIDEO电源关
#define POWER_CMD_CALLBACK            14                // 注册中断回调函数

/**************************************************************************************
* Description    : 定义电源控制引脚
**************************************************************************************/
#define POWER_PIN_MPU                 PORT_F, PTF10                 // MPU电源
#define POWER_PIN_5V                  PORT_F, PTF6                  // 5V电源
#define POWER_PIN_WDG                 PORT_E, PTE2                  // WDG电源
#define POWER_PIN_4G1                 PORT_F, PTF4                  // 4G电源
#define POWER_PIN_4G2                 PORT_F, PTF5                  // 4G电源
#define POWER_PIN_EXT                 PORT_E, PTE15                 // 外部3V电源
#define POWER_PIN_3V                  PORT_A, PTA14                 // MPU 3V电源
#define POWER_PIN_VIDEO3V             PORT_G, PTG14                 // VIDEO电源
#define POWER_PIN_VIDEO12V            PORT_E, PTE12                 // VIDEO电源

#define MODE_SUSPEND                  0                 // CPU挂起状态
#define MODE_RUNNING                  1                 // CPU运行状态
#define WKUP_IRQ_MAX                  29                // 最多的唤醒中断数量
#define WKUP_IRQ_PRIO                 7                 // 设置唤醒中断优先级

/**************************************************************************************
* Description    : 定义休眠唤醒中断服务函数
**************************************************************************************/
typedef void (*wkup_isr)(int8_t irq, uint8_t status);

/**************************************************************************************
* Description    : 定义中断服务结构
**************************************************************************************/
struct wkup_config {
    int8_t wkup_no;
    uint8_t falling_edge;
    uint8_t rising_edge;
    wkup_isr cb;
};

#endif /* __POWER_DRIVER_H__ */

