/**************************************************************************************
 **** Copyright (C), 2017, xx xx xx xx info&tech Co., Ltd.

 * File Name     : frtos_utils.c
 * Author        :
 * Date          : 2017-08-17
 * Description   : .C file function description
 * Version       : 1.0
**************************************************************************************/
#include "frtos_errno.h"
#include "frtos_mem.h"
#include "frtos_utils.h"

/**************************************************************************************
* FunctionName   : utils_itoa()
* Description    : 数字转字符串
* EntryParameter : val,待转换数字, buf,字符BUF, radix,(进制:2,8,10,16)
* ReturnValue    : 返回错误码, 返回转换后的字符串长度
**************************************************************************************/
int16_t utils_itoa(uint32_t val, void *buf, uint8_t radix)
{
    uint8_t *p = (uint8_t *)buf;
    uint8_t *firstdig = p;
    uint8_t temp = 0;
    uint8_t digval = 0;

    do{
        digval = (uint8_t)(val % radix);
        val /= radix;
        if(digval > 9)
            *p++ = (uint8_t)(digval - 10 + 'a');
        else
            *p++ = (uint8_t)(digval + '0');
    }while(val > 0);

    *p-- = '\0';
    do{
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;
        --p;
        ++firstdig;
    }while(firstdig < p);

    return strlen(buf);
}

/**************************************************************************************
* FunctionName   : utils_calc_bcc()
* Description    : 计算BCC检验码
* EntryParameter : dat,数据, size,数据长度
* ReturnValue    : 返回BCC校验码
**************************************************************************************/
uint8_t utils_calc_bcc(const void *dat, uint16_t size)
{
    uint8_t bcc = *(uint8_t *)dat;
    uint8_t *p = (uint8_t *)dat + 1;

    if(NULL == dat || size <= 0){
        return 0;
    }
    while(--size){
        bcc ^= *p;
        p++;
    }

    return bcc;
}

/**************************************************************************************
* FunctionName   : utils_calc_lrc()
* Description    : 计算LRC检验码
* EntryParameter : dat,数据, size,数据长度
* ReturnValue    : 返回LRCC校验码
**************************************************************************************/
uint8_t utils_calc_lrc(const void *dat, uint16_t size)
{
    uint8_t lrc = *(uint8_t *)dat;
    uint8_t *p = (uint8_t *)dat + 1;

    if(NULL == dat || size <= 0){
        return 0;
    }
    while(--size){
        lrc ^= *p;
        p++;
    }

    return ~lrc;
}

/**************************************************************************************
* FunctionName   : utils_char2bcd()
* Description    : 字符转BCD码(Binary-Coded Decimal)
* EntryParameter : ch,要转换的字符
* ReturnValue    : 返回转换后的BCD码
**************************************************************************************/
uint8_t utils_char2bcd(const uint8_t ch)
{
    if(ch >= '0' && ch <= '9') return ch - '0';
    if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;

    return 0;
}

/**************************************************************************************
* FunctionName   : utils_str2bcd()
* Description    : 字符串转BCD码(Binary-Coded Decimal)
* EntryParameter : *str,要转换的字符串, bcd_ar,返回转换后的BCD码数组,
                   arr_len,数组长度
* ReturnValue    : 返回转换后的数据个数
**************************************************************************************/
int16_t utils_str2bcd(const char *str, uint8_t *bcd_arr, int16_t arr_len)
{
    int16_t i = 0, j = 0;
    uint8_t high = 0, low = 0;

    if(NULL == bcd_arr || 0 == arr_len){
        return 0;
    }
    for(i = 0; i < (int16_t)strlen(str); i += 2){
        high=utils_char2bcd(str[i]);
        if((i + 1) < (int16_t)strlen(str)){
            low = utils_char2bcd(str[i+1]);
        }else{
            low = 0xF;
        }
        if(j < arr_len){
            bcd_arr[j++] = (high << 4) + low;
        }else{
            return j;
        }
    }

    return j;
}

/**************************************************************************************
* FunctionName   : utils_mem_test()
* Description    : 测试一段内存是否为指定字符
* EntryParameter : ch,待检测的字节, p_mem,待检测内存, len,长度
* ReturnValue    : 返回错误码
**************************************************************************************/
int8_t utils_mem_test(char ch, void *mem, uint16_t len)
{
    uint16_t i = 0;

    for(i = 0; i < len; i++){
        if(((char *)mem)[i] != ch)return -EIO;
    }
    return 0;
}

/**************************************************************************************
* FunctionName   : mem_scannf
* Description    : 数据解析函数，类似于sscanf，但不需要开辟新的内存空间
* EntryParameter : 同sscanf
* ReturnValue    : 返回解析到的位置:0，表示解析失败
**************************************************************************************/
int __optimize("O3") mem_scannf(char *buffer, int len, char *fmt,...)
{
    int i = 0;
    va_list arg;

    va_start(arg,fmt);
    while (unlikely(len > 0 && fmt[i] != '\0')) {
        if (unlikely(fmt[i] == '%' && fmt[i+1] == 'd')) {
            int *intp = va_arg(arg,int *);
            *intp = atoi(buffer);
            i++;
        } else if (unlikely(fmt[i] == '%' && fmt[i+1] == 's')) {
            char **buf = va_arg(arg, char **);
            *buf = buffer;
            i++;
        } else {
           while (likely(len > 0 && *buffer != fmt[i])) {
               buffer++;
               len --;
           }
           if(len > 0 && *buffer == fmt[i]) {
               *buffer++ = '\0';
               len--;
           }
        }
        i++;
    }
    va_end(arg);
    return len;
}

/**************************************************************************************
* FunctionName   : memstr
* Description    : 字符串内存查找，类似于strstr，但支持长度传入，避免越界
* EntryParameter : 同strstr
* ReturnValue    : 返回找到的子串位置，未找到返回NULL
**************************************************************************************/
char __optimize("O3") *memstr(char *src, int len, char *substr)
{
    if (unlikely(src == NULL || len <= 0 || substr == NULL)) {
        return NULL;
    }

    if (unlikely(*substr == '\0')) {
        return NULL;
    }

    int sublen = strlen(substr);

    int i;
    char* cur = src;
    int last_possible = len - sublen + 1;
    for (i = 0; i < last_possible; i++) {
        if (unlikely(*cur == *substr)) {
            if (unlikely(memcmp(cur, substr, sublen) == 0)) {
                return cur;
            }
        }
        cur++;
    }

    return NULL;
}
