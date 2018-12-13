#include "frtos_types.h"
#include "frtos_errno.h"
#include "frtos_drivers.h"
#include "config_driver.h"
#include "frtos_lock.h"
#include "tp701.h"

/**************************************************************************************
* Description    : 临界区定义
**************************************************************************************/
#define TP_ERITICAL_ENTER()       taskENTER_CRITICAL()
#define TP_ERITICAL_EXIT()        taskEXIT_CRITICAL()

/**************************************************************************************
* FunctionName   : tp701_motor_move()
* Description    : 步进电机移动位置
* EntryParameter : times，电机移动次数
* ReturnValue    : None
**************************************************************************************/
static void tp701_motor_move(uint8_t times)
{
    static int8_t motor_pos = 0;

    for (;times > 0;times--){
        switch(motor_pos) {
        case 0:
            gpio_pin_set(TP701_MTAIN, 0);
            gpio_pin_set(TP701_MTA_IN, 1);
            gpio_pin_set(TP701_MTBIN, 0);
            gpio_pin_set(TP701_MTB_IN, 1);
            break;
        case 1:
            gpio_pin_set(TP701_MTAIN, 0);
            gpio_pin_set(TP701_MTA_IN, 1);
            gpio_pin_set(TP701_MTBIN, 1);
            gpio_pin_set(TP701_MTB_IN, 0);
            break;
        case 2:
            gpio_pin_set(TP701_MTAIN, 1);
            gpio_pin_set(TP701_MTA_IN, 0);
            gpio_pin_set(TP701_MTBIN, 1);
            gpio_pin_set(TP701_MTB_IN, 0);
            break;
        case 3:
            gpio_pin_set(TP701_MTAIN, 1);
            gpio_pin_set(TP701_MTA_IN, 0);
            gpio_pin_set(TP701_MTBIN, 0);
            gpio_pin_set(TP701_MTB_IN, 1);
            break;
        }
        motor_pos = (motor_pos+1)%4;
        motor_relax();
    }
}
/**************************************************************************************
* FunctionName   : tp701_clear_stb()
* Description    : stb控制线清零
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void tp701_clear_stb(void)
{
    gpio_pin_set(TP701_STB12, 0);
    gpio_pin_set(TP701_STB34, 0);
    gpio_pin_set(TP701_STB56, 0);
}

/**************************************************************************************
* FunctionName   : tp701_enable()
* Description    : 使能tp701设备
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void tp701_enable(void)
{
    gpio_pin_set(TP701_HC373, 0);
    gpio_pin_set(TP701_PRT_WE1, 1);
    tp701_clear_stb();
}

/**************************************************************************************
* FunctionName   : tp701_disable()
* Description    : 禁用tp701设备
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void tp701_disable(void)
{
    gpio_pin_set(TP701_HC373, 1);
    tp701_clear_stb();
    gpio_pin_set(TP701_PRT_WE1, 0);

}

/**************************************************************************************
* FunctionName   : tp701_write_dot()
* Description    : 向tp701写入一行点阵数据
* EntryParameter : dot,需要打印的数据，len，需要打印的数据长度
* ReturnValue    : None
**************************************************************************************/
static void tp701_write_dot(uint8_t *dot,uint16_t len)
{
    int8_t shift;
    uint8_t i = 0;

    for (i = 0; i < len; i++) {
        shift = 7;
        do {
            gpio_pin_set(TP701_PDATA, (dot[i]>>shift) & 0x1);
            gpio_pin_set(TP701_PCLK, 0);
            gpio_pin_set(TP701_PCLK, 1);
            shift--;
        } while(shift >= 0);
    }

    gpio_pin_set(TP701_PLAT, 0);
    gpio_pin_set(TP701_PLAT, 1);

}

/**************************************************************************************
* FunctionName   : tp701_trans_data()
* Description    : 将写入的点阵数据传输到机头打印
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static void tp701_trans_data(void)
{
    /* 三次分开避免电流过大 */
    gpio_pin_set(TP701_STB12, 1);
    trans_relax();
    gpio_pin_set(TP701_STB12, 0);

    gpio_pin_set(TP701_STB34, 1);
    trans_relax();
    gpio_pin_set(TP701_STB34, 0);

    gpio_pin_set(TP701_STB56, 1);
    trans_relax();
    gpio_pin_set(TP701_STB56, 0);
}

/**************************************************************************************
* FunctionName   : tp701_write_line()
* Description    : 打印一行点阵
* EntryParameter : dot,需要打印的数据一行指针，len,需要打印的当前行的长度
* ReturnValue    : None
**************************************************************************************/
static void tp701_write_line(uint8_t *dot, uint16_t len)
{
    tp701_enable();
    tp701_write_dot(dot,len);
    tp701_trans_data();
    tp701_motor_move(2);
    tp701_disable();
}

/**************************************************************************************
* FunctionName   : tp701_write()
* Description    : 打印数据
* EntryParameter : ubuf,需要打印的数据指针，size，需要打印的数据长度
* ReturnValue    : None
**************************************************************************************/
static int32_t tp701_write(uint8_t idx, void *ubuf, int32_t size)
{
    int row;
    uint8_t *dot = (uint8_t *)ubuf;

    if(unlikely(size != TP701_MSG_SIZE)) return -EMSGSIZE;
    if(unlikely(ubuf == NULL)) return -EMEM;
    TP_ERITICAL_ENTER();
    for (row = 0; row < TP701_ROW; row++) {
        tp701_write_line(dot + row * TP701_COL, TP701_COL);
    }
    TP_ERITICAL_EXIT();
    (void)idx;
	return size;
}

/**************************************************************************************
* FunctionName   : tp701_init()
* Description    : 设备初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
static int32_t __init tp701_init(void)
{
    // 1,禁用tp701
    tp701_disable();

    // 2，清除tp701数据线
    gpio_pin_set(TP701_PDATA, 0);
    gpio_pin_set(TP701_PCLK, 0);
    gpio_pin_set(TP701_MTA_IN, 0);
    gpio_pin_set(TP701_MTAIN, 0);
    gpio_pin_set(TP701_MTB_IN, 0);
    gpio_pin_set(TP701_MTBIN, 0);
    gpio_pin_set(TP701_PLAT, 0);

    return 0;
}

static __const struct driver s32k_tp701 = {
    .idx   = DRIVER_PRINT,
    .name  = "print",
    .init  = tp701_init,
    .write = tp701_write,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
MODULE_INIT(s32k_tp701);
