/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : gps.c
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
#include "frtos_sys.h"
#include "data.pb-c.h"
#include "minmea.h"
#include "frtos_log.h"

/**************************************************************************************
* Description    : 定义GPS上报数据最大长度
**************************************************************************************/
#define MAX_DATA_LEN                 200
#define MAX_SUBPROTO_LEN             100
#define GPS_UART                     DRIVER_UART6

/**************************************************************************************
* Description    : 定义GPS数据每行最大长度
**************************************************************************************/
#define MEA_MAX_LENGTH              80

/**************************************************************************************
* Description    : 定义GPS开关
**************************************************************************************/
#define GPS_PWR_GPIO                PORT_F,PTF2
#define GPS_RST_GPIO                PORT_F,PTF1
static uint8_t debug = 0;                               // GPS调试开关

/**************************************************************************************
* Description    : 定义GPS对应API
**************************************************************************************/
#define MOVE_STEPS(UDATA,LEN,STEP)  do{ UDATA+=STEP;LEN-=STEP; } while(0)
#define FILL_GPS_EOF(SOURCE, LEN) do{ SOURCE[LEN] = '\0'; } while(0)

/**************************************************************************************
* FunctionName   : gps_begin()
* Description    : 获取字符串中，gps的起始偏移
* EntryParameter : GPS报文串,GPS报文串长度
* ReturnValue    : GPS起始偏移
**************************************************************************************/
static inline int gps_begin(char *udata, uint16_t len)
{
    int8_t line_len = 0;

    while(*udata != '$' && len > line_len){
        udata++;
        line_len++;
    }
    return line_len;
}

/**************************************************************************************
* FunctionName   : gps_end()
* Description    : 获取GPS报文长度
* EntryParameter : GPS字符串,GPS报文串长度
* ReturnValue    : GPS报文长度
**************************************************************************************/
static inline int gps_end(char *udata, uint16_t len)
{
    int8_t length = 0;

    while(udata[0] != '\r' && udata[1] != '\n' && len > length){
        udata++;
        length++;
    }
    return length;
}

/**************************************************************************************
* FunctionName   : n303_parse()
* Description    : 解析NMEA报文数据
* EntryParameter : sentence, NMEA报文指针
* ReturnValue    : None
**************************************************************************************/
static __used inline void n303_parse(void *sentence)
{
    uint8_t frame[550] = {0};
    enum minmea_sentence_id nmea_id = 0;

    // 1.获取NEMA报文ID
    nmea_id = minmea_sentence_id(sentence, false);

    // 2.解析NEMA报文
    switch(nmea_id){
    case MINMEA_SENTENCE_RMC:{
    	struct minmea_sentence_rmc *rmc = (struct minmea_sentence_rmc *)frame;
        if(true == minmea_parse_rmc(rmc, sentence)){
            //
        }
        break;
    }
    case MINMEA_SENTENCE_GGA:{
        struct minmea_sentence_gga *gga = (struct minmea_sentence_gga *)frame;
        if(true == minmea_parse_gga(gga, sentence)){
            //
        }
        break;
    }
    case MINMEA_SENTENCE_GSA:{
        struct minmea_sentence_gsa *gsa = (struct minmea_sentence_gsa *)frame;
        if(minmea_parse_gsa(gsa, sentence)){
            //
        }
        break;
    }
    case MINMEA_SENTENCE_GST:{
        struct minmea_sentence_gst *gst = (struct minmea_sentence_gst *)frame;
        if(true == minmea_parse_gst(gst, sentence)){
            //
        }
        break;
    }
    case MINMEA_SENTENCE_GSV:
    case MINMEA_SENTENCE_GPGSV:
    case MINMEA_SENTENCE_BDGSV:{
        struct minmea_sentence_gsv *gsv = (struct minmea_sentence_gsv *)frame;
        if(true == minmea_parse_gsv(gsv, sentence)){
            //
        }
        break;
    }
    case MINMEA_SENTENCE_VTG:{
        struct minmea_sentence_vtg *vtg = (struct minmea_sentence_vtg *)frame;
        if(minmea_parse_vtg(vtg, sentence)){
            //
        }
        break;
    }
    case MINMEA_SENTENCE_TXT:{
        struct minmea_sentence_txt *txt = (struct minmea_sentence_txt *)frame;
        if(minmea_parse_txt(txt, sentence)){
            //
        }
        break;
    }
    case MINMEA_SENTENCE_ZDA:{
        struct minmea_sentence_zda *zda = (struct minmea_sentence_zda *)frame;
        if(minmea_parse_zda(zda, sentence)){
            //
        }
        break;
    }
    default:
        break;
    }
}

/**************************************************************************************
* FunctionName   : gps_report()
* Description    : 将GPS报文发送到MPU
* EntryParameter : GPS字符串,GPS报文串长度
* ReturnValue    : GPS报文长度
**************************************************************************************/
static int32_t gps_report(char *gps, uint32_t len)
{
    uint32_t length = 0;
    ProtobufCBinaryData proto;
    struct piddata *pidata = NULL;

    Subid msg = SUBID__INIT;
    Gps gpsmsg = GPS__INIT;

    gpsmsg.nmea.len = len;
    gpsmsg.nmea.data = (uint8_t *)gps;

    // 填充应答protobuf 子ID信息
    msg.id = IOC__DATA;
    msg.n_subdata = 1;
    msg.subdata = &proto;
    proto.len = gps__get_packed_size(&gpsmsg);
    proto.data = (uint8_t *)mem_malloc(proto.len);
    gps__pack(&gpsmsg, proto.data);

    // 准备发送protobuf打包
    length = subid__get_packed_size(&msg);
    pidata = (struct piddata *)mem_malloc(length + sizeof(struct piddata));

    pidata->id = GPS_PID;
    pidata->len = length;
    subid__pack(&msg, pidata->data);
    fuser_data_set(INIT_PID, pidata, pidata->len + sizeof(struct piddata));

    mem_free(proto.data);
    mem_free(pidata);
    (void)len;

    return 0;
}

/**************************************************************************************
* FunctionName   : gps_recv()
* Description    : gps数据串
* EntryParameter : data，GPS数据， len,gps数据长度
* ReturnValue    : 返回处理的数据长度或者错误码
**************************************************************************************/
static int32_t gps_recv(int32_t idx, void *data, int32_t len)
{
    if(unlikely(debug)) {
        stdio_output(data, len);
    }
    gps_report(data, len);

    (void)idx;
    return 0;
}

/**************************************************************************************
* FunctionName   : gps_config()
* Description    : GPS配置
* EntryParameter : data，指向发送的数据， len,指向发送数据长度
* ReturnValue    : 返回发送状态或者长度
**************************************************************************************/
static int32_t gps_config(uint8_t idx, void *data, int32_t len)
{
    uint8_t i = 0;
    Subid *msg = NULL;
    Gps *gpsconfig = NULL;

    // 1.解开通用子协议数据头
    msg = subid__unpack(NULL,len, data);
    if(unlikely(msg == NULL)) return -EFAULT;

    for (i = 0; i < msg->n_subdata; i++) {
        gpsconfig = gps__unpack(NULL, msg->subdata[i].len, msg->subdata[i].data);
        if(unlikely(gpsconfig == NULL)) break;

        if(likely(msg->id == IOC__SET)) {
#if 0
            fdrive_write(GPS_UART, gpsconfig->nmea, strlen(gpsconfig->nmea));
#else
            fdrive_write(GPS_UART, gpsconfig->nmea.data, gpsconfig->nmea.len);
#endif
        }
        gps__free_unpacked(gpsconfig, NULL);
    }
    // 2.释放内存
    subid__free_unpacked(msg, NULL);

    (void)idx;
    return len;
}

/**************************************************************************************
* TypeName       : gps_cmd()
* Description    : gps控制指令
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t gps_cmd(char *argv, int32_t argc)
{
    //1.设置GPS的debug收发开关
    if(argc == 2 && 0 == strcmp(argv, "-h")) {
        stdio_printf("gps debug on"STRBR);
        stdio_printf("gps debug off"STRBR);
    } else if (argc == 8 && 0 == strcmp(argv, "debug on")) {
        debug = 1;
    } else if (argc == 9 && 0 == strcmp(argv, "debug off")) {
        debug = 0;
    }
    return argc;
}

/**************************************************************************************
* FunctionName   : gps_init()
* Description    : GPS初始化初始化
* EntryParameter : None
* ReturnValue    : 返回错误码
**************************************************************************************/
static int32_t gps_init(void)
{
    struct gpio_args_s gpio[] = {{GPS_PWR_GPIO, 1}, {GPS_RST_GPIO, 1}};

    /* 打开GPS模块电源 */
	fdrive_ioctl(DRIVER_GPIO, _IOC_SET_DATA, (void *)&gpio[0], sizeof(struct gpio_args_s));

    /* 拉高GPS模块复位脚  */
    fdrive_ioctl(DRIVER_GPIO, _IOC_SET_DATA, (void *)&gpio[1], sizeof(struct gpio_args_s));

    return fdrive_ioctl(GPS_UART, _IOC_SET_CB, gps_recv, sizeof(gps_recv));
}

/**************************************************************************************
* Description    : 定义通信任务结构
**************************************************************************************/
static __const struct applite gps = {
    .idx   = GPS_PID,
    .name  = "gps",
    .init  = gps_init,
    .set   = gps_config,
    .debug = gps_cmd,
};

/**************************************************************************************
* Description    : 模块初始化
**************************************************************************************/
APP_REGISTER(gps);
