# <a name="目录">目录</a>
+ [设备端C-SDK概述](#设备端C-SDK概述)
+ [硬件平台抽象层(HAL层)](#硬件平台抽象层(HAL层))
    * [必选实现](#必选实现)
    * [多线程相关](#多线程相关)
    * [MQTT和HTTP通道功能需要](#MQTT和HTTP通道功能需要)
    * [CoAP通道功能需要](#CoAP通道功能需要)
    * [本地通信功能需要](#本地通信功能需要)
    * [设备信息设置与获取需要](#设备信息设置与获取需要)
    * [OTA功能需要](#OTA功能需要)
    * [本地定时任务功能需要](#本地定时任务功能需要)
    * [WIFI配网功能需要](#WIFI配网功能需要)

# <a name="设备端C-SDK概述">设备端C-SDK概述</a>

        +---------------------------+
        |                           |
        |  Link Kit Example Program |
        |                           |
        +---------------------------+
        |                           |
        |  Link Kit Interface Layer |
        |                           |
        |       IOT_XXX_YYY() APIs  |
        |   linkkit_mmm_nnn() APIs  |
        |                           |
        +---------------------------+
        |                           |
        |  Link Kit Core Implements |
        |  : =>                     |
        |  : You SHOULD NOT Focus   |
        |  : on this unless         |
        |  : you're debugging bugs  |
        |                           |
        +---------------------------+
        |                           |
        |  Hardware Abstract Layer  |
        |                           |
        |     HAL_XXX_YYY() APIs    |
        |                           |
        |  : You MUST Implement     |
        |  : this part for your     |
        |  : target device first    |
        +---------------------------+

设备端C-SDK是分为三层的, 如上图

+ 最底层称为"硬件平台抽象层", 也简称`HAL层`, 对应上图的 `Hardware Abstract Layer`
    - 这里是抽象不同的嵌入式目标板上操作系统对我们SDK的支撑函数
    - 举例来说, 包括网络收发, TLS/DTLS通道建立和读写, 内存申请释放, 互斥量加锁解锁等
    - **注意: 在任何跨平台移植时, 实现这部分是需要完成的第一步工作**
    - **注意: 阿里的SDK里, 并不含有多平台的HAL层实现, 但我们提供了Linux桌面OS(Ubuntu16.04)上的参考实现, 移植时可以作为参考**

+ 中间层称为"SDK内核实现层", 对应上图的 `Link Kit Core Implements`
    - 这里是C-SDK的核心实现部分, 它基于HAL层接口完成了MQTT/CoAP通道等的功能封装
    - 举例来说, 包括MQTT的连接建立, 报文收发, CoAP的连接建立, 报文收发, OTA的固件状态查询, OTA的固件下载等
    - **注意: 如果HAL层实现的好, 这一层在跨平台移植时, 理想情况下不需要做任何修改**

+ 最上层称为"SDK接口声明层", 对应上图的 `Link Kit Interface Layer`
    - 这里没有实现, 只有一系列C函数的原型声明, 也就是SDK跨平台移植完成之后, 可以用于编写业务逻辑, 和阿里云服务器通信的API
    - 举例来说, 怎么去使用这些API做业务逻辑, 我们在`examples`目录提供了丰富的示例程序, 并且只要填入了设备信息, 就可以在Linux主机上运行体验

以下按照从下到上的顺序, 逐个对每个层次做更加详细的说明

# <a name="硬件平台抽象层(HAL层)">硬件平台抽象层(HAL层)</a>

+ 所有HAL层函数的声明都在 `include/iot_import.h` 这个头文件中列出
+ 各功能点引入的HAL层接口依赖在`include/imports/iot_import_*.h`中列出
+ 这些`include/imports`目录下的子文件, 都被`include/iot_import.h`包含
+ 这个部分的函数声明, 对应在`src/ref-impl/hal/os/ubuntu/`有提供一份`Ubuntu`桌面系统上的参考实现, 会被编译成`output/release/lib/libiot_hal.a`

以下可以列出所有跨平台移植时需要实现的HAL层接口

    include$ grep -ro "HAL_[_A-Za-z0-9]*" *|cut -d':' -f2|sort -u|cat -n

     1  HAL_Aes128_Cbc_Decrypt
     2  HAL_Aes128_Cbc_Encrypt
     3  HAL_Aes128_Cfb_Decrypt
     4  HAL_Aes128_Cfb_Encrypt
     ...

对这些函数做实现的时候, 可以参考`src/ref-impl/hal/os/ubuntu`下和`src/ref-impl/hal/os/win7`下已经写好的示例, 这些示例在`Ubuntu16.04`主机和`Win7`主机上被完善的编写和测试过

    src/ref-impl/hal$ tree -A
    .
    +-- CMakeLists.txt
    +-- iot.mk
    +-- iotx_hal_internal.h
    +-- os
    |   +-- macos
    |   |   +-- HAL_Crypt_MacOS.c
    |   |   +-- HAL_OS_MacOS.c
    |   |   +-- HAL_TCP_MacOS.c
    |   |   +-- HAL_UDP_MacOS.c
    |   +-- ubuntu
    |   |   +-- base64.c
    |   |   +-- base64.h
    |   |   +-- cJSON.c
    |   |   +-- cJSON.h
    |   |   +-- HAL_Crypt_Linux.c
    |   |   +-- HAL_OS_linux.c
    |   |   +-- HAL_TCP_linux.c
    |   |   +-- HAL_UDP_linux.c
    |   |   +-- kv.c
    |   |   +-- kv.h
    |   +-- win7
    |       +-- HAL_OS_win7.c
    |       +-- HAL_TCP_win7.c
    +-- ssl
        +-- itls
        |   +-- HAL_TLS_itls.c
        +-- mbedtls
        |   +-- HAL_DTLS_mbedtls.c
        |   +-- HAL_TLS_mbedtls.c
        +-- openssl
            +-- HAL_TLS_openssl.c

以下是这些函数的一个说明表格, 更多函数的详细信息, 请查阅代码中的注释

在这些HAL接口中

## <a name="必选实现">必选实现</a>

| 函数名                   | 说明                                                                    |
|--------------------------|-------------------------------------------------------------------------|
| HAL_Malloc               | 申请一片堆上内存                                                        |
| HAL_Free                 | 释放一片堆上内存                                                        |
| HAL_SleepMs              | 睡眠函数, 使当前执行线程睡眠指定的毫秒数                                |
| HAL_Snprintf             | 打印函数, 向内存缓冲区格式化构建一个字符串, 参考C99标准库函数`snprintf` |
| HAL_Printf               | 打印函数, 用于向串口或其它标准输出打印日志或调试信息                    |
| HAL_Vsnprintf            | 字符串打印函数, 将`va_list`类型的变量, 打印到指定目标字符串             |
| HAL_UptimeMs             | 时钟函数, 获取本设备从加电以来到目前时间点已经过去的毫秒数              |

## <a name="多线程相关">多线程相关</a>

| 函数名                   | 说明                                                                           |
|--------------------------|--------------------------------------------------------------------------------|
| HAL_MutexCreate          | 创建一个互斥量, 用于同步控制, 对于仅支持单线程应用, 可实现为空函数             |
| HAL_MutexDestroy         | 销毁一个互斥量, 用于同步控制, 对于仅支持单线程应用, 可实现为空函数             |
| HAL_MutexLock            | 加锁一个互斥量, 用于同步控制, 对于仅支持单线程应用, 可实现为空函数             |
| HAL_MutexUnlock          | 解锁一个互斥量, 用于同步控制, 对于仅支持单线程应用, 可实现为空函数             |
| HAL_ThreadCreate         | 按照指定入参创建一个线程，对于仅支持单线程应用, 可实现为空函数                 |
| HAL_ThreadDelete         | 杀死指定的线程，对于仅支持单线程应用, 可实现为空函数                           |
| HAL_ThreadDetach         | 设置指定的线程为`Detach`状态，对于仅支持单线程应用, 可实现为空函数             |
| HAL_SemaphoreCreate      | 创建一个计数信号量，对于仅支持单线程应用, 可实现为空函数                       |
| HAL_SemaphoreDestroy     | 销毁一个计数信号量，对于仅支持单线程应用, 可实现为空函数                       |
| HAL_SemaphorePost        | 在指定的计数信号量上做自减操作并等待，对于仅支持单线程应用, 可实现为空函数     |
| HAL_SemaphoreWait        | 在指定的计数信号量上做自增操作, 解除其它线程的等待，对于仅支持单线程应用, 可实现为空函数 |

## <a name="MQTT和HTTP通道功能需要">MQTT和HTTP通道功能需要</a>

| 函数名                   | 说明                                                                    |
|--------------------------|-------------------------------------------------------------------------|
| HAL_SSL_Destroy          | 销毁一个TLS连接, 用于MQTT功能, HTTPS功能                                |
| HAL_SSL_Establish        | 建立一个TLS连接, 用于MQTT功能, HTTPS功能                                |
| HAL_SSL_Read             | 从一个TLS连接中读数据, 用于MQTT功能, HTTPS功能                          |
| HAL_SSL_Write            | 向一个TLS连接中写数据, 用于MQTT功能, HTTPS功能                          |
| HAL_TCP_Destroy          | 销毁一个TLS连接, 用于MQTT功能, HTTPS功能                                |
| HAL_TCP_Establish        | 建立一个TCP连接, 包含了域名解析的动作和TCP连接的建立                    |
| HAL_TCP_Read             | 在指定时间内, 从TCP连接读取流数据, 并返回读到的字节数                   |
| HAL_TCP_Write            | 在指定时间内, 向TCP连接发送流数据, 并返回发送的字节数                   |
| HAL_Random               | 随机数函数, 接受一个无符号数作为范围, 返回0到该数值范围内的随机无符号数 |
| HAL_Srandom              | 随机数播种函数, 使`HAL_Random`的返回值每个执行序列各不相同, 类似`srand` |

## <a name="CoAP通道功能需要">CoAP通道功能需要</a>

| 函数名                   | 说明                                                                    |
|--------------------------|-------------------------------------------------------------------------|
| HAL_DTLSSession_create   | 初始化DTLS资源并建立一个DTLS会话, 用于CoAP功能                          |
| HAL_DTLSSession_free     | 销毁一个DTLS会话并释放DTLS资源, 用于CoAP功能                            |
| HAL_DTLSSession_read     | 从DTLS连接中读数据, 用于CoAP功能                                        |
| HAL_DTLSSession_write    | 向DTLS连接中写数据, 用于CoAP功能                                        |
| HAL_Aes128_Cbc_Decrypt   | AES128解密，CBC模式，用于CoAP报文加解密                                 |
| HAL_Aes128_Cbc_Encrypt   | AES128加密，CBC模式，用于CoAP报文加解密                                 |
| HAL_Aes128_Cfb_Decrypt   | AES128解密，CFB模式，用于CoAP报文加解密                                 |
| HAL_Aes128_Cfb_Encrypt   | AES128加密，CFB模式，用于CoAP报文加解密                                 |
| HAL_Aes128_Destroy       | AES128反初始化                                                          |
| HAL_Aes128_Init          | AES128初始化                                                            |
| HAL_UDP_close            | 关闭一个UDP socket                                                      |
| HAL_UDP_create           | 创建一个UDP socket                                                      |
| HAL_UDP_read             | 阻塞的从一个UDP socket中读取数据包, 并返回读到的字节数                  |
| HAL_UDP_readTimeout      | 在指定时间内, 从一个UDP socket中读取数据包, 返回读到的字节数            |
| HAL_UDP_write            | 阻塞的向一个UDP socket中发送数据包, 并返回发送的字节数                  |

## <a name="本地通信功能需要">本地通信功能需要</a>

| 函数名                         | 说明                                                            |
|--------------------------------|-----------------------------------------------------------------|
| HAL_UDP_create_without_connect | 创建一个本地UDP socket，但不发起任何网络交互                    |  
| HAL_UDP_close_without_connect  | 销毁指定的UDP socket，回收资源                                  |
| HAL_UDP_joinmulticast          | 在指定的UDP socket上发送加入组播组的请求                        |
| HAL_UDP_sendto                 | 在指定的UDP socket上发送指定缓冲区的指定长度, 阻塞时间不超过指定时长, 且指定长度若发送完需提前返回  |
| HAL_UDP_recvfrom               | 从指定的UDP句柄接收指定长度数据到缓冲区, 阻塞时间不超过指定时长, 且指定长度若接收完需提前返回, 源地址保存在出参中 |
| HAL_Kv_Set                     | 写入指定KV数据                                                  |
| HAL_Kv_Get                     | 读取指定KV数据                                                  |
| HAL_Kv_Del                     | 删除指定KV数据                                                  |
| HAL_Kv_Erase_All               | 擦除所有的KV数据                                                |

## <a name="设备信息设置与获取需要">设备信息设置与获取需要</a>

| 函数名                   | 说明                                                                    |
|--------------------------|-------------------------------------------------------------------------|
| HAL_GetChipID            | 获取芯片ID                                                              |
| HAL_GetDeviceID          | 获取设备ID                                                              |
| HAL_GetFirmwareVesion    | 获取固件版本号，必须实现                                                |
| HAL_GetModuleID          | 获取模组ID，用于紧密合作伙伴, 一般客户只需要在此可实现为空函数          |
| HAL_GetPartnerID         | 获取合作伙伴ID，用于紧密合作伙伴, 一般客户只需要在此可实现为空函数      |
| HAL_GetDeviceName        | 获取DeviceName，三元组获取函数之一，必须实现                            |
| HAL_GetDeviceSecret      | 获取DeviceSecret，三元组获取函数之一，必须实现                          |
| HAL_GetProductKey        | 获取ProductKey，三元组获取函数之一，必须实现                            |
| HAL_GetProductSecret     | 获取ProductSecret，三元组获取函数之一，必须实现                         |
| HAL_SetDeviceName        | 设置DeviceName，三元组配置函数之一，必须实现                            |
| HAL_SetDeviceSecret      | 设置DeviceSecret，三元组配置函数之一，必须实现                          |
| HAL_SetProductKey        | 设置ProductKey, 三元组配置函数之一，必须实现                            |
| HAL_SetProductSecret     | 设置ProductSecret，三元组配置函数之一，必须实现                         |

## <a name="OTA功能需要">OTA功能需要</a>

| 函数名                             | 说明                                                        |
|-----------------------------------|--------------------------------------------------------------|
| HAL_Firmware_Persistence_Start    | 固件持久化开始，包含OTA功能时必须实现                        |
| HAL_Firmware_Persistence_Stop     | 固件持久化结束，包含OTA功能时必须实现                        |
| HAL_Firmware_Persistence_Write    | 固件持久化写入，包含OTA功能时必须实现                        |

## <a name="本地定时任务功能需要">本地定时任务功能需要</a>

| 函数名                             | 说明                                                        |
|-----------------------------------|--------------------------------------------------------------|
| HAL_UTC_Set                       | 设置UTC时间，单位ms                                          |
| HAL_UTC_Get                       | 获取UTC时间，单位ms                                          |


## <a name="WIFI配网功能需要">WIFI配网功能需要</a>
请查看`include/imports/iot_import_awss.h`
