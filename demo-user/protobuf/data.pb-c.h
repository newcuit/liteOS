/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: data.proto */

#ifndef PROTOBUF_C_data_2eproto__INCLUDED
#define PROTOBUF_C_data_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003000 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Subid Subid;
typedef struct _Ack Ack;
typedef struct _Gpio Gpio;
typedef struct _Voltage Voltage;
typedef struct _Gsensor Gsensor;
typedef struct _Gsensor__Accel Gsensor__Accel;
typedef struct _Gsensor__Gyro Gsensor__Gyro;
typedef struct _Gps Gps;
typedef struct _Vehic Vehic;
typedef struct _Can Can;
typedef struct _Flash Flash;
typedef struct _Version Version;
typedef struct _Rtc Rtc;
typedef struct _SerialSet SerialSet;
typedef struct _SerialData SerialData;


/* --- enums --- */

typedef enum _IOC {
  IOC__SET = 1,
  IOC__GET = 2,
  IOC__DATA = 3,
  IOC__SEND = 4,
  IOC__FILTER = 5,
  IOC__POWERON = 6,
  IOC__LEDON = 7,
  IOC__INIT = 8,
  IOC__CLEAR = 9,
  IOC__REBOOT = 10,
  IOC__LEDOFF = 11,
  IOC__PWROFF = 12,
  IOC__SETWAKE = 13,
  IOC__SUSPEND = 14,
  IOC__SLEEP = 15,
  IOC__UPGRADE = 16
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(IOC)
} IOC;

/* --- messages --- */

struct  _Subid
{
  ProtobufCMessage base;
  int32_t id;
  size_t n_subdata;
  ProtobufCBinaryData *subdata;
};
#define SUBID__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&subid__descriptor) \
    , 0, 0,NULL }


/*
 * ack struct
 */
struct  _Ack
{
  ProtobufCMessage base;
  int32_t type;
  protobuf_c_boolean has_rev1;
  int32_t rev1;
};
#define ACK__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ack__descriptor) \
    , 0, 0, 0 }


/*
 * gpio struct
 */
struct  _Gpio
{
  ProtobufCMessage base;
  int32_t gpio;
  protobuf_c_boolean has_value;
  int32_t value;
};
#define GPIO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gpio__descriptor) \
    , 0, 0, 0 }


/*
 * adc struct
 */
struct  _Voltage
{
  ProtobufCMessage base;
  int32_t id;
  protobuf_c_boolean has_value;
  int32_t value;
};
#define VOLTAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&voltage__descriptor) \
    , 0, 0, 0 }


struct  _Gsensor__Accel
{
  ProtobufCMessage base;
  int32_t x;
  int32_t y;
  int32_t z;
};
#define GSENSOR__ACCEL__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gsensor__accel__descriptor) \
    , 0, 0, 0 }


struct  _Gsensor__Gyro
{
  ProtobufCMessage base;
  int32_t x;
  int32_t y;
  int32_t z;
};
#define GSENSOR__GYRO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gsensor__gyro__descriptor) \
    , 0, 0, 0 }


/*
 * gsensor
 */
struct  _Gsensor
{
  ProtobufCMessage base;
  int32_t interval;
  Gsensor__Accel *a;
  Gsensor__Gyro *g;
  protobuf_c_boolean has_event;
  int32_t event;
  int32_t threshold;
};
#define GSENSOR__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gsensor__descriptor) \
    , 0, NULL, NULL, 0, 0, 0 }


/*
 * gps struct
 */
struct  _Gps
{
  ProtobufCMessage base;
  ProtobufCBinaryData nmea;
};
#define GPS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gps__descriptor) \
    , {0,NULL} }


/*
 * vehicle struct
 */
struct  _Vehic
{
  ProtobufCMessage base;
  int32_t interval;
  protobuf_c_boolean has_acc;
  int32_t acc;
  protobuf_c_boolean has_total;
  int32_t total;
  protobuf_c_boolean has_pulse;
  int32_t pulse;
};
#define VEHIC__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&vehic__descriptor) \
    , 0, 0, 0, 0, 0, 0, 0 }


/*
 * can struct
 */
struct  _Can
{
  ProtobufCMessage base;
  uint32_t canid;
  uint32_t id;
  protobuf_c_boolean has_data;
  ProtobufCBinaryData data;
};
#define CAN__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&can__descriptor) \
    , 0, 0, 0, {0,NULL} }


/*
 * flash struct
 */
struct  _Flash
{
  ProtobufCMessage base;
  uint32_t offset;
  uint32_t size;
  protobuf_c_boolean has_data;
  ProtobufCBinaryData data;
};
#define FLASH__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&flash__descriptor) \
    , 0, 0, 0, {0,NULL} }


/*
 *version
 */
struct  _Version
{
  ProtobufCMessage base;
  uint32_t btime;
  ProtobufCBinaryData os_name;
};
#define VERSION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&version__descriptor) \
    , 0, {0,NULL} }


/*
 *RTC struct
 */
struct  _Rtc
{
  ProtobufCMessage base;
  uint32_t rtime;
};
#define RTC__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&rtc__descriptor) \
    , 0 }


/*
 *SERIAL struct setconfig
 */
struct  _SerialSet
{
  ProtobufCMessage base;
  uint32_t port;
  uint32_t baudrate;
  uint32_t parity;
  uint32_t databit;
  uint32_t stopbit;
};
#define SERIAL__SET__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&serial__set__descriptor) \
    , 0, 0, 0, 0, 0 }


/*
 *SERIAL struct data
 */
struct  _SerialData
{
  ProtobufCMessage base;
  uint32_t port;
  ProtobufCBinaryData data;
};
#define SERIAL__DATA__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&serial__data__descriptor) \
    , 0, {0,NULL} }


/* Subid methods */
void   subid__init
                     (Subid         *message);
size_t subid__get_packed_size
                     (const Subid   *message);
size_t subid__pack
                     (const Subid   *message,
                      uint8_t             *out);
size_t subid__pack_to_buffer
                     (const Subid   *message,
                      ProtobufCBuffer     *buffer);
Subid *
       subid__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   subid__free_unpacked
                     (Subid *message,
                      ProtobufCAllocator *allocator);
/* Ack methods */
void   ack__init
                     (Ack         *message);
size_t ack__get_packed_size
                     (const Ack   *message);
size_t ack__pack
                     (const Ack   *message,
                      uint8_t             *out);
size_t ack__pack_to_buffer
                     (const Ack   *message,
                      ProtobufCBuffer     *buffer);
Ack *
       ack__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ack__free_unpacked
                     (Ack *message,
                      ProtobufCAllocator *allocator);
/* Gpio methods */
void   gpio__init
                     (Gpio         *message);
size_t gpio__get_packed_size
                     (const Gpio   *message);
size_t gpio__pack
                     (const Gpio   *message,
                      uint8_t             *out);
size_t gpio__pack_to_buffer
                     (const Gpio   *message,
                      ProtobufCBuffer     *buffer);
Gpio *
       gpio__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gpio__free_unpacked
                     (Gpio *message,
                      ProtobufCAllocator *allocator);
/* Voltage methods */
void   voltage__init
                     (Voltage         *message);
size_t voltage__get_packed_size
                     (const Voltage   *message);
size_t voltage__pack
                     (const Voltage   *message,
                      uint8_t             *out);
size_t voltage__pack_to_buffer
                     (const Voltage   *message,
                      ProtobufCBuffer     *buffer);
Voltage *
       voltage__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   voltage__free_unpacked
                     (Voltage *message,
                      ProtobufCAllocator *allocator);
/* Gsensor__Accel methods */
void   gsensor__accel__init
                     (Gsensor__Accel         *message);
/* Gsensor__Gyro methods */
void   gsensor__gyro__init
                     (Gsensor__Gyro         *message);
/* Gsensor methods */
void   gsensor__init
                     (Gsensor         *message);
size_t gsensor__get_packed_size
                     (const Gsensor   *message);
size_t gsensor__pack
                     (const Gsensor   *message,
                      uint8_t             *out);
size_t gsensor__pack_to_buffer
                     (const Gsensor   *message,
                      ProtobufCBuffer     *buffer);
Gsensor *
       gsensor__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gsensor__free_unpacked
                     (Gsensor *message,
                      ProtobufCAllocator *allocator);
/* Gps methods */
void   gps__init
                     (Gps         *message);
size_t gps__get_packed_size
                     (const Gps   *message);
size_t gps__pack
                     (const Gps   *message,
                      uint8_t             *out);
size_t gps__pack_to_buffer
                     (const Gps   *message,
                      ProtobufCBuffer     *buffer);
Gps *
       gps__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gps__free_unpacked
                     (Gps *message,
                      ProtobufCAllocator *allocator);
/* Vehic methods */
void   vehic__init
                     (Vehic         *message);
size_t vehic__get_packed_size
                     (const Vehic   *message);
size_t vehic__pack
                     (const Vehic   *message,
                      uint8_t             *out);
size_t vehic__pack_to_buffer
                     (const Vehic   *message,
                      ProtobufCBuffer     *buffer);
Vehic *
       vehic__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   vehic__free_unpacked
                     (Vehic *message,
                      ProtobufCAllocator *allocator);
/* Can methods */
void   can__init
                     (Can         *message);
size_t can__get_packed_size
                     (const Can   *message);
size_t can__pack
                     (const Can   *message,
                      uint8_t             *out);
size_t can__pack_to_buffer
                     (const Can   *message,
                      ProtobufCBuffer     *buffer);
Can *
       can__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   can__free_unpacked
                     (Can *message,
                      ProtobufCAllocator *allocator);
/* Flash methods */
void   flash__init
                     (Flash         *message);
size_t flash__get_packed_size
                     (const Flash   *message);
size_t flash__pack
                     (const Flash   *message,
                      uint8_t             *out);
size_t flash__pack_to_buffer
                     (const Flash   *message,
                      ProtobufCBuffer     *buffer);
Flash *
       flash__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   flash__free_unpacked
                     (Flash *message,
                      ProtobufCAllocator *allocator);
/* Version methods */
void   version__init
                     (Version         *message);
size_t version__get_packed_size
                     (const Version   *message);
size_t version__pack
                     (const Version   *message,
                      uint8_t             *out);
size_t version__pack_to_buffer
                     (const Version   *message,
                      ProtobufCBuffer     *buffer);
Version *
       version__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   version__free_unpacked
                     (Version *message,
                      ProtobufCAllocator *allocator);
/* Rtc methods */
void   rtc__init
                     (Rtc         *message);
size_t rtc__get_packed_size
                     (const Rtc   *message);
size_t rtc__pack
                     (const Rtc   *message,
                      uint8_t             *out);
size_t rtc__pack_to_buffer
                     (const Rtc   *message,
                      ProtobufCBuffer     *buffer);
Rtc *
       rtc__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   rtc__free_unpacked
                     (Rtc *message,
                      ProtobufCAllocator *allocator);
/* SerialSet methods */
void   serial__set__init
                     (SerialSet         *message);
size_t serial__set__get_packed_size
                     (const SerialSet   *message);
size_t serial__set__pack
                     (const SerialSet   *message,
                      uint8_t             *out);
size_t serial__set__pack_to_buffer
                     (const SerialSet   *message,
                      ProtobufCBuffer     *buffer);
SerialSet *
       serial__set__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   serial__set__free_unpacked
                     (SerialSet *message,
                      ProtobufCAllocator *allocator);
/* SerialData methods */
void   serial__data__init
                     (SerialData         *message);
size_t serial__data__get_packed_size
                     (const SerialData   *message);
size_t serial__data__pack
                     (const SerialData   *message,
                      uint8_t             *out);
size_t serial__data__pack_to_buffer
                     (const SerialData   *message,
                      ProtobufCBuffer     *buffer);
SerialData *
       serial__data__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   serial__data__free_unpacked
                     (SerialData *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Subid_Closure)
                 (const Subid *message,
                  void *closure_data);
typedef void (*Ack_Closure)
                 (const Ack *message,
                  void *closure_data);
typedef void (*Gpio_Closure)
                 (const Gpio *message,
                  void *closure_data);
typedef void (*Voltage_Closure)
                 (const Voltage *message,
                  void *closure_data);
typedef void (*Gsensor__Accel_Closure)
                 (const Gsensor__Accel *message,
                  void *closure_data);
typedef void (*Gsensor__Gyro_Closure)
                 (const Gsensor__Gyro *message,
                  void *closure_data);
typedef void (*Gsensor_Closure)
                 (const Gsensor *message,
                  void *closure_data);
typedef void (*Gps_Closure)
                 (const Gps *message,
                  void *closure_data);
typedef void (*Vehic_Closure)
                 (const Vehic *message,
                  void *closure_data);
typedef void (*Can_Closure)
                 (const Can *message,
                  void *closure_data);
typedef void (*Flash_Closure)
                 (const Flash *message,
                  void *closure_data);
typedef void (*Version_Closure)
                 (const Version *message,
                  void *closure_data);
typedef void (*Rtc_Closure)
                 (const Rtc *message,
                  void *closure_data);
typedef void (*SerialSet_Closure)
                 (const SerialSet *message,
                  void *closure_data);
typedef void (*SerialData_Closure)
                 (const SerialData *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    ioc__descriptor;
extern const ProtobufCMessageDescriptor subid__descriptor;
extern const ProtobufCMessageDescriptor ack__descriptor;
extern const ProtobufCMessageDescriptor gpio__descriptor;
extern const ProtobufCMessageDescriptor voltage__descriptor;
extern const ProtobufCMessageDescriptor gsensor__descriptor;
extern const ProtobufCMessageDescriptor gsensor__accel__descriptor;
extern const ProtobufCMessageDescriptor gsensor__gyro__descriptor;
extern const ProtobufCMessageDescriptor gps__descriptor;
extern const ProtobufCMessageDescriptor vehic__descriptor;
extern const ProtobufCMessageDescriptor can__descriptor;
extern const ProtobufCMessageDescriptor flash__descriptor;
extern const ProtobufCMessageDescriptor version__descriptor;
extern const ProtobufCMessageDescriptor rtc__descriptor;
extern const ProtobufCMessageDescriptor serial__set__descriptor;
extern const ProtobufCMessageDescriptor serial__data__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_data_2eproto__INCLUDED */
