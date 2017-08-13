**以下详细描述如何将华东2节点设备端V2.0版本C-SDK移植到目标硬件平台.**

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

