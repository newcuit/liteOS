/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : system.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/

#include "frtos_mem.h"
#include "frtos_list.h"
#include "frtos_app.h"
#include "frtos_errno.h"
#include "frtos_sys.h"
#include "frtos_log.h"
#include "frtos_delay.h"
#include "frtos_drivers.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define sleep                               0
#define suspend                             1
#define APPLITE_DEPTH_MGR                   1024                // 管理任务栈深度

/**************************************************************************************
* Description    : 驱动模块安装地址
**************************************************************************************/
extern struct driver *__DRIVER_LIST0_S__[];
extern struct driver *__DRIVER_LIST7_E__[];

/**************************************************************************************
* Description    : 轻量级任务安装地址
**************************************************************************************/
extern struct applite * __APP_LIST_CHAIN_S__[];
extern struct applite * __APP_LIST_CHAIN_E__[];

#ifdef USING_OS_FREERTOS
/**************************************************************************************
* Description    : 普通任务安装地址
**************************************************************************************/
extern struct task * __TASK_LIST_CHAIN_S__[];
extern struct task * __TASK_LIST_CHAIN_E__[];

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
static TaskHandle_t mgrhdle = NULL;           // 系统管理任务堆栈

/**************************************************************************************
* Description    : 定义系统堆内存
**************************************************************************************/
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
uint8_t __attribute__((section(".ucheap")))  ucHeap[FRTOS_UCHEAP_SIZE];
#endif

/**************************************************************************************
* FunctionName   : vApplicationStackOverflowHook()
* Description    : 栈溢出回调函数
* EntryParameter : xTask,任务, pcTaskName,任务名称
* ReturnValue    : None
**************************************************************************************/
void __default vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;(void)pcTaskName;while(1);
}

/**************************************************************************************
* FunctionName   : vApplicationMallocFailedHook()
* Description    : Malloc失败回调函数
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void __default vApplicationMallocFailedHook(void)
{
    while(1);
}

/**************************************************************************************
* FunctionName   : StartIdleMonitor()
* Description    : 启动空闲模拟器
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void __default StartIdleMonitor (void)
{
}

/**************************************************************************************
* FunctionName   : EndIdleMonitor()
* Description    : 结束空闲模拟器
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void __default EndIdleMonitor (void)
{
}

/**************************************************************************************
* FunctionName   : vApplicationTickHook()
* Description    : TickHook
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void __default vApplicationTickHook( void )
{
}
#endif

/**************************************************************************************
* MacroName      : arch_lowpower
* Description    : 系统CPU进入休眠模式
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_lowpower(void)
{
    return 0;
}

/**************************************************************************************
* MacroName      : arch_wakeup
* Description    : 系统CPU唤醒
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_wakeup(void)
{
    return 0;
}

/**************************************************************************************
* MacroName      : arch_init
* Description    : 系统CPU初始化早期功能
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_init(void)
{
    return 0;
}

/**************************************************************************************
* MacroName      : arch_post
* Description    : 系统CPU初始化就绪状态
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_post(void)
{
    return 0;
}

/**************************************************************************************
* MacroName      : applite_init
* Description    : 系统CPU轻量级任务预处理
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default applite_init(void)
{
    return 0;
}

/**************************************************************************************
* MacroName      : applite_post
* Description    : 系统CPU初始化结束
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default applite_post(void)
{
    return 0;
}

#ifdef USING_OS_FREERTOS
/**************************************************************************************
* MacroName      : task_init
* Description    : 系统CPU普通任务预处理
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default task_init(void)
{
    return 0;
}

/**************************************************************************************
* MacroName      : task_post
* Description    : 系统CPU普通任务初始化完成
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default task_post(void)
{
    return 0;
}
#endif

/**************************************************************************************
* MacroName      : arch_hooks1
* Description    : 系统CPU执行一轮函数回调
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_hooks1(void)
{
    return 0;
}

/**************************************************************************************
* MacroName      : arch_hooks2
* Description    : 系统CPU执行一轮函数回调
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_hooks2(void)
{
    return 0;
}

/**************************************************************************************
* MacroName      : arch_reboot
* Description    : 系统重启
* EntryParameter : magic,幻术
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t __default arch_reboot(uint32_t magic)
{
    (void)magic;
    return 0;
}

/**************************************************************************************
* MacroName      : fuser_data_set
* Description    : 设置数据
* EntryParameter : idx,应用ID，data,设置应用数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fuser_data_set(uint8_t idx, void *data, int32_t len)
{
    struct applite **applites = NULL;
    struct task **tasks = NULL;

    // 1.查找轻量级任务应用程序
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++){
        if(unlikely((*applites)->idx == idx)) {
            if(likely((*applites)->set)) return (*applites)->set(idx, data, len);
            else return -EACCES;
        }
    }

    // 2.查找普通任务
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++){
        if(unlikely((*tasks)->idx == idx)) {
            if(likely((*tasks)->set)) return (*tasks)->set(idx, data, len);
            else return -EACCES;
        }
    }
    return -ESRCH;
}

/**************************************************************************************
* MacroName      : fuser_data_get
* Description    : 获取数据
* EntryParameter : idx,应用ID，data,获取应用数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fuser_data_get(uint8_t idx, void *data, int32_t len)
{
    struct applite **applites = NULL;
    struct task **tasks = NULL;

    // 2.查找轻量级任务应用程序
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++){
        if(unlikely((*applites)->idx == idx)) {
            if(likely((*applites)->get)) return (*applites)->get(idx, data, len);
            else return -EACCES;
        }
    }

    // 3.查找普通任务
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++){
        if(unlikely((*tasks)->idx == idx)) {
            if(likely((*tasks)->get)) return (*tasks)->get(idx, data, len);
            else return -EACCES;
        }
    }
    return -ESRCH;
}

/**************************************************************************************
* MacroName      : fdrive_read
* Description    : 读取驱动数据
* EntryParameter : idx,驱动ID，data,数据， len数据长度
* ReturnValue    : 返回错误码或者长度
**************************************************************************************/
int32_t fdrive_read(uint8_t idx, void *data, int32_t len)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(unlikely((*drivers)->idx == idx)) {
            if(likely((*drivers)->read)) return (*drivers)->read(idx, data, len);
            else return -EACCES;
        }
    }
    return -ENODEV;
}

/**************************************************************************************
* MacroName      : fdrive_write
* Description    : 写驱动数据
* EntryParameter : idx,驱动ID，data,数据， len数据长度
* ReturnValue    : 返回错误码或者写入长度
**************************************************************************************/
int32_t fdrive_write(uint8_t idx, void *data, int32_t len)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(unlikely((*drivers)->idx == idx)) {
            if(likely((*drivers)->write)) return (*drivers)->write(idx, data, len);
            else return -EACCES;
        }
    }
    return -ENODEV;
}

/**************************************************************************************
* MacroName      : fdrive_ioctl
* Description    : 控制驱动程序
* EntryParameter : idx,驱动ID，cmd,控制命令字，data,数据， len数据长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fdrive_ioctl(uint8_t idx,int32_t cmd, void *data, int32_t len)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(unlikely((*drivers)->idx == idx)) {
            if(likely((*drivers)->ioctl)) return (*drivers)->ioctl(idx, cmd, data, len);
            else return -EACCES;
        }
    }
    return -ENODEV;
}

/**************************************************************************************
* MacroName      : system_lowpower
* Description    : 控制驱动进入休眠
* EntryParameter : idx,驱动ID
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t system_lowpower(uint8_t mode)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST7_E__ - 1; drivers >= __DRIVER_LIST0_S__; drivers--){
        if((*drivers)->lowpower) (*drivers)->lowpower(mode);
    }
    arch_lowpower();

    return 0;
}

/**************************************************************************************
* MacroName      : system_wakeup
* Description    : 控制驱动唤醒
* EntryParameter : idx,驱动ID
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t system_wakeup(void)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if((*drivers)->wakeup) (*drivers)->wakeup();
    }
    return 0;
}

/**************************************************************************************
* MacroName      : lowpower_driver
* Description    : 控制驱动进入休眠
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t lowpower_driver(const int idx, uint8_t mode)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(unlikely((*drivers)->lowpower && (*drivers)->idx == idx)) {
            return (*drivers)->lowpower(mode);
        }
    }
    return 0;
}

/**************************************************************************************
* MacroName      : wakeup_driver
* Description    : 控制驱动唤醒
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t wakeup_driver(const int idx)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(unlikely((*drivers)->wakeup && (*drivers)->idx == idx)) {
            return (*drivers)->wakeup();
        }
    }
    return 0;
}

/**************************************************************************************
* MacroName      : fsystem_reboot
* Description    : 系统重启
* EntryParameter : magic,幻术
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t fsystem_reboot(uint32_t magic)
{
    struct driver **drivers = NULL;

    // 1.查找所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++){
        if(likely((*drivers)->stop)) (*drivers)->stop();
    }
    arch_reboot(magic);

    return -ENODEV;
}

/**************************************************************************************
* FunctionName   : app_master()
* Description    : 主任务
* EntryParameter : args,参数指针
* ReturnValue    : None
**************************************************************************************/
static void app_master(void * args)
{
    uint32_t c_tick = 0;
    struct driver **drivers = NULL;
    struct applite **applites = NULL;
#ifdef USING_OS_FREERTOS
    struct task **tasks = NULL;
#endif

    // 0.轻量级任务预处理
    applite_init();

    // 1.初始化轻量级任务应用程序
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++) {
        if(likely((*applites)->init)) ((*applites)->init)();
        stdio_printf("applite %s running"STRBR, (*applites)->name);
    }

    // 2.轻量级任务初始化就绪
    applite_post();

#ifdef USING_OS_FREERTOS
    // 3.普通任务预处理
    task_init();

    // 4.创建普通任务
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++) {
        if(likely((*tasks)->main)) {
            xTaskCreate((*tasks)->main, NULL, (*tasks)->depth <= 0?256:(*tasks)->depth,\
                NULL, (*tasks)->pro <= 0? TASK_DEFULT_PRO: (*tasks)->pro, NULL);
            stdio_printf("task %s running"STRBR, (*tasks)->name);
        }
    }

    // 5.普通任务就绪
    task_post();
#endif

    stdio_printf("system running !!!!!!!!!"STRBR);
    // 6.执行管理任务
    while(1) {
        // 7.驱动程序周期执行
        for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++) {
            if(unlikely((*drivers)->run)) ((*drivers)->run)();
        }

        // 8.驱动执行一轮回调函数
        arch_hooks1();

        frtos_delay_ms(APP_MGR_TASK_TIME);
        // 9.初始化轻量级任务应用程序
        for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++) {
            c_tick = frtos_get_tick();
            if(likely((*applites)->run)) ((*applites)->run)();
            (*applites)->used_tick = frtos_get_tick() - c_tick;
        }

        // 10.应用执行一轮回调函数
        arch_hooks2();
        frtos_delay_ms(APP_MGR_TASK_TIME);
    }
    (void)args;
}

/**************************************************************************************
* FunctionName   : os_start()
* Description    : 主程序入口
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
int os_start(void)
{
    struct driver **drivers = NULL;

    // 0. 初始化BSP相关
    arch_init();

    // 1.初始化所有內核驱动模块
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++) {
        if(likely((*drivers)->init)) ((*drivers)->init)();
        stdio_printf(STRBR"driver devid %d loaded", (*drivers)->idx);
    }
    stdio_printf(STRBR);

    // 2.BSP初始化结束
    arch_post();

#ifdef USING_OS_FREERTOS
    // 3.创建管理任务
    xTaskCreate(app_master, "master", APPLITE_DEPTH_MGR, NULL, 3, &mgrhdle);

    // 4.启动操作系统调度
    vTaskStartScheduler();
    while(1);
#else
    app_master(NULL);
#endif
}
