/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : ic.c
 * Author        :
 * Date          : 2017-03-15
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "gpio_driver.h"
#include "i2c_driver.h"
#include "frtos_gpio.h"
#include "frtos_sys.h"
#include "frtos_tasklet.h"
#include "frtos_irq.h"
#include "data.pb-c.h"
#include "frtos_delay.h"

/**************************************************************************************
* Description    : 定义IC数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                                    300
#define MAX_SUBPROTO_LEN                                250
#define IC_DATA_LEN                                     210

/**************************************************************************************
* Description    : IC卡电源引脚
**************************************************************************************/
#define IC_CARD_POWER                                   110 // RFIC_RST PTD14
#define IC_ADDR                                         0x50   // IC卡地址
#define INSERT_GPIO                                     136    // IC_DET PTE8

/**************************************************************************************
* FunctionName   : ic_power()
* Description    : IC卡电源控制
* EntryParameter : value, 电源控制电平
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t ic_power(uint32_t value)
{
    struct gpio gpio = {
        IC_CARD_POWER,
        value,
    };
    return fdrive_write(DRIVER_GPIO, (void *)&gpio,sizeof(struct gpio));
}

/**************************************************************************************
* FunctionName   : ic_read_data()
* Description    : 读取寄存器内容
* EntryParameter : reg,寄存器地址, buf,缓冲区, length,缓存区长度
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t __used ic_read_data(uint8_t *reg,  uint8_t *buf, int16_t len)
{
    struct i2c_msg_s msgs[] = {
        [0] = {IC_ADDR, reg, 2, I2C_M_W},
        [1] = {IC_ADDR, buf, len, I2C_M_RD},
    };

	if(fdrive_ioctl(DRIVER_I2C, _IOC_BUS_TRANSPORTS, &msgs,
	        sizeof(msgs)) < 0) {
	    return -EIO;
	}

    return 0;
}

/**************************************************************************************
* FunctionName   : iccard_get()
* Description    : IC卡回数据给MPU
* EntryParameter : args, gpio结构信息
* ReturnValue    : 错误码
**************************************************************************************/
static int32_t iccard_get(uint32_t insert)
{
    Subid msg = SUBID__INIT;
    ProtobufCBinaryData proto;
    uint8_t buffer[MAX_DATA_LEN];
    uint8_t bytedata[MAX_SUBPROTO_LEN];
    struct piddata *pidata = (struct piddata *)buffer;

    Iccard card = ICCARD__INIT;
    uint8_t reg[2] = {0x20 >> 8, 0x20};

    // 1.初始化protobuf具体数据信息
    card.has_insert = 1;
    card.insert = insert;
    card.has_addr = 0;
    card.addr = 0x20;
    card.has_ic = 0;
    card.ic.len = IC_DATA_LEN;
    card.ic.data = mem_malloc(card.ic.len);

    if(insert && !ic_read_data(reg,  card.ic.data, IC_DATA_LEN)) {
        card.has_ic = 1;
        card.has_addr = 1;
    }

    // 2.填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = iccard__get_packed_size(&card);
    proto.data = bytedata;
    iccard__pack(&card, bytedata);

    // 3.准备发送protobuf打包
    pidata->id = ICCARD_PID;
    pidata->len = subid__get_packed_size(&msg);
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));
    mem_free(card.ic.data);
    return 0;
}

/**************************************************************************************
* FunctionName   : iccard_set()
* Description    : 写IC卡
* EntryParameter : args, gpio结构信息
* ReturnValue    : 错误码
**************************************************************************************/
int32_t iccard_set(Subid *msg)
{
    uint16_t i, j;
    uint8_t buf[3];
    Iccard *card = NULL;
    struct i2c_msg_s msgs[] = {
        [0] = {IC_ADDR, buf, 3, I2C_M_W},
    };

    // 1. 处理MPU的数据
    for (j = 0; j < msg->n_subdata; j++) {
        card = iccard__unpack(NULL, msg->subdata[j].len, msg->subdata[j].data);
        if(unlikely(card == NULL)) break;

        if(msg->id == IOC__SET && card->has_addr && card->has_ic) {
            for(i = 0; i < card->ic.len; i++){
                buf[0] = card->addr >> 8;
                buf[1] = card->addr++;
                buf[2] = card->ic.data[i];
                if(fdrive_ioctl(DRIVER_I2C, _IOC_BUS_TRANSPORTS, &msgs, sizeof(msgs)) < 0) {
                    break;
                }
            }
        }
        iccard__free_unpacked(card, NULL);
    }
    return 0;
}

/**************************************************************************************
* FunctionName   : ic_config()
* Description    : IC卡配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t ic_config(uint8_t idx, void *data, int32_t len)
{
    Subid *msg = NULL;
    struct gpio io = {.gpio = (uint32_t)INSERT_GPIO, .value = 0};

    // 1. 检测IC卡插入状态
    if(fdrive_read(DRIVER_GPIO, (void *)&io, sizeof(struct gpio)) < 0) {
        return -EIO;
    }

    // 2. IC卡未插入， 返回
    if(io.value == 1) return -EINVAL;

    // 3.打开IC卡电源开关
    ic_power(1);
    frtos_delay_ms(10);

    // 4.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    // 5.解析子协议
    if(likely(msg->id == IOC__GET)) {
        iccard_get(!io.value);
    } else if(msg->id == IOC__SET) {
        iccard_set(msg);
    }

    // 6.释放内存
    subid__free_unpacked(msg, NULL);

    // 7. 关闭IC卡电源
    ic_power(0);

    (void)idx;
    return len;
}

/**************************************************************************************
* FunctionName   : ic_init()
* Description    : IC卡初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t ic_init(void)
{
    // IC卡下电
    return ic_power(0);
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite ic = {
    .idx   = ICCARD_PID,
    .name  = "ic",
    .init  = ic_init,
    .set   = ic_config,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(ic);
