/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_app.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_APP_H__
#define __FRTOS_APP_H__

#include "string.h"
#include "frtos_types.h"
#include "frtos_errno.h"
#include "frtos_mem.h"

/**************************************************************************************
* Description    : 任务模式优先级
**************************************************************************************/
#define TASK_DEFULT_PRO                             2
#define THREAD_STACK_SIZE                           512

#define APP_MGR_TASK_TIME                           5               // 管理任务查询时间(毫秒)

/**************************************************************************************
* TypeName       : task_run_t()
* Description    : 独立任务运行函数
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef void (*task_run_t)(void *args);

/**************************************************************************************
* TypeName       : app_init_t()
* Description    : 轻量级任务初始化函数
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*app_init_t)(void);

/**************************************************************************************
* TypeName       : app_run_t()
* Description    : 轻量级任务周期运行函数
* EntryParameter : None
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
typedef int32_t (*app_run_t)(void);

/**************************************************************************************
* TypeName       : data_set_t()
* Description    : 数据设置，应用id号， 写入数据缓冲， 写入数据长度
* EntryParameter : None
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
typedef int32_t (*data_set_t)(uint8_t, void *, int32_t);

/**************************************************************************************
* TypeName       : data_get_t()
* Description    : 数据获取，应用id号， 读取数据缓冲， 读取数据长度
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*data_get_t)(uint8_t, void *, int32_t);

/**************************************************************************************
* TypeName       : debug_cmd_t()
* Description    : 应用debug函数
* EntryParameter : argv，应用debug参数， argc， 应用debug参数长度
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*debug_cmd_t)(char *argv, int32_t argc);

/**************************************************************************************
* TypeName       : thread_fun_t()
* Description    : 线程定义
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*thread_fun_t)(void);

/**************************************************************************************
* Description    : 独立任务初始化结构定义
**************************************************************************************/
struct task {
    __const uint8_t idx;                        // 独立任务id
    __const int8_t pro;                         // 独立任务优先级
    __const uint16_t depth;                     // 独立任务栈深度
    __const char *name;                         // 独立任务名称
    __const task_run_t main;                    // 独立任务运行函数
    __const data_set_t set;                     // 数据设置函数
    __const data_get_t get;                     // 数据获取函数
    __const debug_cmd_t debug;                  // 应用debug接口
};

/**************************************************************************************
* Description    : 轻量级任务初始化结构定义
**************************************************************************************/
struct applite {
    __const uint8_t idx;                        // 轻量级任务id
    __const char *name;                         // 轻量级任务名称
    __const app_init_t init;                    // 轻量级任务初始化函数
    __const app_run_t run;                      // 轻量级任务运行函数
    __const data_set_t set;                     // 数据设置函数
    __const data_get_t get;                     // 数据获取函数
    __const debug_cmd_t debug;                  // 应用debug接口
    uint32_t used_tick;                         // 每次运行占用tick数据
};

/**************************************************************************************
* Description    : 运行任务结构定义
**************************************************************************************/
struct thread_t {
    uint8_t idx;                        // 索引
    void *data;                         // 内容
    int32_t len;                        // 内容长度
    thread_fun_t func;                       // 运行函数
};

/**************************************************************************************
* TypeName       : APP_REGISTER()
* Description    : 轻量级任务注册函数
* EntryParameter : 轻量级任务结构体
* ReturnValue    : None
**************************************************************************************/
#define APP_REGISTER(applites) \
    static __const struct applite* __attribute__((used,section(".app_list_chain"))) \
	__applite_##applites##__ = &applites

/**************************************************************************************
* TypeName       : TASK_REGISTER()
* Description    : 独立任务注册函数
* EntryParameter : 独立任务结构体
* ReturnValue    : None
**************************************************************************************/
#define TASK_REGISTER(tasks) \
    static __const struct task* __attribute__((used,section(".task_list_chain"))) \
	__task_##tasks##__ = &tasks

/**************************************************************************************
* MacroName      : app_thread_run
* Description    : 系统线程调用
* EntryParameter : args, 用户函数线程
* ReturnValue    : 返回None
**************************************************************************************/
void app_thread_run(void *args);

/**************************************************************************************
* FunctionName   : thread_run()
* Description    : 动态运行一个任务
* EntryParameter : app,指向该任务的调度函数， depth，指定当前任务的栈深度
* ReturnValue    : 返回None
**************************************************************************************/
static inline int32_t thread_run(thread_fun_t app,uint16_t stack_size,  uint8_t prio)
{
    xTaskCreate(app_thread_run, NULL, stack_size, (void *)app, prio, NULL);
    return 0;
}

#endif /* __FRTOS_APP_H__ */
