/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : pflash_driver.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
 **************************************************************************************/
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "frtos_lock.h"
#include "frtos_ioctl.h"
#include "frtos_irq.h"
#include "config_driver.h"
#include "dflash_driver.h"

/**************************************************************************************
* Description    : 模块内部全局数据定义
**************************************************************************************/
static flash_ssd_config_t                           rom_config;

/**************************************************************************************
* Description    : 模块内部全局数据定义
**************************************************************************************/
static const flash_user_config_t rom_usr_config = {
    .PFlashBase  = 0x00000000U,
    .PFlashSize  = 0x00080000U,
    .DFlashBase  = DFLASH_BASE,
    .EERAMBase   = DFALSH_RAMBASE,
    .CallBack    = NULL_CALLBACK
};

/**************************************************************************************
* FunctionName   : s32_dflash_handler()
* Description    : FLASH中断服务器程序
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
START_FUNCTION_DECLARATION_RAMSECTION
static void s32_dflash_handler(void)
END_FUNCTION_DECLARATION_RAMSECTION
static void s32_dflash_handler(void)
{
    FTFx_FCNFG &= (~FTFx_FCNFG_CCIE_MASK);
}

/**************************************************************************************
* FunctionName   : e2_write()
* Description    : 拷贝数据到非易失存储
* EntryParameter : *dst,目标地址, *src,原始地址, len,拷贝字节数
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t e2_write(void *dst, const void *src, uint32_t len)
{
    // 1.检查参数
    if(NULL == dst || NULL == src || 0 == len || \
        (uint32_t)dst < rom_config.EERAMBase || \
        (uint32_t)dst > (rom_config.EERAMBase + rom_config.EEESize)){
        return -EINVAL;
    }

    // 2.进入临界区
    local_irq_disable();

    // 3.写数据到EEPROM
    if(STATUS_SUCCESS != FLASH_DRV_EEEWrite(&rom_config, \
        (uint32_t)dst, len, src)){
        local_irq_enable();
        return -EFAULT;
    }

    // 4.退出临界区
    local_irq_enable();

    return len;
}

/**************************************************************************************
* FunctionName   : e2_read()
* Description    : 非易失存储e2_read
* EntryParameter : *dst,目标地址, *src,原始地址， len，长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t e2_read(void *dst, void* src, uint32_t len)
{
    uint32_t i = 0;

    // 1.检查参数
    if(NULL == dst || NULL == src || 0 == len || \
        (uint32_t)src < rom_config.EERAMBase || \
        ((uint32_t)src + len) > (rom_config.EERAMBase + rom_config.EEESize)){
        return -EINVAL;
    }
    // 2.进入临界区
    local_irq_disable();

    // 3.从EEPROM读取数据
    for(i = 0; i < len; i++) *((uint8_t *)dst + i) = *((uint8_t *)src + i);

    // 3.退出临界区
	local_irq_enable();

    return len;
}


/**************************************************************************************
 * FunctionName   : dflash_ioctrl()
 * Description    : 控制
 * EntryParameter : *args,参数, len,参数长度
 * ReturnValue    : 返回错误码
 **************************************************************************************/
static int32_t dflash_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    int32_t ret = 0;
    struct dflash *ddata = (struct dflash *)args;

    if (unlikely(NULL == args || len != sizeof(struct dflash))) {
        return -EINVAL;
    }

    switch (cmd) {
    case _IOC_W:
        ret = e2_write((void *)(DFALSH_RAMBASE+ddata->addr), (void *)ddata->data, ddata->length);
        break;
    case _IOC_R:
        ret = e2_read((void *)ddata->data, (void *)(DFALSH_RAMBASE+ddata->addr), ddata->length);
        break;
    }

    (void) idx;
    return ret;
}

/**************************************************************************************
 * FunctionName   : dflash_init()
 * Description    : dflash初始化
 * EntryParameter : None
 * ReturnValue    : None
 **************************************************************************************/
static int32_t __init dflash_init(void)
{
    int32_t err = 0;

    // 1.关闭缓存
#ifdef S32K144_SERIES
    MSCM->OCMDR[0u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu)
                | MSCM_OCMDR_OCM2(0xFu);
    MSCM->OCMDR[1u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu)
                | MSCM_OCMDR_OCM2(0xFu);
    MSCM->OCMDR[2u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu)
                | MSCM_OCMDR_OCM2(0xFu);
    MSCM->OCMDR[3u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu)
                | MSCM_OCMDR_OCM2(0xFu);
#endif
    // 2.安装FLASH中断服务程序
    INT_SYS_InstallHandler(FTFC_IRQn, s32_dflash_handler, NULL);
    INT_SYS_SetPriority(FTFC_IRQn, 3);

    // 3.使能FLASH中断
    INT_SYS_EnableIRQ(FTFC_IRQn);

    // 4.使能全局中断
    local_irq_enable();

    // 5.初始化物理设备
    if (STATUS_SUCCESS != FLASH_DRV_Init(&rom_usr_config, &rom_config)) {
        err = -EIO;goto RETURN_2;
    }
    // 6.如果EEPROM已启用
    if(0 != rom_config.EEESize) goto RETURN_1;

    // 7.配置EEPROM
    // (EEPROM size = 4 Kbytes, EEPROM backup size = 64 Kbytes, Keys is 3 to 24 keys)
    if(STATUS_SUCCESS != FLASH_DRV_DEFlashPartition( \
        &rom_config, 0x02u, 0x08u, 0x00u, false)){
        err = -EFAULT;goto RETURN_2;
    }

    // 8.重新初始化驱动程序以更新新的EEPROM配置
    if(STATUS_SUCCESS != FLASH_DRV_Init(&rom_usr_config, &rom_config)){
        err = -EFAULT;goto RETURN_2;
    }

    RETURN_1:

    // 9.使FlexRAM可用于EEPROM
    if(STATUS_SUCCESS != FLASH_DRV_SetFlexRamFunction( \
        &rom_config, EEE_ENABLE, 0, NULL)){
        err = -EFAULT;goto RETURN_2;
    }

    RETURN_2:

    return err;
}

static __const struct driver s32k_dflash = {
    .name  = "dflash",
    .idx   = DRIVER_DFLASH,
    .init  = dflash_init,
    .ioctl = dflash_ioctrl,
};

/**************************************************************************************
 * Description    : 模块初始化
 **************************************************************************************/
CORE_INIT(s32k_dflash);
