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
#include "spc_flexuart.h"
#include "pflash_driver.h"

/**************************************************************************************
* Description    : 模块内部全局数据定义
**************************************************************************************/

/**************************************************************************************
* Description    : 模块内部全局数据定义
**************************************************************************************/
/*
 * GBA: Flash functions initialisation
 *
 * */
extern const unsigned int FlashInit_C[];
extern const unsigned int FlashErase_C[];
extern const unsigned int BlankCheck_C[];
extern const unsigned int FlashProgram_C[];
extern const unsigned int ProgramVerify_C[];
extern const unsigned int CheckSum_C[];
extern const unsigned int GetLock_C[];
extern const unsigned int SetLock_C[];

/* Assign function pointers */
#define pFlashInit ((pFLASHINIT)FlashInit_C)
#define pFlashErase ((pFLASHERASE)FlashErase_C)
#define pBlankCheck ((pBLANKCHECK) BlankCheck_C)
#define pFlashProgram ((pFLASHPROGRAM)FlashProgram_C)
#define pProgramVerify ((pPROGRAMVERIFY)ProgramVerify_C)
#define pCheckSum ((pCHECKSUM)CheckSum_C)
#define pGetLock ((pGETLOCK)GetLock_C)
#define pSetLock ((pSETLOCK)SetLock_C)

/* CFlash */
static SSD_CONFIG ssdConfig = {
    C_REG_BASE,             /* Flash control register base */
    C_ARRAY_BASE,           /* base of main array */
    0xB7FFF,                /* size of main array */
    SHADOW_ROW_BASE,        /* base of shadow row */
    SHADOW_ROW_SIZE,        /* size of shadow row */
    0,                      /* block number in low address space */
    0,                      /* block number in middle address space */
    0,                      /* block number in high address space */
    FLASH_PAGE_SIZE,        /* flash page size selection */
    FALSE                   /* debug mode selection */
};

/**************************************************************************************
* FunctionName   : spc_rom_erase()
* Description    : FLASH擦除操作，！暂不支持指定地址擦除！！
* EntryParameter : addr，要擦除的地址， len， 要擦除的长度
* ReturnValue    : None
**************************************************************************************/
int8_t spc_rom_erase(uint32_t *addr, uint32_t len)
{
    uint8_t returnCode;
    BOOL   shadowFlag = FALSE;                /* H7F shadow select flag */
    // 只擦除IMAGE header区域
    UINT32 lowEnabledBlocks = 0X00000002;     /* selected blocks in low space */
    UINT32 midEnabledBlocks = 0X00000000;     /* selected blocks in middle space */
    UINT32 highEnabledBlocks = 0X00000000;    /* selected blocks in high space */

    // 1. 关闭全局中断
    local_irq_disable();

    // 2. 开始擦出,擦出全部CFLASH
    returnCode = pFlashErase( &ssdConfig, shadowFlag, lowEnabledBlocks,\
             midEnabledBlocks, highEnabledBlocks, (tpfNullCallback)NULL_CALLBACK );

    if ( C90FL_OK != returnCode )
    {
        local_irq_enable();
        return -EIO;
    }

    local_irq_enable();

    (void)len;
    (void)addr;
    return 0;
}

/**************************************************************************************
* FunctionName   : spc_rom_write()
* Description    : FLASH写操作
* EntryParameter : buffer要写入的数据， addr，写入的地址， len，写入的数据长度
* ReturnValue    : 返回写入长度或者错误码
**************************************************************************************/
int32_t spc_rom_write(uint8_t *buffer, uint32_t *addr, uint32_t len)
{
    uint8_t returnCode;
    UINT32 dest;                 /* destination address */
    UINT32 size;                 /* size applicable */
    UINT32 source;               /* source address for program and verify */
    UINT32 failAddress;          /* save the failed address in flash */
    UINT64 failData;             /* save the failed data in flash */
    UINT64 failSource;           /* save the failed data in source buffer */

    dest = ssdConfig.mainArrayBase + (UINT32)addr;
    source = (UINT32)buffer;
    size = len;

    stdio_printf("dest:%x , addr: %04x,buffer:%08x\r\n", dest, source, *(UINT32 *)source);

    // 1. 判断要编写的地址是否合法
    if(dest < ssdConfig.mainArrayBase || dest > ssdConfig.mainArrayBase        \
            + ssdConfig.mainArraySize){
        return -EINVAL;
    }

    // 2. 在写flash的时候关闭全局终端
    local_irq_disable();

    /* program main array */
    returnCode = pFlashProgram( &ssdConfig, dest, size, source, (tpfNullCallback)NULL_CALLBACK );
    if ( C90FL_OK != returnCode )
    {
        local_irq_enable();
        return -EIO;
    }

    /* Verify the programmed data */
    returnCode = pProgramVerify( &ssdConfig, dest, size, source, &failAddress, &failData, &failSource, (tpfNullCallback)NULL_CALLBACK);
    if ( C90FL_OK != returnCode ) //当返回值是C90FL_ERROR_VERIFY的时候可以得到fail的信息
    {
        local_irq_enable();
        return -EIO;
    }

    local_irq_enable();

    return len;
}

/**************************************************************************************
* FunctionName   : spc_rom_read()
* Description    : FLASH读操作
* EntryParameter : buffer缓冲区， addr，地址， len，数据长度
* ReturnValue    : 返回写入长度或者错误码
**************************************************************************************/
int32_t spc_rom_read(uint8_t *buffer, uint8_t *addr, uint32_t len)
{
    memcpy(buffer, addr, len);

    return len;
}

/**************************************************************************************
 * FunctionName   : pflash_ioctrl()
 * Description    : 控制
 * EntryParameter : *args,参数, len,参数长度
 * ReturnValue    : 返回错误码
 **************************************************************************************/
static int32_t pflash_ioctrl(uint8_t idx, int32_t cmd, void *args, int32_t len)
{
    int32_t ret = 0;
    struct pflash *pdata = args;

    if (unlikely(NULL == args || len != sizeof(struct pflash))) {
        return -EINVAL;
    }

    switch (cmd) {
    case _IOC_E:
        ret = spc_rom_erase((uint32_t *)pdata->addr, pdata->length);
        break;
    case _IOC_R:
        ret = spc_rom_read(pdata->data, (uint8_t *)pdata->addr, pdata->length);
        break;
    case _IOC_W:
        ret = spc_rom_write(pdata->data, (uint32_t *)pdata->addr, pdata->length);
        break;
    }

    (void) idx;
    return ret;
}

/**************************************************************************************
 * FunctionName   : pflash_init()
 * Description    : Pflash初始化
 * EntryParameter : None
 * ReturnValue    : None
 **************************************************************************************/
static int32_t __init pflash_init(void)
{
    uint8_t returnCode; /* Return code from each SSD function. */
    // 1. 需要注册中断处理函数和使能驱动吗？

    returnCode = pFlashInit( &ssdConfig );
    if (returnCode != C90FL_OK) {
        return -EIO;
    }

    return 0;
}

static __const struct driver spc_pflash = {
    .idx   = DRIVER_PFLASH,
    .name  = "pflash",
    .init  = pflash_init,
    .ioctl = pflash_ioctrl,
};

/**************************************************************************************
 * Description    : 模块初始化
 **************************************************************************************/
EARLY_INIT(spc_pflash);
