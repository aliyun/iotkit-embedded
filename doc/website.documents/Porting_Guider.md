**以下详细描述如何将华东2节点设备端V2.0版本C-SDK移植到目标硬件平台.**

## V2.0设备端C-SDK概述

        +---------------------------+            +---------------------------+
        |                           |            |                           | =>  构建完成后产生:
        |  IoT SDK Example Program  |            | sample/mqtt|coap|ota/*.c  |
        |                           |            |                           |     output/release/bin/*-example
        +---------------------------+            +---------------------------+
        |                           |            |                           | =>  SDK提供功能的API, 都在这里声明
        |  IoT SDK Interface Layer  |            | src/sdk-impl/iot_export.h | =>  构建完成后产生:
        |                           |            |                           |
        |     IOT_XXX_YYY() APIs    |            |  Has all APIs' prototype  |     output/release/include/iot-sdk/iot_export.h
        |                           |            |                           |     output/release/include/iot-sdk/exports/*.h
        +---------------------------+            +---------------------------+
        |                           |            |                           | =>  SDK提供功能的API, 都在这里实现
        |                           |            | src/utils: utilities      | =>  构建完成后产生:
        |                           |   +--->    | src/log: logging          |
        |  IoT SDK Core Implements  |            | src/guider: authenticate  |     output/release/lib/libiot_sdk.a
        |  : =>                     |   <---+    | src/system: device mgmt   |
        |  : You SHOULD NOT Focus   |            | src/mqtt: MQTT client     |
        |  : on this unless         |            | src/coap: CoAP client     |
        |  : you're debugging bugs  |            | src/shadow: device shadow |
        |                           |            | src/ota: OTA channel      |
        |                           |            |                           |
        +---------------------------+            +---------------------------+
        |                           |            |                           | =>  SDK仅含有示例代码, 移植时需二次开发
        |  Hardware Abstract Layer  |            | src/sdk-impl/iot_import.h | =>  构建完成后产生:
        |                           |            | : =>                      |
        |     HAL_XXX_YYY() APIs    |            | : HAL_*() declarations    |     output/release/lib/libiot_platform.a
        |                           |            |                           |
        |  : You MUST Implement     |            | src/platform/*/*/*.c      |     output/release/include/iot-sdk/iot_import.h
        |  : this part for your     |            | : =>                      |     output/release/include/iot-sdk/imports/*.h
        |  : target device first    |            | : HAL_*() example impls   |
        +---------------------------+            +---------------------------+

2.0版本相对1.0.1版本在结构性方面, 升级了编译系统, 支持后续功能模块的灵活迭代和裁剪, 但是在代码架构方面, 和1.0.1版本保持恒定, 也是分为三层的, 如上图

* 最底层称为"硬件平台抽象层", 也简称`HAL层`, 对应上图的 `Hardware Abstract Layer`
    - 这里是抽象不同的嵌入式目标板上操作系统对我们SDK的支撑函数
    - 举例来说, 包括网络收发, TLS/DTLS通道建立和读写, 内存申请是否, 互斥量加锁解锁等
    - **注意: 在任何跨平台移植时, 实现这部分是需要完成的第一步工作**
    - **注意: 阿里的SDK里, 并不含有多平台的HAL层实现, 但我们提供了Linux桌面OS(Ubuntu16.04)上的参考实现, 移植时可以作为参考**

* 中间层称为"SDK内核实现层", 对应上图的 `IoT SDK Core Implements`
    - 这里是IoT套件C-SDK的核心实现部分, 它基于HAL层接口完成了MQTT/CoAP通道等的功能封装
    - 举例来说, 包括MQTT的连接建立, 报文收发, CoAP的连接建立, 报文收发, OTA的固件状态查询, OTA的固件下载等
    - **注意: 如果HAL层实现的好, 这一层在跨平台移植时, 理想情况不需要做任何修改**

* 最上层称为"SDK接口声明层", 对应上图的 `IoT SDK Interface Layer`
    - 这里没有实现, 只有一系列C函数的原型声明, 也就是SDK跨平台移植完成之后, 可以用于编写业务逻辑, 和阿里云服务器通信的API
    - 举例来说, 怎么去使用这些API做业务逻辑, 我们在`sample`目录提供了丰富的示例程序, 并且只要填入了设备信息, 就可以在Linux主机上运行体验

以下按照从下到上的顺序, 逐个对每个层次做更加详细的说明

## 硬件平台抽象层(HAL层)

* 所有HAL层函数的声明都在 `src/sdk-impl/iot_import.h` 这个头文件中列出
* 各功能点引入的HAL层接口依赖在`src/sdk-impl/imports/iot_import_*.h`中列出
* 这些`imports`目录下的子文件, 都被`src/sdk-impl/iot_import.h`包含
* 在V2.0版本的编译系统中, 这个部分会被编译成`output/release/lib/libiot_platform.a`

以下可以列出所有跨平台移植时需要实现的HAL层接口

    src/sdk-impl$ grep -ro "HAL_[_A-Za-z0-9]*" *|cut -d':' -f2|sort -u|cat -n

     1  HAL_DTLSSession_create
     2  HAL_DTLSSession_free
     3  HAL_DTLSSession_init
     4  HAL_DTLSSession_read
     5  HAL_DTLSSession_write
     6  HAL_Free
     7  HAL_GetPartnerID
     8  HAL_Malloc
     9  HAL_MutexCreate
    10  HAL_MutexDestroy
    11  HAL_MutexLock
    12  HAL_MutexUnlock
    13  HAL_Printf
    14  HAL_SleepMs
    15  HAL_SSL_Destroy
    16  HAL_SSL_Establish
    17  HAL_SSL_Read
    18  HAL_SSL_Write
    19  HAL_TCP_Destroy
    20  HAL_TCP_Establish
    21  HAL_TCP_Read
    22  HAL_TCP_Write
    23  HAL_UDP_close
    24  HAL_UDP_create
    25  HAL_UDP_read
    26  HAL_UDP_readTimeout
    27  HAL_UDP_resolveAddress
    28  HAL_UDP_write
    29  HAL_UptimeMs

对这些函数做实现的时候, 可以参考`src/platform`下已经写好的示例, 这些示例在`Ubuntu16.04`主机上被完善的编写和测试过

    src/platform$ tree
    .
    +-- iot.mk
    +-- os
    |   +-- linux
    |       +-- HAL_OS_linux.c
    |       +-- HAL_TCP_linux.c
    |       +-- HAL_UDP_linux.c
    +-- ssl
        +-- mbedtls
            +-- HAL_DTLS_mbedtls.c
            +-- HAL_TLS_mbedtls.c

以下是这些函数的一个说明表格, 更多详细信息, 请查阅代码中的注释

| 序号  | 函数名                   | 说明                                                                    |
|-------|--------------------------|-------------------------------------------------------------------------|
|     1 | HAL_DTLSSession_create   | 建立一个DTLS连接, 用于CoAP功能                                          |
|     2 | HAL_DTLSSession_free     | 销毁一个DTLS连接, 用于CoAP功能                                          |
|     3 | HAL_DTLSSession_init     | 初始化一个DTLS连接需要用到的数据结构, 在HAL_DTLSSession_create之前调用  |
|     4 | HAL_DTLSSession_read     | 从DTLS连接中读数据, 用于CoAP功能                                        |
|     5 | HAL_DTLSSession_write    | 向DTLS连接中写数据, 用于CoAP功能                                        |
|     6 | HAL_Free                 | 释放一片堆上内存                                                        |
|     7 | HAL_GetPartnerID         | 用于紧密合作伙伴, 一般客户只需要在此可实现为空函数                      |
|     8 | HAL_Malloc               | 申请一片堆上内存                                                        |
|     9 | HAL_MutexCreate          | 创建一个互斥量, 用于同步控制, 目前SDK仅支持单线程应用, 可实现为空函数   |
|    10 | HAL_MutexDestroy         | 销毁一个互斥量, 用于同步控制, 目前SDK仅支持单线程应用, 可实现为空函数   |
|    11 | HAL_MutexLock            | 加锁一个互斥量, 用于同步控制, 目前SDK仅支持单线程应用, 可实现为空函数   |
|    12 | HAL_MutexUnlock          | 解锁一个互斥量, 用于同步控制, 目前SDK仅支持单线程应用, 可实现为空函数   |
|    13 | HAL_Printf               | 打印函数, 用于向串口或其它标准输出打印日志或调试信息                    |
|    14 | HAL_SleepMs              | 睡眠函数, 使当前执行线程睡眠指定的毫秒数                                |
|    15 | HAL_SSL_Destroy          | 销毁一个TLS连接, 用于MQTT功能, HTTPS功能                                |
|    16 | HAL_SSL_Establish        | 建立一个TLS连接, 用于MQTT功能, HTTPS功能                                |
|    17 | HAL_SSL_Read             | 从一个TLS连接中读数据, 用于MQTT功能, HTTPS功能                          |
|    18 | HAL_SSL_Write            | 向一个TLS连接中写数据, 用于MQTT功能, HTTPS功能                          |
|    19 | HAL_TCP_Destroy          | 销毁一个TLS连接, 用于MQTT功能, HTTPS功能                                |
|    20 | HAL_TCP_Establish        | 建立一个TCP连接, 包含了域名解析的动作和TCP连接的建立                    |
|    21 | HAL_TCP_Read             | 在指定时间内, 从TCP连接读取流数据, 并返回读到的字节数                   |
|    22 | HAL_TCP_Write            | 在指定时间内, 向TCP连接发送流数据, 并返回发送的字节数                   |
|    23 | HAL_UDP_close            | 关闭一个UDP连接的socket, 用于CoAP功能                                   |
|    24 | HAL_UDP_create           | 创建一个UDP连接的socket, 用于CoAP功能                                   |
|    25 | HAL_UDP_read             | 阻塞的从一个UDP连接中读取数据包, 并返回读到的字节数, 用于CoAP功能       |
|    26 | HAL_UDP_readTimeout      | 在指定时间内, 从一个UDP连接中读取数据包, 返回读到的字节数, 用于CoAP功能 |
|    27 | HAL_UDP_resolveAddress   | 解析一个可能是UDP服务器的域名地址为IP地址, 用于CoAP功能                 |
|    28 | HAL_UDP_write            | 阻塞的向一个UDP连接中发送数据包, 并返回发送的字节数, 用于CoAP功能       |
|    29 | HAL_UptimeMs             | 时钟函数, 获取本设备从加电以来到目前时间点已经过去的毫秒数              |

在这些HAL接口中

**必选实现**

| 序号  | 函数名                   | 说明                                                                    |
|-------|--------------------------|-------------------------------------------------------------------------|
|     6 | HAL_Free                 | 释放一片堆上内存                                                        |
|     8 | HAL_Malloc               | 申请一片堆上内存                                                        |
|    13 | HAL_Printf               | 打印函数, 用于向串口或其它标准输出打印日志或调试信息                    |
|    14 | HAL_SleepMs              | 睡眠函数, 使当前执行线程睡眠指定的毫秒数                                |
|    29 | HAL_UptimeMs             | 时钟函数, 获取本设备从加电以来到目前时间点已经过去的毫秒数              |

**可实现为空**

| 序号  | 函数名                   | 说明                                                                    |
|-------|--------------------------|-------------------------------------------------------------------------|
|     7 | HAL_GetPartnerID         | 用于紧密合作伙伴, 一般客户只需要在此可实现为空函数                      |
|     9 | HAL_MutexCreate          | 创建一个互斥量, 用于同步控制, 目前SDK仅支持单线程应用, 可实现为空函数   |
|    10 | HAL_MutexDestroy         | 销毁一个互斥量, 用于同步控制, 目前SDK仅支持单线程应用, 可实现为空函数   |
|    11 | HAL_MutexLock            | 加锁一个互斥量, 用于同步控制, 目前SDK仅支持单线程应用, 可实现为空函数   |
|    12 | HAL_MutexUnlock          | 解锁一个互斥量, 用于同步控制, 目前SDK仅支持单线程应用, 可实现为空函数   |

**没有MQTT时可实现为空**

| 序号  | 函数名                   | 说明                                                                    |
|-------|--------------------------|-------------------------------------------------------------------------|
|    15 | HAL_SSL_Destroy          | 销毁一个TLS连接, 用于MQTT功能, HTTPS功能                                |
|    16 | HAL_SSL_Establish        | 建立一个TLS连接, 用于MQTT功能, HTTPS功能                                |
|    17 | HAL_SSL_Read             | 从一个TLS连接中读数据, 用于MQTT功能, HTTPS功能                          |
|    18 | HAL_SSL_Write            | 向一个TLS连接中写数据, 用于MQTT功能, HTTPS功能                          |
|    19 | HAL_TCP_Destroy          | 销毁一个TLS连接, 用于MQTT功能, HTTPS功能                                |
|    20 | HAL_TCP_Establish        | 建立一个TCP连接, 包含了域名解析的动作和TCP连接的建立                    |
|    21 | HAL_TCP_Read             | 在指定时间内, 从TCP连接读取流数据, 并返回读到的字节数                   |
|    22 | HAL_TCP_Write            | 在指定时间内, 向TCP连接发送流数据, 并返回发送的字节数                   |

**没有CoAP时可实现为空**

| 序号  | 函数名                   | 说明                                                                    |
|-------|--------------------------|-------------------------------------------------------------------------|
|     1 | HAL_DTLSSession_create   | 建立一个DTLS连接, 用于CoAP功能                                          |
|     2 | HAL_DTLSSession_free     | 销毁一个DTLS连接, 用于CoAP功能                                          |
|     3 | HAL_DTLSSession_init     | 初始化一个DTLS连接需要用到的数据结构, 在HAL_DTLSSession_create之前调用  |
|     4 | HAL_DTLSSession_read     | 从DTLS连接中读数据, 用于CoAP功能                                        |
|     5 | HAL_DTLSSession_write    | 向DTLS连接中写数据, 用于CoAP功能                                        |
|    23 | HAL_UDP_close            | 关闭一个UDP连接的socket, 用于CoAP功能                                   |
|    24 | HAL_UDP_create           | 创建一个UDP连接的socket, 用于CoAP功能                                   |
|    25 | HAL_UDP_read             | 阻塞的从一个UDP连接中读取数据包, 并返回读到的字节数, 用于CoAP功能       |
|    26 | HAL_UDP_readTimeout      | 在指定时间内, 从一个UDP连接中读取数据包, 返回读到的字节数, 用于CoAP功能 |
|    27 | HAL_UDP_resolveAddress   | 解析一个可能是UDP服务器的域名地址为IP地址, 用于CoAP功能                 |
|    28 | HAL_UDP_write            | 阻塞的向一个UDP连接中发送数据包, 并返回发送的字节数, 用于CoAP功能       |

## SDK内核实现层

* 所有被提供的函数的声明都在 `src/sdk-impl/iot_export.h` 这个头文件中列出
* 各功能点提供的接口在`src/sdk-impl/exports/iot_export_*.h`中列出
* 这些`exports`目录下的子文件, 都被`src/sdk-impl/iot_export.h`包含
* 在V2.0版本的编译系统中, 这个部分会被编译成`output/release/lib/libiot_sdk.a`

## SDK接口声明层 + 例程

请参考[快速接入页面](https://help.aliyun.com/document_detail/30530.html)


**以下详细描述如何将华东2节点设备端V1.0.1版本C-SDK移植到目标硬件平台.**
## V1.0.1设备端C-SDK简介
SDK基本框架如下图所示：

![SDK框架](http://docs-aliyun.cn-hangzhou.oss.aliyun-inc.com/assets/pic/56047/cn_zh/1499417498273/iotx-sdk.jpg)

* SDK大致可分为硬件抽象层、SDK内核代码、面向应用的API。
* 在移植到目标硬件平台时，需要根据硬件平台的情况实现硬件平台抽象接口。
* 在硬件平台抽象层中，包含OS层、network层、ssl层等3类。其中，OS层主要包括时间、互斥锁以及其它等接口，在目录$(SDK_PATH)/src/platform/os/下；network层主要包括网络相关的接口（目前为TCP接口），在目录$(SDK_PATH)/src/platform/network/下；ssl层包含ssl或tls相关接口，在目录$(SDK_PATH)/src/platform/ssl/下。

## 硬件平台抽象层
硬件平台抽象层包含数据类型、OS（或硬件）接口、TCPIP网络接口、SSL（TLS）接口等4个部分。下面分别对这4部分进行叙述。

### 数据类型


* 表1-自定义数据类型

| 序号  | 数据类型名称  | 说明  |
| -------------- | ------------ | ------------ |
| 1  | bool      | bool类型  |
| 2  | int8_t    | 8比特有符号整型  |
| 3  | uint8_t   | 8比特无符号整型  |
| 4  | int16_t   | 16比特有符号整型 |
| 5  | uint16_t  | 16比特无符号整型  |
| 6  | int32_t   | 32比特有符号整型  |
| 7  | uint32_t  | 32比特无符号整型  |
| 8  | int64_t   | 64比特有符号整型  |
| 9  | uint64_t  | 64比特无符号整型  |
| 10 | uintptr_t | 能够容纳指针类型长度的无符号整型 |
| 11 | intptr_t  | 能够容纳指针类型长度的有符号整型 |

* 表2-自定义关键字

| 序号  | 关键字名称  | 说明  |
| -------------- | ------------ | ------------ |
| 1  | true   | bool值：真。如果目标平台无此定义，可宏定义： #define true (1)  |
| 2  | false  | bool值：假。如果目标平台无此定义，可宏定义： #define false (0) |

* 此部分定义在源文件：$(SDK_PATH)/src/platform/os/aliot_platform_datatype.h。
* 请根据目标平台情况实现，实现请放置于源文件：$(SDK_PATH)/src/platform/aliot_platform_datatype.h中。

*注： SDK所定义的数据类型是C99标准所定义的数据类型的一部分，如果目标硬件平台完全支持C99标准，则无需修改此部分代码即可满足于目标平台。*

### OS（硬件）接口

* 表3-OS相关接口说明

| 序号  | 接口名称  | 说明  |
| -------------- | ------------ | ------------ |
| 1  | aliot_platform_malloc        | 分配内存块  |
| 2  | aliot_platform_free          | 释放内存块  |
| 3  | aliot_platform_time_get_ms   | 获取系统时间（单位：ms），允许溢出  |
| 4  | aliot_platform_printf        | 格式化输出 |
| 5  | aliot_platform_ota_start     | 启动OTA，由于暂不支持OTA功能，该接口暂无需实现  |
| 6  | aliot_platform_ota_write     | 写OTA固件，由于暂不支持OTA功能，该接口暂无需实现 |
| 7  | aliot_platform_ota_finalize  | 完成OTA，由于暂不支持OTA功能，该接口暂无需实现  |
| 8  | aliot_platform_msleep        | 睡眠指定时间，如果是无OS的平台，将函数实现为延时指定时间即可 |
| 9  | aliot_platform_mutex_create  | 创建互斥锁，如果是无OS的平台，无需实现该接口 |
| 10 | aliot_platform_mutex_destroy | 销毁互斥锁，如果是无OS的平台，无需实现该接口 |
| 11 | aliot_platform_mutex_lock    | 锁住指定互斥锁，如果是无OS的平台，无需实现该接口 |
| 12 | aliot_platform_mutex_unlock  | 释放指定互斥锁，如果是无OS的平台，无需实现该接口 |
| 13 | aliot_platform_module_get_pid| 该接口仅用于特定场景，若无涉及，返回NULL即可 |

* 详细的接口输入输出说明请参考源文件：（$(SDK_PATH)/src/platform/os/aliot_platform_os.h）。
* 实现时，请在路径$(SDK_PATH)/src/platform/os/下创建一个文件夹（请注意这个文件夹名，后续编译将用到这个名字），相应移植实现放置在该文件夹下。


***注：如果是无OS平台，所有面向应用的接口都不能被并发调用（包括在中断服务程序中调用）***


### TCPIP网络接口

* 表4-TCPIP网络相关接口说明

| 序号  | 接口名称  | 说明  |
| -------------- | ------------ | ------------ |
| 1  | aliot_platform_tcp_establish | 建立tcp连接，返回连接句柄 |
| 2  | aliot_platform_tcp_destroy   | 释放一个tcp连接 |
| 3  | aliot_platform_tcp_write     | 往TCP通道写入数据。注意实现超时参数 |
| 4  | aliot_platform_tcp_read      | 从TCP通道读取数据。注意实现超时参数 |

* 详细的接口输入输出说明请参考源文件：（$(SDK_PATH)/src/platform/network/aliot_platform_network.h）。
* 实现时，请在路径$(SDK_PATH)/src/platform/network/下创建一个文件夹（请注意这个文件夹名，后续编译将用到这个名字），相应移植实现放置在该文件夹下。


### SSL接口

* 表5-SSL相关接口说明

| 序号  | 接口名称  | 说明  |
| -------------- | ------------ | ------------ |
| 1  | aliot_platform_ssl_establish | 建立经SSL加密的传输通道 |
| 2  | aliot_platform_ssl_destroy   | 释放一个SSL通道 |
| 3  | aliot_platform_ssl_write     | 往SSL通道写入数据。注意实现超时参数 |
| 4  | aliot_platform_ssl_read      | 从SSL通道读取数据。注意实现超时参数 |

* 详细的接口输入输出说明请参考源文件：（$(SDK_PATH)/src/platform/ssl/aliot_platform_ssl.h）。
* 实现时，请在路径$(SDK_PATH)/src/platform/ssl/下创建一个文件夹（请注意这个文件夹名，后续编译将用到这个名字），相应移植实现放置在该文件夹下。


## 集成编译
平台移植工作完成之后，修改make.settings文件的相关配置项即可进行编译，具体如下：

    PLATFORM_OS        = linux     //OS文件夹名
    PLATFORM_NETWORK   = linuxsock //network文件夹名
    PLATFORM_SSL       = mbedtls   //SSL文件夹名

完成配置后，执行make，即可编译目标平台的代码。

## 移植样例
请参考SDK代码中提供的平台适配代码。

