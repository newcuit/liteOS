/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_app.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/

#include "frtos_mem.h"
#include "frtos_list.h"
#include "frtos_app.h"
#include "frtos_errno.h"
#include "frtos_delay.h"

/**************************************************************************************
* MacroName      : app_thread_run
* Description    : 系统线程调用
* EntryParameter : args, 用户函数线程
* ReturnValue    : 返回None
**************************************************************************************/
void __default app_thread_run(void *args)
{
	thread_fun_t func = (thread_fun_t)args;

    func();
    vTaskDelete(NULL);
}
