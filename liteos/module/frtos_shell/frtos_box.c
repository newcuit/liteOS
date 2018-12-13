/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_box.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/

#include "frtos_shell.h"

#include "frtos_mem.h"
#include "frtos_list.h"
#include "frtos_errno.h"
#include "frtos_delay.h"
#include "frtos_app.h"
#include "frtos_drivers.h"
#include "frtos_time.h"
#include "frtos_log.h"

/**************************************************************************************
* Description    : shell列表安装地址
**************************************************************************************/
extern struct shell * __SHELL_LIST_CHAIN_S__[];
extern struct shell * __SHELL_LIST_CHAIN_E__[];

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
#endif

/**************************************************************************************
* Description    : 驱动模块安装地址
**************************************************************************************/
extern struct driver *__DRIVER_LIST0_S__[];
extern struct driver *__DRIVER_LIST7_E__[];

/**************************************************************************************
* TypeName       : ls_dev()
* Description    : 查询dev目录下文件
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t ls_dev(char *argv, int32_t argc)
{
    struct driver **drivers = NULL;

    // 1.查找所有注册驱动列表
    for (drivers = __DRIVER_LIST0_S__; drivers < __DRIVER_LIST7_E__; drivers++) {
        if((*drivers)->name) stdio_printf("\033[35m/dev/%s\033[0m\t"STRBR, (*drivers)->name);
    }

    (void)argv;
    return argc;
}

/**************************************************************************************
* TypeName       : ls_bin()
* Description    : 查询bin目录下文件
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t ls_bin(char *argv, int32_t argc)
{
    struct shell **slist = NULL;
    struct applite **applites = NULL;
#ifdef USING_OS_FREERTOS
    struct task **tasks = NULL;
#endif

    // 1.查找所有注册shell指令
    for (slist = __SHELL_LIST_CHAIN_S__; slist < __SHELL_LIST_CHAIN_E__; slist++){
        if((*slist)->name)  stdio_printf("\033[32m/bin/%s\033[0m\t"STRBR, (*slist)->name);
    }

    // 2.查找所有注册applites指令
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++) {
        if((*applites)->name) stdio_printf("\033[32m/bin/%s\033[0m\t"STRBR, (*applites)->name);
    }

#ifdef USING_OS_FREERTOS
    // 3.查找所有注册task指令
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++) {
        if((*tasks)->name) stdio_printf("\033[32m/bin/%s\033[0m\t"STRBR, (*tasks)->name);
    }
#endif

    (void)argv;
    return argc;
}

/**************************************************************************************
* TypeName       : shell_ls()
* Description    : ls指令，显示目录下内容
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_ls(char *argv, int32_t argc)
{
    uint8_t i = 0;
    static __const struct shell_args args_table[] = {
        {
            .name = "/bin",
            .cmd  = ls_bin,
        },{
            .name = "/dev",
            .cmd  = ls_dev,
        },
    };

    // 参数检测
    if(argv == NULL || argc == 0) {
        for(i = 0; i < ARRAY_LEN(args_table); i++){
            stdio_printf("\033[34m%s\033[0m\t",args_table[i].name);
        }
        stdio_printf(STRBR);
    } else {
        for(i = 0; i < ARRAY_LEN(args_table); i++){
            // 参数检测
            if((uint32_t)argc != strlen(args_table[i].name) ||
                    0 != strcmp(argv, args_table[i].name)) {
                continue;
            }

            // 执行具体指令
            if(NULL != args_table[i].cmd) args_table[i].cmd(argv, argc);
            else stdio_printf("Parameters do not support."STRBR);
        }
    }
    return argc;
}

/**************************************************************************************
* Description    : 定义ls运行指令
**************************************************************************************/
static __const struct shell c_ls = {
    .name = "ls",
    .cmd  = shell_ls,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_ls);

/**************************************************************************************
* TypeName       : shell_pwd()
* Description    : pwd指令，显示当前目录
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_pwd(char *argv, int32_t argc)
{
    stdio_printf("/"STRBR);

    (void)argv;
    return argc;
}

/**************************************************************************************
* Description    : 定义pwd运行指令
**************************************************************************************/
static __const struct shell c_pwd = {
    .name = "pwd",
    .cmd  = shell_pwd,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_pwd);

/**************************************************************************************
* TypeName       : shell_cd()
* Description    : cd指令，进入指定目录
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_cd(char *argv, int32_t argc)
{
    (void)argv;
    return argc;
}

/**************************************************************************************
* Description    : 定义cd运行指令
**************************************************************************************/
static __const struct shell c_cd = {
    .name = "cd",
    .cmd  = shell_cd,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_cd);

/**************************************************************************************
* TypeName       : shell_mount()
* Description    : mount指令，查看挂载情况
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_mount(char *argv, int32_t argc)
{
    (void)argv;
    return argc;
}

/**************************************************************************************
* Description    : 定义mount运行指令
**************************************************************************************/
static __const struct shell c_mount = {
    .name = "mount",
    .cmd  = shell_mount,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_mount);

/**************************************************************************************
* TypeName       : shell_cat()
* Description    : cat指令，查看文件内容
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_cat(char *argv, int32_t argc)
{
    (void)argv;
    return argc;
}

/**************************************************************************************
* Description    : 定义cat运行指令
**************************************************************************************/
static __const struct shell c_cat = {
    .name = "cat",
    .cmd  = shell_cat,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_cat);

/**************************************************************************************
* TypeName       : shell_echo()
* Description    : echo指令，打印文字
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_echo(char *argv, int32_t argc)
{
    if(argc > 0 && argv != NULL) {
        stdio_output((uint8_t *)argv, argc);
    }
    stdio_printf(STRBR);

    return argc;
}

/**************************************************************************************
* Description    : 定义cat运行指令
**************************************************************************************/
static __const struct shell c_echo = {
    .name = "echo",
    .cmd  = shell_echo,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_echo);

/**************************************************************************************
* TypeName       : shell_uptime()
* Description    : uptime指令，获取运行时长
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_uptime(char *argv, int32_t argc)
{
    time_sys_t seconds = time_gettick()/configTICK_RATE_HZ;

    // 1.进行时间转换并输出
    stdio_printf("uptime: %d days, %d hours, %d min, %d seconds"STRBR,
            seconds/60/60/24,seconds/60/60%24, seconds/60%60, seconds%60);

    (void)argv;
    return argc;
}

/**************************************************************************************
* Description    : 定义uptime运行指令
**************************************************************************************/
static __const struct shell c_uptime = {
    .name = "uptime",
    .cmd  = shell_uptime,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_uptime);

/**************************************************************************************
* TypeName       : shell_clear()
* Description    : clear指令，清屏
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_clear(char *argv, int32_t argc)
{
    // 1.清屏
    stdio_printf("\033[2J \33[0;0H");

    (void)argv;
    return argc;
}

/**************************************************************************************
* Description    : 定义clear运行指令
**************************************************************************************/
static __const struct shell c_clear = {
    .name = "clear",
    .cmd  = shell_clear,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_clear);

/**************************************************************************************
* TypeName       : shell_ps()
* Description    : ps指令，查看当前应用空间
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_ps(char *argv, int32_t argc)
{
    struct applite **applites = NULL;
#ifdef USING_OS_FREERTOS
    struct task **tasks = NULL;

    stdio_printf(" cpu freq\t preempt\t tick\t timers\t minstack"STRBR);
    // 1.显示CPU信息
    stdio_printf(" %d\t    %d\t\t %d\t  %d\t  %d"STRBR,configCPU_CLOCK_HZ,configUSE_PREEMPTION,
            configTICK_RATE_HZ,configTIMER_QUEUE_LENGTH,configMINIMAL_STACK_SIZE);

    stdio_printf(" memory\t  used\t  idle\t  min"STRBR);
    // 2.显示内存信息
    stdio_printf("  %d\t %d\t %d\t %d"STRBR,configTOTAL_HEAP_SIZE,
            configTOTAL_HEAP_SIZE-xPortGetFreeHeapSize(), xPortGetFreeHeapSize(),
            xPortGetMinimumEverFreeHeapSize());
#endif

    stdio_printf(STRBR " pid\t attr\t name\t ticks \tmem \tstat"STRBR);
    // 3.查找所有注册applites指令
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++) {
        if((*applites)->name) {
            stdio_printf("  %d\t  W\t %s\t  %d\t N\trunning"STRBR, (*applites)->idx,
                    (*applites)->name, (*applites)->used_tick);
        }
    }

#ifdef USING_OS_FREERTOS
    // 4.查找所有注册task指令
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++) {
        if((*tasks)->name) {
            stdio_printf("  %d\t  T\t %s\t  N\t N\trunning"STRBR, (*tasks)->idx, (*tasks)->name);
        }
    }
#endif

    (void)argv;
    return argc;
}

/**************************************************************************************
* Description    : 定义ps运行指令
**************************************************************************************/
static __const struct shell c_ps = {
    .name = "ps",
    .cmd  = shell_ps,
};

/**************************************************************************************
* Description    : 指令初始化
**************************************************************************************/
SHELL_REGISTER(c_ps);
