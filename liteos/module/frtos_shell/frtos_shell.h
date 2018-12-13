/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_shell.h
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#ifndef __FRTOS_SHELL_H__
#define __FRTOS_SHELL_H__

#include "string.h"
#include "frtos_types.h"
#include "frtos_errno.h"
#include "frtos_mem.h"

/**************************************************************************************
* Description    : 模块定义串口回显头信息
**************************************************************************************/
#define CONSOLE_ECHO                               "[root@ifoton:/]$ " // 定义回显
#define CONSOLE_MAX                                 200               // 定义最大命令长度
#define KEY_DEL1                                    0x7F              // 删除键
#define KEY_DEL2                                    0x08              // 删除键
#define KEY_CTRL_C                                  0x03              // CTRL+C
#define KEY_SPACE                                   0x20              // 空格键
#define KEY_ENTER                                   0x0D              // 回车键
#define KEY_LR                                      "\r"              // 回到行首
#define KEY_MUX_L                                   0x1B              //
#define KEY_MUX_H                                   0x5B              //
#define KEY_UP                                      0x41              // 上
#define KEY_DOWN                                    0x42              // 下
#define KEY_RIGHT                                   0x43              // 右
#define KEY_LEFT                                    0x44              // 左
#define KEY_HOME                                    0x31              // home键
#define KEY_INSERT                                  0x32              // insert键
#define KEY_DEL                                     0x33              // del键
#define KEY_END                                     0x34              // end键
#define KEY_PAGEUP                                  0x35              // page up键
#define KEY_PAGEDOWN                                0x36              // page down键
#define KEY_CTRL_E                                  0x7E              // 功能键结束

/**************************************************************************************
* Description    : 模块定义串口错误码
**************************************************************************************/
#define ENOCRLR                                     1                // 找不到错误码
#define ENOCHAR                                     2                // 无字符错误码

/**************************************************************************************
* TypeName       : shell_cmd_t()
* Description    : shell指令入口
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
typedef int32_t (*shell_cmd_t)(char *, int32_t);

/**************************************************************************************
* Description    : shell指令结构定义
**************************************************************************************/
struct shell {
    __const char *name;                         // shell名称
    __const shell_cmd_t cmd;                    // shell指令函数
};

/**************************************************************************************
* Description    : 命令参数结构定义
**************************************************************************************/
struct shell_args {
    char *name;                         // 参数名称
    shell_cmd_t cmd;                    // 参数执行函数
};

/**************************************************************************************
* TypeName       : SHELL_REGISTER()
* Description    : shell注册函数
* EntryParameter : shell指令结构体
* ReturnValue    : None
**************************************************************************************/
#define SHELL_REGISTER(cmd) \
    static __const struct shell* __attribute__((used,section(".shell_list_chain"))) \
    __list_##cmd##__ = &cmd

/**************************************************************************************
* FunctionName   : shell_recv()
* Description    : shell数据串
* EntryParameter : data，shell命令数据， len,shell数据长度
* ReturnValue    : 返回处理的数据长度或者错误码
**************************************************************************************/
int32_t shell_recv(int32_t idx, void *data, int32_t len);

#endif /* __FRTOS_SHELL_H__ */
