/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_shell.c
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
#include "frtos_log.h"

/**************************************************************************************
* Description    : 模块定义命名缓存区
**************************************************************************************/
static int32_t shell_pos = 0;
static uint8_t shell_command[CONSOLE_MAX];

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
* FunctionName   : shell_find_char()
* Description    : shell查找字符所在偏移
* EntryParameter : data，shell命令数据，chr, 指定字符， len,shell数据长度
* ReturnValue    : 返回换行字符所在偏移,没有返回-1
**************************************************************************************/
static inline int32_t shell_find_char(uint8_t *data, uint8_t chr, int32_t len)
{
    int32_t idx = 0;

    for (idx = 0; idx < len; idx++) {
        if(unlikely(data[idx] == chr)) return idx;
    }

    return -ENOCRLR;
}

/**************************************************************************************
* FunctionName   : shell_find_nonchar()
* Description    : shell查找回第一个非指定字符的位置
* EntryParameter : data，shell命令数据， chr, 指定字符， len,shell数据长度
* ReturnValue    : 返回换行字符所在偏移,没有返回-1
**************************************************************************************/
static inline int32_t shell_find_nonchar(char *data, uint8_t chr, int32_t len)
{
    int32_t idx = 0;

    for (idx = 0; idx < len; idx++) {
        if(unlikely(data[idx] != chr)) return idx;
    }

    return -ENOCHAR;
}

/**************************************************************************************
* FunctionName   : push_to_cbuf()
* Description    : 将字符插入到shell 缓存
* EntryParameter : data，shell命令数据， len,shell数据长度
* ReturnValue    : 返回None
**************************************************************************************/
static inline void push_to_cbuf(uint8_t *data, int32_t len)
{
    int32_t copy = shell_pos+len > CONSOLE_MAX?CONSOLE_MAX-shell_pos:len;

    if(likely(len > 0)) {
        memcpy(shell_command + shell_pos, data, copy);
        shell_pos  += copy;
    }
}

/**************************************************************************************
* FunctionName   : backspace_cbuf()
* Description    : 回退删除指令
* EntryParameter : data，shell命令数据， len,shell数据长度
* ReturnValue    : 返回None
**************************************************************************************/
static void backspace_cbuf(char chr)
{
    int32_t del_pos = 0;

    // 1.查找del键，删除对应字符
    while((del_pos = shell_find_char(shell_command, chr, shell_pos)) >= 0) {
        del_pos = shell_find_char(shell_command, chr, shell_pos);

        memcpy(shell_command+del_pos-1, shell_command+del_pos+1, shell_pos-del_pos-1);
        shell_pos = shell_pos - 2 > 0?shell_pos - 2:0;
    }
}

/**************************************************************************************
* FunctionName   : shell_cmd_parse()
* Description    : shell命令解析
* EntryParameter : shell，shell命令数据， pos,shell数据长度
* ReturnValue    : 返回None
**************************************************************************************/
static void shell_cmd_parse(char *shell, int32_t pos)
{
    int32_t ofs = 0;
    int8_t cmd_len = 0;
    uint8_t execute = 0;
    char *shell_name = NULL;
    struct shell **slist = NULL;
    struct applite **applites = NULL;
#ifdef USING_OS_FREERTOS
    struct task **tasks = NULL;
#endif
    shell_cmd_t shell_debug = NULL;

    // 1.换行
    stdio_printf(STRBR);

    // 2.查找所有注册shell指令
    for (slist = __SHELL_LIST_CHAIN_S__; slist < __SHELL_LIST_CHAIN_E__; slist++){
        cmd_len = strlen((*slist)->name);

        if(unlikely((shell[cmd_len] == ' ' || cmd_len == pos ) &&
                !memcmp(shell, (*slist)->name, cmd_len))) {
            shell_debug = (*slist)->cmd;
            shell_name = (char *)(*slist)->name;
            execute = 1;
            goto finish;
        }
    }

    // 3.查找所有注册applite指令
    for (applites = __APP_LIST_CHAIN_S__; applites < __APP_LIST_CHAIN_E__; applites++) {
        cmd_len = strlen((*applites)->name);

        if(unlikely((shell[cmd_len] == ' ' || cmd_len == pos ) &&
                !memcmp(shell, (*applites)->name, cmd_len))) {
            shell_debug = (*applites)->debug;
            shell_name = (char *)(*applites)->name;
            execute = 1;
            goto finish;
        }
    }

#ifdef USING_OS_FREERTOS
    // 4.查找所有注册task指令
    for(tasks = __TASK_LIST_CHAIN_S__; tasks < __TASK_LIST_CHAIN_E__; tasks++) {
        cmd_len = strlen((*tasks)->name);

        if(unlikely((shell[cmd_len] == ' ' || cmd_len == pos ) &&
                !memcmp(shell, (*tasks)->name, cmd_len))) {
            shell_debug = (*tasks)->debug;
            shell_name = (char *)(*tasks)->name;
            execute = 1;
            goto finish;
        }
    }
#endif

finish:
    // 5.没有找到执行指令, pos=0为只有换行符
    if(!execute && pos != 0) {
        shell[shell_find_char((uint8_t*)shell, KEY_SPACE, pos)] = '\0';
        stdio_printf("-ash: %s:command not found..."STRBR,shell);
    } else if(pos != 0) {
        if(likely(shell_debug != NULL)) {
            ofs = shell_find_nonchar(shell + cmd_len, KEY_SPACE, pos - cmd_len);
            if(ofs > 0) {
                shell_debug(shell + cmd_len + ofs, pos - cmd_len - ofs);
            } else {
                shell_debug(NULL, 0);
            }
        } else {
            stdio_printf("bash: %s: command not implements!"STRBR, shell_name != NULL?shell_name:"nil");
        }
    }
}

/**************************************************************************************
* FunctionName   : shell_recv()
* Description    : shell数据串
* EntryParameter : data，shell命令数据， len,shell数据长度
* ReturnValue    : 返回处理的数据长度或者错误码
**************************************************************************************/
int32_t shell_recv(int32_t idx, void *data, int32_t len)
{
    int32_t enter_pos = 0;
    uint8_t *chars = (uint8_t *)data;

    // 0.查找特殊功能键，上下左右等(丢弃)
    if(len >= 3 && chars[0] == KEY_MUX_L && chars[1] == KEY_MUX_H) {
        if(len == 3 && chars[2] >= KEY_UP && chars[2] <= KEY_LEFT) {
            return 0;
        }
        if(len == 4 && chars[2] >= KEY_HOME &&
                chars[2] <= KEY_PAGEDOWN && chars[3] == KEY_CTRL_E) {
            return 0;
        }
    }
    // 1.查找接收串中是否包含字符换行
    enter_pos = shell_find_char(data, KEY_ENTER, len);

    // 2.将没有换行回车的数据插入到命令字符
    push_to_cbuf(data, enter_pos >= 0?enter_pos:len);

    // 3.如果存在DEL键， 则回退字符
    if(shell_find_char(data, KEY_DEL1, len) >= 0) {
        backspace_cbuf(KEY_DEL1);
    }
    if(shell_find_char(data, KEY_DEL2, len) >= 0) {
        backspace_cbuf(KEY_DEL2);
    }

    // 4. ctrl+c，则结束正文
    if(shell_find_char(data, KEY_CTRL_C, len) >= 0) {
        shell_pos = 0;
        stdio_printf(STRBR);
    }
    // 5.如果存在换行， 则是一条完整的指令， 执行它
    shell_command[shell_pos] = '\0';
    if(unlikely(enter_pos != -ENOCRLR)) {
        shell_cmd_parse((char *)shell_command, shell_pos);
        shell_pos = 0;
    }

    // 6.刷新当前行信息
    shell_command[shell_pos] = '\0';
    stdio_printf(KEY_LR CONSOLE_ECHO "%s",shell_command);

    (void)idx;
    return 0;
}
