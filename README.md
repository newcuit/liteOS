# liteOS
基于单片机的轻量级操作系统（调度暂时用freertos提供的）

    MCU在加电复位的时候， CPU自动上访复位向量reset，进行CPU初始化阶段。经过bss,、data段的初始化后，启动freertos操作系统，让系统支持调度功能。
在进入freertos操作系统后，CPU将会通过driver_init函数从__driver_list__列表中依次读取驱动程序的init函数来完成驱动部分的初始化。
然后，进入app_init，通过__app_list__列表完成周期轻量级任务的初始化，最后完成独立任务的初始化（__task_list__），最终进入循环执行轻量级任务的功能。

1、__driver_list__是一种存在于rom存储中的数据结构，该结构原型定义如下：
struct driver_list {
    uint8_t idx;           // 驱动id，由系统统一定义，存放于公共头文件中
    int8_t (*init)(void);     // 驱动初始化函数
    void (*stop)(void);     // 驱动停止函数
    int8_t (*suspend)(void); // 驱动进入休眠
    int8_t (*wakeup)(void); // 唤醒驱动
    int32_t (*read)(uint8_t idx, void *data, int32_t len); //读取驱动数据
    int32_t (*write)(uint8_t idx, void *data, int32_t len);//写入驱动数据
int32_t (*ioctl)(uint8_t idx, int cmd, void *data, int32_t len);//控制驱动
}
     驱动在编写的时候只需要实现静态（const）的driver_list即可，其中init函数是必须实现的。结构体组织成功后，通过宏定义
EARLAY_INIT()、CORE_INIT(), MODULE_INIT()、POST_INIT()等函数将该结构放置于只读数据段，并将该结构的地址放着于代码段的__driver_list__地址上，
形成一张链表，__driver_list__就是该链表的头部。__driver_list__链表内部也存在优先级的链接方式，比如EARLAY_INIT就是链接在链表头部（优先级最高），
POST_INIT链接在链表尾部（表示优先级最低）。EARLY_INIT定义如：
#define EARLAY_INIT(drivers)  static __const struct driver_list * __attribute__((section("__driver_list1__"))) __driver_##drivers = &drviers;
CORE_INIT定义如：#define CORE_INIT(drivers)  static __const struct driver_list * __attribute__((section("__driver_list3__"))) __driver_##drivers = &drviers;
MODULE_INIT定义如：#define MODULE_INIT(drivers)  static __const struct driver_list * __attribute__((section("__driver_list5__"))) __driver_##drivers = &drviers;
POST_INIT定义如：#define POST_INIT(drivers)  static __const struct driver_list * __attribute__((section("__driver_list7__"))) __driver_##drivers = &drviers;

应用程序访问驱动程序的对应接口如下：
int32_t fdrive_read(uint8_t idx, void *data, int32_t len)；          //idx是驱动id
int32_t fdrive_write(uint8_t idx, void *data, int32_t len)；         //idx是驱动id
int32_t fdrive_ioctl(uint8_t idx, int cmd, void *data,  int32_t len)； //idx是驱动id
2、__app_list__同__driver_list__也是一种存在于rom存储中的数据结构，定义方式和__driver_list相似，只是对应的段名有所不同。__app_list__对应的结构体实现如下：
struct app_list {
    uint8_t idx;                       //应用程序对应的id号
    char *name;                      //应用程序对应的名字
    int8_t (*init)(void);                //轻量级应用初始化函数
    void (*run)(void);                 //轻量级任务周期运行函数
    int32_t (*set)(uint8_t idx, void *data, int32_t len);  //设置数据到任务数据区
    int32_t (*get)(uint8_t idx, void *data, int32_t len);  //获取任务处理的数据结果
}
应用程序之间相互访问数据，资源共享的接口如下：
int32_t fuser_data_set(uint8_t idx, void *data, int32_t len);//idx为需要访问的应用ID
int32_t fuser_data_get(uint8_t idx, void *data, int32_t len);//idx为需要访问的应用ID
3、__task_list__同__app_list__也是一种存在于rom存储中的数据结构，定义方式和__app_list相似，只是对应的段名有所不同。__task_list__对应的结构体实现如下：
struct task_list {
    uint8_t idx;                       //应用程序对应的id号
    char *name;                      //应用程序对应的名字
    int8_t (*main)(void);               //主任务main函数，不返回
    int32_t (*set)(uint8_t idx, void *data, int32_t len);  //设置数据到任务数据区
    int32_t (*get)(uint8_t idx, void *data, int32_t len);  //获取任务处理的数据结果
}
应用程序之间相互访问数据，资源共享的接口如下：
int32_t fuser_data_set(uint8_t idx, void *data, int32_t len);//idx为需要访问的应用ID
int32_t fuser_data_get(uint8_t idx, void *data, int32_t len);//idx为需要访问的应用ID
