/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_dlyrun.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_time.h"
#include "frtos_errno.h"
#include "frtos_softimer.h"
#include "frtos_lock.h"
#include "frtos_irq.h"
#include "frtos_tasklet.h"
#include "frtos_drivers.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static softimer_t tasklet_timer = NULL;                /* 定时器 */

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static struct list_head tasklet_list = LIST_HEAD_INIT(tasklet_list);

/**************************************************************************************
* FunctionName   : tasklet_cancel()
* Description    : 取消延后执行函数
* EntryParameter : work,工作队列
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t tasklet_cancel(struct workqueue *work)
{
    list_del(&(work->head));

    return 0;
}

/**************************************************************************************
* FunctionName   : tasklet_init()
* Description    : 调度函数
* EntryParameter : work,工作队列, fn, 工作队列函数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t tasklet_init(struct workqueue *work, workqueue_fn_t fn)
{
    work->args = NULL;
    work->jffies = 0;
    work->run = fn;

    return 0;
}

/**************************************************************************************
* FunctionName   : tasklet_schedule()
* Description    : 调度函数
* EntryParameter : work,工作队列, args,传入参数， ms,延时毫秒数
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t tasklet_schedule(struct workqueue *work, void *args, uint32_t ms)
{
    uint32_t jffies = 0;

    // 1.填充数据
    jffies = time_gettick() + time_ms2tick(ms);
    if(work->jffies > 0 && work->args == args &&
            true != time_after(int32_t, jffies, work->jffies)) {
        work->jffies = jffies;
    }
    if(likely(work->jffies == 0)) {
        work->args = args;
        work->jffies = jffies;
        // 2.添加到发送链表
        list_add_tail(&work->head, &tasklet_list);
    }

    return 0;
}

/*************************************************************************************
* FunctionName   : tasklet_isr()
* Description    : tasklet中断回调
* EntryParameter : None
* ReturnValue    : 返回错误码
*************************************************************************************/
static void tasklet_isr(softimer_t timer)
{
    void *args;
    struct workqueue *work = NULL, *tmp = NULL;

    list_for_each_entry_safe(work, tmp, &tasklet_list, head) {
        if(true != time_after(int32_t, time_gettick(), work->jffies))
            continue;
        //local_irq_disable();
        args = work->args;
        work->jffies = 0;
        list_del(&(work->head));
        //local_irq_enable();

        work->run(args);
	}
    (void)timer;
}

/**************************************************************************************
* FunctionName   : init_tasklet()
* Description    : 初始化tasklet功能
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __init init_tasklet(void)
{
    tasklet_timer = softimer_create(tasklet_isr, SOFTIMER_RELOAD_ENABLE, 10);
    if(NULL == tasklet_timer) return -EMEM;

    return softimer_start(tasklet_timer, 0);
}

static __const struct driver frtos_tasklet = {
    .init = init_tasklet,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
EARLY_INIT(frtos_tasklet);
