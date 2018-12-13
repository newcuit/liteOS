/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : reboot_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_drivers.h"
#include "clock.h"
#include "gpio_driver.h"
#include "frtos_time.h"

/**************************************************************************************
* Description    : 模块内部数据定义
**************************************************************************************/
#define SPC5_RUNMODE_RESET          0U

/**************************************************************************************
* MacroName      : arch_reboot
* Description    : 系统重启
* EntryParameter : magic,幻术
* ReturnValue    : 返回错误码
**************************************************************************************/
int32_t arch_reboot(uint32_t magic)
{
    gpio_pin_set(PORT_A, PTA14, 0); //PA14 LOW
    time_delayms(2000);
    SPCSetRunMode(SPC5_RUNMODE_RESET);
    (void)magic;
    return 0;
}
