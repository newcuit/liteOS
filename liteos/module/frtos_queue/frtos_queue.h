/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_queue.h
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_QUEUE_H__
#define __FRTOS_QUEUE_H__

#include "frtos_types.h"
#include "frtos_errno.h"

#ifdef USING_OS_FREERTOS
/**************************************************************************************
* Description    : 自定义数据类型定义
**************************************************************************************/
typedef xQueueHandle fqueue_t;                  // 队列数据类型

/**************************************************************************************
* FunctionName   : fqueue_create()
* Description    : 创建队列
* EntryParameter : len,队列长度, item_size,项目字节数
* ReturnValue    : 返回创建好的队列指针, NULL,创建失败
**************************************************************************************/
static inline fqueue_t fqueue_create(uint32_t len, uint32_t item_size)
{
    return xQueueCreate(len, item_size);
}

/**************************************************************************************
* FunctionName   : fqueue_push()
* Description    : 入队列
* EntryParameter : queue,队列指针, *txitem,入队列项目指针, ms,等待毫秒
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t fqueue_push(fqueue_t queue, void *txitem, uint32_t ms)
{
    if(NULL == queue || NULL == txitem) return -EMEM;
    if(pdPASS != xQueueSend(queue, txitem, ms)) return -ENOMEM;

    return 0;
}

/**************************************************************************************
* FunctionName   : fqueue_pop()
* Description    : 出队列
* EntryParameter : queue,队列指针, *rxitem,出队列项目指针, ms,等待毫秒,
                   del,从队列删除(true,删除, false,不删除)
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t fqueue_pop(fqueue_t queue, void *rxitem, uint32_t ms, bool del)
{
    if(NULL == queue || NULL == rxitem) return -EMEM;

    if(true == del){
        if(pdPASS != xQueueReceive(queue, rxitem, ms)) return -EEMPTY;
    }else{
        if(pdPASS != xQueuePeek(queue, rxitem, ms))    return -EEMPTY;
    }

    return 0;
}
#else
#include "frtos_list.h"

/**************************************************************************************
* Description    : 定义队列头部
**************************************************************************************/
typedef struct {
    int8_t depth;
    struct list_head list;
} *fqueue_t;                  // 队列数据类型

/**************************************************************************************
* Description    : 定义队列消息体
**************************************************************************************/
typedef struct {
    void *data;
    struct list_head list;
} fqueue_item_t;

/**************************************************************************************
* FunctionName   : fqueue_create()
* Description    : 创建队列
* EntryParameter : len,队列长度, item_size,项目字节数
* ReturnValue    : 返回创建好的队列指针, NULL,创建失败
**************************************************************************************/
static inline fqueue_t fqueue_create(uint32_t len, uint32_t item_size)
{
    fqueue_t queue_head = NULL;

    queue_head = mem_malloc(sizeof(fqueue_t));
    if(unlikely(queue_head == NULL)) return NULL;

    queue_head->len = len;
    INIT_LIST_HEAD(&queue_head->list);
    (void)item_size;

    return queue_head;
}

/**************************************************************************************
* FunctionName   : fqueue_push()
* Description    : 入队列
* EntryParameter : queue,队列指针, *txitem,入队列项目指针, ms,等待毫秒
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t fqueue_push(fqueue_t queue, void *txitem, uint32_t ms)
{
    fqueue_item_t *item = NULL;

    if(unlikely(queue_head->len <= 0)) return -EFULL;

    item = mem_malloc(sizeof(fqueue_item_t));
    if(unlikely(item == NULL)) return -EMEM;

    item->data = txitem;
    list_add(&item->list, &queue_head->list);
    queue_head->len--;
    (void)ms;

    return 0;
}

/**************************************************************************************
* FunctionName   : fqueue_pop()
* Description    : 出队列
* EntryParameter : queue,队列指针, *rxitem,出队列项目指针, ms,等待毫秒,
                   del,从队列删除(true,删除, false,不删除)
* ReturnValue    : 返回错误码
**************************************************************************************/
static inline int8_t fqueue_pop(fqueue_t queue, void *rxitem, uint32_t ms, bool del)
{
    fqueue_item_t *item = list_first_entry(&queue_head->list, struct list_head, list);

    if(unlikely(item == NULL)) return -EEMPTY;
    rxitem = &item->data;

    if(del) {list_del(&item->list);mem_free(item);queue_head->len++;}
    else list_move_tail(&item->list, &queue_head->list);

    return 0;
}
#endif
#endif /* __FRTOS_QUEUE_H__ */

