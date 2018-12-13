/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_irq.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_IRQ_H__
#define __FRTOS_IRQ_H__

#include "frtos_types.h"
#include "frtos_list.h"

/**************************************************************************************
* Description    : 驱动层触发方式定义(需要驱动层实现)
**************************************************************************************/
#define IRQ_TRIG_DISABLE            0                   // 中断触发关闭
#define IRQ_TRIG_UP                 1                   // 上升沿触发
#define IRQ_TRIG_DOWN               2                   // 下降沿触发
#define IRQ_TRIG_EDGE               3                   // 任意边沿触发
#define IRQ_TRIG_LOW                4                   // 低电平触发
#define IRQ_TRIG_HIGH               5                   // 高电平触发

#define IRQ_TRIG_SHARED             0x80                // 共享中断

/**************************************************************************************
* FunctionName   : irq_cb_t()
* Description    : 中断回调类型
* EntryParameter : irq,中断号
* ReturnValue    : None
**************************************************************************************/
typedef void (*irq_cb_t)(uint32_t irq);

/**************************************************************************************
* Description    : 中断标志数据类型
**************************************************************************************/
struct irq_reg_s{
    uint32_t irq;                                        // 中断号
    uint8_t trig;                                        // 触发模式
    irq_cb_t handler;                                    // 中断句柄
    struct list_head list;                               // 链表
};

/**************************************************************************************
* FunctionName   : irq_set_trigger()
* Description    : 中断触发模式设置(需要外部实现)
* EntryParameter : irq,中断号, trig,触发模式
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_set_trigger(uint8_t irq, uint8_t trig);

/**************************************************************************************
* FunctionName   : irq_fast_dispatch()
* Description    : 快速中断服务调度器
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t irq_fast_dispatch(uint32_t irq);

/**************************************************************************************
* FunctionName   : free_irq()
* Description    : 释放中断函数
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t free_irq(uint32_t irq);

/**************************************************************************************
* FunctionName   : request_irq()
* Description    : 注册中断函数
* EntryParameter : irq,中断号, trig，触发模式，handler，中断函数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t request_irq(uint32_t irq, uint8_t trig, irq_cb_t handler);

/**************************************************************************************
* FunctionName   : irq_enable()
* Description    : 使能中断触发模式(需要外部实现)
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_enable(uint8_t irq);

/**************************************************************************************
* FunctionName   : irq_disable()
* Description    : 禁用中断触发模式(需要外部实现)
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_disable(uint8_t irq);

/**************************************************************************************
* FunctionName   : local_irq_disable()
* Description    : 禁用本地所有中断
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t local_irq_disable(void);

/**************************************************************************************
* FunctionName   : local_irq_enable()
* Description    : 使能本地所有中断
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t local_irq_enable(void);

#endif /* __IRQ_H__ */

