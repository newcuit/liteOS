/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : irq.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "config_driver.h"
#include "components.h"
#include "frtos_irq.h"
#include "eirq_cfg.h"
#include "eirq.h"
#include "irq.h"

/**************************************************************************************
* Description    : 引入外部数据变量
**************************************************************************************/
extern eirq_config1 ext_irqconf[];

/**************************************************************************************
* FunctionName   : irq_enable()
* Description    : 使能中断触发模式(需要外部实现)
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_enable(uint8_t irq)
{
    SIU.IRER.R |= (1UL << irq);
    return 0;
}

/**************************************************************************************
* FunctionName   : irq_disable()
* Description    : 禁用中断触发模式(需要外部实现)
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_disable(uint8_t irq)
{
    SIU.IRER.R &= (~(1UL << irq));
    return 0;
}

/**************************************************************************************
* FunctionName   : local_irq_disable()
* Description    : 禁用本地所有中断
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t local_irq_disable(void)
{
    irqIsrDisable();
    return 0;
}

/**************************************************************************************
* FunctionName   : local_irq_enable()
* Description    : 使能本地所有中断
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t local_irq_enable(void)
{
    irqIsrEnable();
    return 0;
}

/**************************************************************************************
* FunctionName   : irq_set_trigger()
* Description    : 中断触发模式设置(需要外部实现)
* EntryParameter : irq,中断号, trig,触发模式
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t irq_set_trigger(uint8_t irq, uint8_t trig)
{
    switch(trig){
    case IRQ_TRIG_DISABLE:
        SIU.IRER.R &= (~(1UL << irq));break;
    case IRQ_TRIG_UP:
        SIU.IREER.R |= (1UL << irq);break;
    case IRQ_TRIG_DOWN:
        SIU.IFEER.R |= (1UL << irq);break;
    case IRQ_TRIG_EDGE:
        SIU.IREER.R |= (1UL << irq);
        SIU.IFEER.R |= (1UL << irq);
        break;
    case IRQ_TRIG_LOW:
        break;
    case IRQ_TRIG_HIGH:
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

/**************************************************************************************
* FunctionName   : request_irq()
* Description    : 注册中断函数
* EntryParameter : irq,中断号, trig，触发模式，handler，中断函数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t request_irq(uint32_t irq, uint8_t trig, irq_cb_t handler)
{
    uint8_t i = 0;

    while(ext_irqconf[i].eirqNumber != -1 && i++ < EXT_MAX_IRQ_NUM);

    if(unlikely(i >= EXT_MAX_IRQ_NUM)) return -EBUSY;

    ext_irqconf[i].eirqNumber = irq;
    ext_irqconf[i].callback = handler;
    ext_irqconf[i].trig = trig & (~IRQ_TRIG_SHARED);

    irq_set_trigger(irq, ext_irqconf[i].trig);
    irq_enable(irq);

    return 0;
}

/**************************************************************************************
* FunctionName   : free_irq()
* Description    : 释放中断函数
* EntryParameter : irq,中断号
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t free_irq(uint32_t irq)
{
    uint8_t i = 0;

    irq_disable(irq);
    while(likely(i < EXT_MAX_IRQ_NUM)) {
        if(unlikely(ext_irqconf[i].eirqNumber == (int32_t)irq)) {
            ext_irqconf[i].eirqNumber = -1;
            ext_irqconf[i].trig = 0;
            ext_irqconf[i].callback = NULL;
        }
        i++;
    }

    return -ENODEV;
}

/**************************************************************************************
* FunctionName   : irq_ioctrl()
* Description    : irq应用控制
* EntryParameter : *args,参数, len,参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t irq_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    struct irq_reg_s *irq;

    if(unlikely((NULL == args && len > 0) || \
        (NULL != args && len != sizeof(struct irq_reg_s)) || len < 0)){
        return -EINVAL;
    }
    irq = (struct irq_reg_s *)args;

    (void)idx;
    (void)cmd;
    return request_irq(irq->irq, irq->trig, irq->handler);
}

/*************************************************************************************
* FunctionName   : irq_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static int32_t __init irq_init(void)
{
    /* 1.初始化中断*/
    irqInit();
    eirqInit();
    irqIsrEnable();
    return 0;
}

/*************************************************************************************
* Description    : 模块初始化
*************************************************************************************/
static __const struct driver spc_irq = {
    .idx   = DRIVER_GPIOINT,
    .name  = "irq",
    .init  = irq_init,
    .ioctl = irq_ioctrl,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(spc_irq);
