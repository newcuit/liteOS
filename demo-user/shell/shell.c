/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : shell.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_shell.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "gpio_driver.h"
#include "frtos_sys.h"
#include "data.pb-c.h"
#include "minmea.h"
#include "frtos_log.h"

/**************************************************************************************
* Description    : 定义shell使用的串口
**************************************************************************************/
#define SHELL_UART                           DRIVER_UART0

/**************************************************************************************
* FunctionName   : shell_init()
* Description    : Shell初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t shell_init(void)
{
    return fdrive_ioctl(SHELL_UART, _IOC_SET_CB, shell_recv, sizeof(shell_recv));
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite shell = {
    .name  = "bash",
    .init  = shell_init,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(shell);
