# <a name="目录">目录</a>
+ [设备端SDK的编译](#设备端SDK的编译)
    * [在Ubuntu上编译主机版本](#在Ubuntu上编译主机版本)
        - [正常的编译过程演示](#正常的编译过程演示)
        - [得到的编译产物说明](#得到的编译产物说明)
    * [交叉编译到嵌入式硬件平台](#交叉编译到嵌入式硬件平台)
        - [0. 安装交叉编译工具链](#0. 安装交叉编译工具链)
        - [1. 添加配置文件](#1. 添加配置文件)
        - [2. 编辑配置文件, 设置编译选项和工具链, 以及跳过编译的目录](#2. 编辑配置文件, 设置编译选项和工具链, 以及跳过编译的目录)
        - [3. 选择配置文件](#3. 选择配置文件)
        - [4. 交叉编译产生库文件`libiot_sdk.a`](#4. 交叉编译产生库文件`libiot_sdk.a`)
        - [5. 获取交叉编译的产物, 包括静态库和头文件](#5. 获取交叉编译的产物, 包括静态库和头文件)
    * [开发未适配平台的HAL层](#开发未适配平台的HAL层)
        - [1. 在`src/ref-impl/hal/`目录下复制一份HAL层实现代码](#1. 在`src/ref-impl/hal/`目录下复制一份HAL层实现代码)
        - [2. 打开之前被关闭的`src/ref-impl/hal`编译开关](#2. 打开之前被关闭的`src/ref-impl/hal`编译开关)
        - [3. 尝试编译被复制的HAL层代码](#3. 尝试编译被复制的HAL层代码)
        - [4. 编辑配置文件, 尝试产生样例程序](#4. 编辑配置文件, 尝试产生样例程序)
        - [5. 重新载入配置文件, 编译可执行程序](#5. 重新载入配置文件, 编译可执行程序)
        - [6. 尝试运行样例程序](#6. 尝试运行样例程序)

# <a name="设备端SDK的编译">设备端SDK的编译</a>

设备端SDK目前支持以下的编译方式:

- 在`Linux`上或者`MacOS`上以`GNU Make` + `各种工具链`编译, 产生`各种目标架构`的SDK
- 在`Linux`上或者`Windows`上或者`MacOS`上以`CMake` + `Mingw32`编译`, 产生Windows架构`的SDK
- 在`Windows`上以`VS Code 2017`编译, 产生`Windows架构`的SDK
- 在`Windows`上以`QT Creator`编译, 产生`Windows架构`的SDK
- 在`Windows`上以`Visual Studio 2017`编译, 产生`Windows架构`的SDK

## <a name="在Ubuntu上编译主机版本">在Ubuntu上编译主机版本</a>

具体步骤是:

    $ make distclean
    $ make

即可得到`libiot_sdk.a`

### <a name="正常的编译过程演示">正常的编译过程演示</a>

    $ make distclean
    $ make
    SELECTED CONFIGURATION:

    VENDOR :   ubuntu
    MODEL  :   x86


    CONFIGURE .............................. [examples]
    CONFIGURE .............................. [src/infra/log]
    CONFIGURE .............................. [src/infra/system]
    CONFIGURE .............................. [src/infra/utils]
    CONFIGURE .............................. [src/protocol/alcs]
    CONFIGURE .............................. [src/protocol/coap]
    CONFIGURE .............................. [src/protocol/http]
    CONFIGURE .............................. [src/protocol/http2]
    CONFIGURE .............................. [src/protocol/mqtt]
    CONFIGURE .............................. [src/ref-impl/hal]
    CONFIGURE .............................. [src/ref-impl/tls]
    CONFIGURE .............................. [src/sdk-impl]
    CONFIGURE .............................. [src/services/file_upload]
    CONFIGURE .............................. [src/services/linkkit/cm]
    CONFIGURE .............................. [src/services/linkkit/dm]
    CONFIGURE .............................. [src/services/shadow]
    CONFIGURE .............................. [src/services/subdev]
    CONFIGURE .............................. [src/services/uOTA]
    CONFIGURE .............................. [src/tools/linkkit_tsl_convert]
    CONFIGURE .............................. [tests]

    BUILDING WITH EXISTING CONFIGURATION:

    VENDOR :   ubuntu
    MODEL  :   x86

    Components: 

    . tests                                             
    . src/services/file_upload                                          
    . src/services/subdev                                          
    . src/services/uOTA                                          
    . src/services/shadow                                          
    . src/services/linkkit/dm                                          
    . src/services/linkkit/cm                                          
    . src/sdk-impl                                          
    . src/protocol/mqtt                                          
    . src/protocol/coap                                          
    . src/protocol/http2                                          
    . src/protocol/alcs                                          
    . src/protocol/http                                          
    . src/tools/linkkit_tsl_convert                                          
    . src/ref-impl/tls                                          
    . src/ref-impl/hal                                          
    . src/infra/log                                          
    . src/infra/system                                          
    . src/infra/utils                                          
    . examples                                          

    [CC] utils_list.o                       <=  ...
    [CC] json_parser.o                      <=  ...
    [CC] ota_service.o                      <=  ...
    [CC] sdk-impl.o                         <=  ...
    [CC] json_token.o                       <=  ...
    [CC] utils_event.o                      <=  ...
    [CC] utils_hmac.o                       <=  ...
    [CC] linked_list.o                      <=  ...
    [CC] string_utils.o                     <=  ...
    [CC] MQTTDeserializePublish.o           <=  ...
    [CC] utils_timer.o                      <=  ...
    [CC] MQTTUnsubscribeClient.o            <=  ...
    [CC] utils_sha256.o                     <=  ...
    [CC] mem_stats.o                        <=  ...
    [CC] iotx_log.o                         <=  ...
    [CC] lite-cjson.o                       <=  ...
    [CC] utils_net.o                        <=  ...
    [CC] utils_epoch_time.o                 <=  ...
    [CC] ota_download_coap.o                <=  ...
    [CC] lite_queue.o                       <=  ...
    [CC] MQTTSubscribeClient.o              <=  ...
    [CC] ota.o                              <=  ...
    [CC] ota_hal_os.o                       <=  ...
    [CC] ota_manifest.o                     <=  ...
    [CC] utils_md5.o                        <=  ...
    [CC] utils_httpc.o                      <=  ...
    [CC] ota_hal_plat.o                     <=  ...
    [CC] utils_sha1.o                       <=  ...
    [CC] mqtt_instance.o                    <=  ...
    [CC] MQTTPacket.o                       <=  ...
    [CC] guider.o                           <=  ...
    [CC] crc.o                              <=  ...
    [CC] utils_base64.o                     <=  ...
    [CC] alcs_client.o                      <=  ...
    [CC] MQTTConnectClient.o                <=  ...
    [CC] ota_socket.o                       <=  ...
    [CC] ota_hal_module.o                   <=  ...
    [CC] alcs_api.o                         <=  ...
    [CC] ca.o                               <=  ...
    [CC] CoAPSerialize.o                    <=  ...
    [CC] alcs_mqtt.o                        <=  ...
    [CC] ota_verify.o                       <=  ...
    [CC] mqtt_client.o                      <=  ...
    [CC] ota_transport_mqtt.o               <=  ...
    [CC] iotx_http_api.o                    <=  ...
    [CC] device.o                           <=  ...
    [CC] report.o                           <=  ...
    [CC] alcs_server.o                      <=  ...
    [CC] CoAPDeserialize.o                  <=  ...
    [CC] CoAPMessage.o                      <=  ...
    [CC] CoAPExport.o                       <=  ...
    [CC] CoAPNetwork.o                      <=  ...
    [CC] ota_version.o                      <=  ...
    [CC] CoAPServer.o                       <=  ...
    [CC] ota_util.o                         <=  ...
    [CC] ota_transport_coap.o               <=  ...
    [CC] CoAPPlatform.o                     <=  ...
    [CC] ota_download_http.o                <=  ...
    [CC] CoAPObserve.o                      <=  ...
    [CC] MQTTSerializePublish.o             <=  ...
    [CC] alcs_adapter.o                     <=  ...
    [CC] alcs_coap.o                        <=  ...
    [CC] CoAPResource.o                     <=  ...
    [CC] iotx_cm_api.o                      <=  ...
    [CC] iotx_cm_cloud_conn.o               <=  ...
    [CC] iotx_cm_common.o                   <=  ...
    [CC] iotx_cm_conn_coap.o                <=  ...
    [CC] iotx_cm_connectivity.o             <=  ...
    [CC] iotx_cm_conn_http.o                <=  ...
    [CC] iotx_cm_conn_mqtt.o                <=  ...
    [CC] dm_api.o                           <=  ...
    [CC] iotx_cm_log.o                      <=  ...
    [CC] iotx_cm_local_conn.o               <=  ...
    [CC] iotx_local_conn_alcs.o             <=  ...
    [CC] dm_cota.o                          <=  ...
    [CC] dm_cm_wrapper.o                    <=  ...
    [CC] dm_dispatch.o                      <=  ...
    [CC] dm_ipc.o                           <=  ...
    [CC] dm_fota.o                          <=  ...
    [CC] dm_conn.o                          <=  ...
    [CC] dm_message.o                       <=  ...
    [CC] dm_manager.o                       <=  ...
    [CC] dm_opt.o                           <=  ...
    [CC] dm_shadow.o                        <=  ...
    [CC] dm_subscribe.o                     <=  ...
    [CC] dm_message_cache.o                 <=  ...
    [CC] dm_ota.o                           <=  ...
    [CC] dm_utils.o                         <=  ...
    [CC] linkkit_gateway_legacy.o           <=  ...
    [CC] linkkit_solo_legacy.o              <=  ...
    [CC] dm_tsl_alink.o                     <=  ...
    [CC] base64.o                           <=  ...
    [CC] HAL_Crypt_Linux.o                  <=  ...
    [CC] HAL_OS_linux.o                     <=  ...
    [CC] HAL_TCP_linux.o                    <=  ...
    [CC] cJSON.o                            <=  ...
    [CC] HAL_DTLS_mbedtls.o                 <=  ...
    [CC] HAL_TLS_mbedtls.o                  <=  ...
    [CC] HAL_UDP_linux.o                    <=  ...
    [CC] pk_wrap.o                          <=  ...
    [CC] kv.o                               <=  ...
    [CC] platform.o                         <=  ...
    [CC] ctr_drbg.o                         <=  ...
    [CC] cipher_wrap.o                      <=  ...
    [CC] entropy.o                          <=  ...
    [CC] pk.o                               <=  ...
    [CC] error.o                            <=  ...
    [CC] sha1.o                             <=  ...
    [CC] oid.o                              <=  ...
    [CC] cipher.o                           <=  ...
    [CC] entropy_poll.o                     <=  ...
    [CC] pem.o                              <=  ...
    [CC] base64.o                           <=  ...
    [CC] asn1parse.o                        <=  ...
    [CC] rsa.o                              <=  ...
    [CC] md_wrap.o                          <=  ...
    [CC] ssl_tls.o                          <=  ...
    [CC] md.o                               <=  ...
    [CC] x509.o                             <=  ...
    [CC] x509_crt.o                         <=  ...
    [CC] sha256.o                           <=  ...
    [CC] net_sockets.o                      <=  ...
    [CC] ssl_ciphersuites.o                 <=  ...
    [CC] aes.o                              <=  ...
    [CC] ssl_cli.o                          <=  ...
    [CC] bignum.o                           <=  ...
    [CC] debug.o                            <=  ...
    [CC] pkparse.o                          <=  ...
    [CC] md5.o                              <=  ...
    [CC] timing.o                           <=  ...
    [CC] ssl_cookie.o                       <=  ...
    [CC] ssl.o                              <=  ...
    [CC] hash.o                             <=  ...
    [AR] libiot_hal.a                       <=  ...
    [AR] libiot_tls.a                       <=  ...
    [AR] libiot_sdk.a                       <=  ...
    [LD] linkkit-example-solo               <=  ...
    [LD] mqtt_multi_thread-example          <=  ...
    [LD] ota_mqtt-example                   <=  ...
    [LD] linkkit_tsl_convert                <=  ...
    [LD] mqtt-example                       <=  ...
    [LD] uota_app-example                   <=  ...
    [LD] sdk-testsuites                     <=  ...
    [LD] mqtt_rrpc-example                  <=  ...
    [LD] http-example                       <=  ...
    [LD] linkkit-example-sched              <=  ...

### <a name="得到的编译产物说明">得到的编译产物说明</a>

SDK编译的产物在编译成功之后都存放在 `output` 目录下:

    $ tree -A output/
    output/
    +-- release
        +-- bin
        |   +-- http-example
        |   +-- linkkit-example-sched
        |   +-- linkkit-example-solo
        |   +-- linkkit_tsl_convert
        |   +-- mqtt-example
        |   +-- mqtt_multi_thread-example
        |   +-- mqtt_rrpc-example
        |   +-- ota_mqtt-example
        |   +-- sdk-testsuites
        |   +-- uota_app-example
        +-- include
        |   +-- exports
        |   |   +-- iot_export_alcs.h
        |   |   +-- iot_export_coap.h
        |   |   +-- iot_export_errno.h
        |   |   +-- iot_export_event.h
        |   |   +-- iot_export_file_uploader.h
        |   |   +-- iot_export_http2.h
        |   |   +-- iot_export_http.h
        |   |   +-- iot_export_mqtt.h
        |   |   +-- iot_export_ota.h
        |   |   +-- iot_export_shadow.h
        |   |   +-- iot_export_subdev.h
        |   |   +-- linkkit_export.h
        |   |   +-- linkkit_gateway_export.h
        |   +-- imports
        |   |   +-- iot_import_awss.h
        |   |   +-- iot_import_coap.h
        |   |   +-- iot_import_config.h
        |   |   +-- iot_import_crypt.h
        |   |   +-- iot_import_dtls.h
        |   |   +-- iot_import_product.h
        |   +-- iot_export.h
        |   +-- iot_import.h
        +-- lib
            +-- libalicrypto.a
            +-- libid2client.a
            +-- libiot_hal.a
            +-- libiot_sdk.a
            +-- libiot_tls.a
            +-- libitls.a
            +-- libkm.a
            +-- libmbedcrypto.a
            +-- libplat_gen.a

可做说明为:

| 产物                                                  | 说明
|-------------------------------------------------------|----------------------------------------------------------------------------------
| output/release/bin/*                                  | 例子程序, 在Ubuntu上运行, 并对照阅读 `examples/` 目录下的源代码, 以体验SDK的功能
| output/release/include/iot_export.h                   | 集中存放了所有SDK向外界提供的顶层用户接口声明, 命名方式为`IOT_XXX_YYY()`和`linkkit_mmm_nnn()`, 并且它也包含了所有`exports/`目录下的文件
| output/release/include/exports/*.h                    | 按不同的子功能分开列出各个子功能所提供的用户接口声明, 比如`iot_export_coap.h`就列出的是SDK提供CoAP相关功能时的可用接口
| output/release/include/imports/iot_import_config.h    | 配置头文件, 集中存放SDK的伸缩属性的可配置项, 比如`CONFIG_MQTT_TX_MAXLEN`表示给MQTT上行报文可以开辟的最大内存缓冲区长度等
| output/release/include/iot_import.h                   | 集中存放了所有SDK依赖外界提供的底层支撑接口声明, 命名方式为`HAL_XXX_YYY()`, 并且它也包含了所有`imports/`目录下的文件
| output/release/include/imports/*.h                    | 按不同的子功能分开列出各个子功能特殊引入的HAL接口的声明, 比如`iot_import_awss.h`就列出的是SDK因为提供配网功能而需要的HAL接口
| output/lib/libiot_sdk.a                               | SDK主库, 集中提供了所有用户接口的实现, 它的上层是用户业务逻辑, 下层是`libiot_hal.a`
| output/lib/libiot_hal.a                               | HAL主库, 集中提供了所有`HAL_XXX_YYY()`接口的实现, 它的上层是`libiot_sdk.a`, 下层是`libiot_tls.a`
| output/lib/libiot_tls.a                               | TLS主库, 集中提供了所有`mbedtls_xxx_yyy()`接口的实现, 它的上层是`libiot_hal.a`
| output/lib/*.a                                        | 其它分库, 它们是从SDK源码目录的`prebuilt/`目录移动过来的, 主要提供一些闭源发布的功能, 比如`ID2`等

## <a name="交叉编译到嵌入式硬件平台">交叉编译到嵌入式硬件平台</a>

对于嵌入式硬件平台的情况, 对编译出目标平台的`libiot_sdk.a`, 需要经历如下几个步骤:

- 在`src/board/`目录下添加一个对应的配置文件, 文件名规范为`config.XXX.YYY`, 其中`XXX`部分就对应后面`src/ref-impl/hal/os/XXX`目录的HAL层代码
- 在配置文件中, 至少要指定交叉编译器`OVERRIDE_CC`, 交叉链接器`OVERRIDE_LD`, 静态库压缩器`OVERRIDE_AR`, 以及编译选项`CONFIG_ENV_CFLAGS`, 链接选项`CONFIG_ENV_LDFLAGS`等
- 尝试编译SDK, 对可能出现的跨平台问题进行修正, 直到成功产生目标格式的`libiot_sdk.a`
- 最后, 您需要以任何编译方式, 产生目标架构的`libiot_hal.a`, 若平台未适配, 则这个库对应的源代码在SDK中并未包含, 需要您根据[SDK跨平台移植](https://help.aliyun.com/document_detail/56047.html)页面自行实现`HAL_*()`接口

---
下面以某款目前未适配的`arm-linux`目标平台为例, 演示如何编译出该平台上可用的`libiot_sdk.a`

### <a name="0. 安装交叉编译工具链">0. 安装交叉编译工具链</a>

> 仍以Ubuntu16.04开发环境为例

    $ sudo apt-get install -y gcc-arm-linux-gnueabihf
    $ arm-linux-gnueabihf-gcc --version

    arm-linux-gnueabihf-gcc (Ubuntu/Linaro 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609
    Copyright (C) 2015 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

### <a name="1. 添加配置文件">1. 添加配置文件</a>

    $ touch src/board/config.arm-linux.demo
    $ ls src/board/
    config.arm-linux.demo  config.macos.make  config.rhino.make  config.ubuntu.x86  config.win7.mingw32

### <a name="2. 编辑配置文件, 设置编译选项和工具链, 以及跳过编译的目录">2. 编辑配置文件, 设置编译选项和工具链, 以及跳过编译的目录</a>

    $ vim src/board/config.arm-linux.demo

    CONFIG_ENV_CFLAGS = \
        -D_PLATFORM_IS_LINUX_ \
        -DCM_VIA_CLOUD_CONN \
        -Wall

    OVERRIDE_CC = arm-linux-gnueabihf-gcc
    OVERRIDE_AR = arm-linux-gnueabihf-ar
    OVERRIDE_LD = arm-linux-gnueabihf-ld

    CONFIG_src/ref-impl/hal         :=
    CONFIG_examples                 :=
    CONFIG_tests                    :=
    CONFIG_src/tools/linkkit_tsl_convert :=

注意, 倒数4行表示对`src/ref-impl/hal`, `examples`, `tests`, `src/tools/linkkit_tsl_convert`这些目录跳过编译, 在编译未被适配平台的库时, 这在最初是必要的

### <a name="3. 选择配置文件">3. 选择配置文件</a>

    $ make reconfig
    SELECT A CONFIGURATION:

    1) config.arm-linux.demo  3) config.rhino.make      5) config.win7.mingw32
    2) config.macos.make      4) config.ubuntu.x86
    #? 1

    SELECTED CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo

    CONFIGURE .............................. [examples]
    CONFIGURE .............................. [src/infra/log]
    CONFIGURE .............................. [src/infra/system]
    CONFIGURE .............................. [src/infra/utils]
    CONFIGURE .............................. [src/protocol/alcs]
    CONFIGURE .............................. [src/protocol/coap]
    CONFIGURE .............................. [src/protocol/http]
    CONFIGURE .............................. [src/protocol/http2]
    CONFIGURE .............................. [src/protocol/mqtt]
    CONFIGURE .............................. [src/ref-impl/hal]
    CONFIGURE .............................. [src/ref-impl/tls]
    CONFIGURE .............................. [src/sdk-impl]
    CONFIGURE .............................. [src/services/file_upload]
    CONFIGURE .............................. [src/services/linkkit/cm]
    CONFIGURE .............................. [src/services/linkkit/dm]
    CONFIGURE .............................. [src/services/shadow]
    CONFIGURE .............................. [src/services/subdev]
    CONFIGURE .............................. [src/services/uOTA]
    CONFIGURE .............................. [src/tools/linkkit_tsl_convert]
    CONFIGURE .............................. [tests]

    BUILDING WITH EXISTING CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo

    Components: 

    . tests                                             
    . src/services/file_upload                                          
    . src/services/subdev                                          
    . src/services/uOTA                                          
    . src/services/shadow                                          
    . src/services/linkkit/dm                                          
    . src/services/linkkit/cm                                          
    . src/sdk-impl                                          
    . src/protocol/mqtt                                          
    . src/protocol/coap                                          
    . src/protocol/http2                                          
    . src/protocol/alcs                                          
    . src/protocol/http                                          
    . src/tools/linkkit_tsl_convert                                          
    . src/ref-impl/tls                                          
    . src/ref-impl/hal                                          
    . src/infra/log                                          
    . src/infra/system                                          
    . src/infra/utils                                          
    . examples     

### <a name="4. 交叉编译产生库文件`libiot_sdk.a`">4. 交叉编译产生库文件`libiot_sdk.a`</a>

    $ make
    BUILDING WITH EXISTING CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo

    [CC] guider.o                           <=  ...
    [CC] utils_epoch_time.o                 <=  ...
    [CC] iotx_log.o                         <=  ...
    [CC] lite_queue.o                       <=  ...
    ...
    ...
    [AR] libiot_sdk.a                       <=  ...
    [AR] libiot_tls.a                       <=  ...

### <a name="5. 获取交叉编译的产物, 包括静态库和头文件">5. 获取交叉编译的产物, 包括静态库和头文件</a>

    $ ls -1 output/release/lib/
    libiot_sdk.a
    libiot_tls.a

这里, `libiot_sdk.a`文件就是编译好的物联网套件SDK, 已经是`ELF 32-bit LSB relocatable, ARM, EABI5 version 1 (SYSV)`格式, 也就是`arm-linux`格式的交叉编译格式了

另外, `libiot_tls.a`是一个裁剪过的加解密库, 您可以选择使用它, 也可以选择使用平台自带的加解密库, 以减小最终固件的尺寸

    $ ls -1 output/release/include/
    exports
    imports
    iot_export.h
    iot_import.h

这里, `iot_import.h`和`iot_export.h`就是使用SDK需要包含的头文件, 它们按功能点又包含不同的子文件, 分别列在`imports/`目录下和`exports/`目录下

## <a name="开发未适配平台的HAL层">开发未适配平台的HAL层</a>

对于平台抽象层的库`libiot_hal.a`, 虽然说不指定任何编译和产生的方式, 但是如果你愿意的话, 当然仍然可以借助物联网套件设备端SDK的编译系统来开发和产生它

---
仍然以上一节中, 某款目前未适配的`arm-linux`目标平台为例, 假设这款平台和`Ubuntu`差别很小, 完全可以用`Ubuntu`上开发测试的HAL层代码作为开发的基础, 则可以这样做:

### <a name="1. 在`src/ref-impl/hal/`目录下复制一份HAL层实现代码">1. 在`src/ref-impl/hal/`目录下复制一份HAL层实现代码</a>

    $ cd src/ref-impl/hal/os/
    $ ls
    macos  ubuntu  win7
    src/ref-impl/hal/os$ cp -rf ubuntu arm-linux
    src/ref-impl/hal/os$ ls
    arm-linux  macos  ubuntu  win7

    src/ref-impl/hal/os$ tree -A arm-linux/
    arm-linux/
    +-- base64.c
    +-- base64.h
    +-- cJSON.c
    +-- cJSON.h
    +-- HAL_Crypt_Linux.c
    +-- HAL_OS_linux.c
    +-- HAL_TCP_linux.c
    +-- HAL_UDP_linux.c
    +-- kv.c
    +-- kv.h

### <a name="2. 打开之前被关闭的`src/ref-impl/hal`编译开关">2. 打开之前被关闭的`src/ref-impl/hal`编译开关</a>

    $ vim src/board/config.arm-linux.demo

    CONFIG_ENV_CFLAGS = \
        -D_PLATFORM_IS_LINUX_ \
        -DCM_VIA_CLOUD_CONN \
        -Wall

    OVERRIDE_CC = arm-linux-gnueabihf-gcc
    OVERRIDE_AR = arm-linux-gnueabihf-ar
    OVERRIDE_LD = arm-linux-gnueabihf-ld

    # CONFIG_src/ref-impl/hal         :=
    CONFIG_examples                 :=
    CONFIG_tests                    :=
    CONFIG_src/tools/linkkit_tsl_convert :=

可以看到在`CONFIG_src/ref-impl/hal :=`这一行前添加了一个`#`符号, 代表这一行被注释掉了, 效果等同于被删掉, `src/ref-impl/hal`将会进入编译过程

### <a name="3. 尝试编译被复制的HAL层代码">3. 尝试编译被复制的HAL层代码</a>

    $ make reconfig
    SELECT A CONFIGURATION:

    1) config.arm-linux.demo  3) config.rhino.make      5) config.win7.mingw32
    2) config.macos.make      4) config.ubuntu.x86
    #? 1

    SELECTED CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo
    ...
    ...

    $ make
    [CC] utils_md5.o                        <=  ...
    [CC] utils_event.o                      <=  ...
    [CC] string_utils.o                     <=  ...
    ...
    ...
    [AR] libiot_sdk.a                       <=  ...
    [AR] libiot_hal.a                       <=  ...
    [AR] libiot_tls.a                       <=  ...

可以看到我们进展的确实十分顺利, 被复制的代码`src/ref-impl/hal/os/arm-linux/*.c`确实直接编译成功了, 产生了`arm-linux`格式的`libiot_hal.a`

### <a name="4. 编辑配置文件, 尝试产生样例程序">4. 编辑配置文件, 尝试产生样例程序</a>

这样有了`libiot_hal.a`, `libiot_tls.a`, 以及`libiot_sdk.a`, 其实以及可以尝试产生样例的可执行程序, 并在目标嵌入式硬件开发板上运行一下试试了

方法和上一步一样, 打开`config.arm-linux.demo`里面的`CONFIG_example`开关, 使得`examples/`目录下的样例源码被编译出来

    $ vi src/board/config.arm-linux.demo

    CONFIG_ENV_CFLAGS = \
        -D_PLATFORM_IS_LINUX_ \
        -DCM_VIA_CLOUD_CONN \
        -Wall

    CONFIG_ENV_LDFLAGS = \
        -lpthread -lrt

    OVERRIDE_CC = arm-linux-gnueabihf-gcc
    OVERRIDE_AR = arm-linux-gnueabihf-ar
    OVERRIDE_LD = arm-linux-gnueabihf-ld

    # CONFIG_src/ref-impl/hal         :=
    # CONFIG_examples                 :=
    CONFIG_tests                    :=
    CONFIG_src/tools/linkkit_tsl_convert :=

可以看到在`CONFIG_examples =`这一行前添加了一个`#`符号, 代表这一行被注释掉了, 效果等同于被删掉, `examples/`目录也就是例子可执行程序进入了编译范围

另外一点需要注意的改动是增加了:

    CONFIG_ENV_LDFLAGS = \
        -lpthread -lrt

部分, 这是因为产生这些样例程序除了链接`libiot_hal.a`和`libiot_hal.a`之外, 还需要连接`libpthread`库和`librt`库

### <a name="5. 重新载入配置文件, 编译可执行程序">5. 重新载入配置文件, 编译可执行程序</a>

    $ make reconfig
    SELECT A CONFIGURATION:

    1) config.arm-linux.demo  3) config.rhino.make      5) config.win7.mingw32
    2) config.macos.make      4) config.ubuntu.x86
    #? 1

    SELECTED CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo
    ...

需要注意, 编译样例程序, 需要用:

    make all

命令, 而不再是上面的

    make

命令来产生编译产物, 比如:

    $ make all
    BUILDING WITH EXISTING CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo

    [CC] base64.o                           <= base64.c                                    
    [CC] cJSON.o                            <= cJSON.c                                    
    [CC] HAL_UDP_linux.o                    <= HAL_UDP_linux.c          
    ...
    ...
    [LD] mqtt_rrpc-example                  <= mqtt_rrpc-example.o                                    
    [LD] uota_app-example                   <= uota_app-example.o                                    
    [LD] http-example                       <= http-example.o                                    
    [LD] mqtt-example                       <= mqtt-example.o                                    
    [LD] mqtt_multi_thread-example          <= mqtt_multi_thread-example.o                                    
    [LD] ota_mqtt-example                   <= ota_mqtt-example.o                                    
    [LD] linkkit-example-sched              <= linkkit_example_sched.o                                    
    [LD] linkkit-example-solo               <= linkkit_example_solo.o     

如果有如上的编译输出, 则代表`mqtt-example`等一系列样例程序已经被成功的编译出来, 它们存放在`output/release/bin`目录下

    $ cd output/release/bin/
    $ ls
    http-example  linkkit-example-sched  linkkit-example-solo  mqtt-example  mqtt_multi_thread-example  mqtt_rrpc-example  ota_mqtt-example  uota_app-example

    $ file *
    http-example:              ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, ...
    linkkit-example-sched:     ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, ...
    linkkit-example-solo:      ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, ...
    mqtt-example:              ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, ...
    mqtt_multi_thread-example: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, ...
    mqtt_rrpc-example:         ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, ...
    ota_mqtt-example:          ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, ...
    uota_app-example:          ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, ...

可以用`file`命令验证, 这些可执行程序确实是交叉编译到`arm-linux`架构上的

### <a name="6. 尝试运行样例程序">6. 尝试运行样例程序</a>

接下来, 您就可以把样例程序例如`mqtt-example`, 用`SCP`, `TFTP`或者其它方式, 拷贝下载到您的目标开发板上运行调试了

- 如果一切顺利, 则证明`src/ref-impl/hal/os/arm-linux`部分的HAL层代码工作正常
- 如果样例程序运行有问题, 则需要再重点修改调试下`src/ref-impl/hal/os/arm-linux`部分的HAL层代码, 因为这些代码是我们从`Ubuntu`主机部分复制的, 完全可能并不适合`arm-linux`

如此反复直到确保`libiot_hal.a`的开发没问题为止
