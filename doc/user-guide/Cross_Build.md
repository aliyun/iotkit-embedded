# 设备端SDK的编译

设备端SDK目前支持以下的编译方式:

- 在`Linux`上以`GNU Make` + `各种工具链`编译, 产生`各种目标架构`的SDK

后续计划支持更多的编译方式, 如:

- 在`Linux`上或者`Windows`上以`CMake` + `Mingw32`编译`, 产生Windows架构`的SDK
- 在`Linux`上或者`Windows`上以`QT Creator`编译, 产生`Windows架构`的SDK
- 在`Linux`上或者`Windows`上以`Visual Studio 2017`编译, 产生`Windows架构`的SDK


## 在Ubuntu上编译主机版本

在[README.md](https://github.com/aliyun/iotkit-embedded/blob/master/README.md)中可以看到如何在`Ubuntu 16.04`主机上编译主机版本的SDK, 也就是产生`X86-64`文件格式的`output/release/lib/libiot_sdk.a`

具体步骤是:

    $ make distclean
    $ make

得到`libiot_sdk.a`之后, 在`output/release/src/`目录下, 有个简单的示意用`Makefile`, 演示了如何将SDK中编译好的静态库在外部的编译方式下使用, 产生应用程序, 如

    output/release/src$ make
    gcc -I../include -I../include/iot-sdk -I../include/mbedtls -o ext.mqtt mqtt-example.c -L../lib -liot_sdk -liot_platform -Bstatic -liot_tls -lgcov
    gcc -I../include -I../include/iot-sdk -I../include/mbedtls -o ext.coap coap-example.c -L../lib -liot_sdk -liot_platform -Bstatic -liot_tls -lgcov
    gcc -I../include -I../include/iot-sdk -I../include/mbedtls -o ext.http http-example.c -L../lib -liot_sdk -liot_platform -Bstatic -liot_tls -lgcov

    output/release/src$ ./ext.mqtt
    [inf] iotx_device_info_init(40): device_info created successfully!
    [dbg] iotx_device_info_set(50): start to set device info!
    [dbg] iotx_device_info_set(64): device_info set successfully!
    ...

## 交叉编译到已适配硬件平台

对于已经适配好硬件平台的情况, 可以直接编译产生目标平台架构的`libiot_sdk.a`

以[芯讯通无线科技](https://market.aliyun.com/store/2962046-0.html)出品, 基于高通`MDM9206`芯片的`NBIoT/GPRS`解决方案[SIM7000C](https://market.aliyun.com/products/201200001/cmgj022417.html)开发板为例

    $ cp -f doc/config.mdm9206.sim7000c src/board/

    $ make reconfig
    SELECT A CONFIGURATION:

    1) config.mdm9206.sim7000c  3) config.win7.mingw32
    2) config.ubuntu.x86
    #? 1

    SELECTED CONFIGURATION:

    VENDOR :   mdm9206
    MODEL  :   sim7000c


    CONFIGURE .............................. [sample]
    CONFIGURE .............................. [src/coap]
    CONFIGURE .............................. [src/guider]
    CONFIGURE .............................. [src/http]
    CONFIGURE .............................. [src/log]
    CONFIGURE .............................. [src/mqtt]
    CONFIGURE .............................. [src/ota]
    CONFIGURE .............................. [src/platform]
    CONFIGURE .............................. [src/sdk-impl]
    CONFIGURE .............................. [src/sdk-tests]
    CONFIGURE .............................. [src/shadow]
    CONFIGURE .............................. [src/system]
    CONFIGURE .............................. [src/tfs]
    CONFIGURE .............................. [src/tls]
    CONFIGURE .............................. [src/utils]

    $ make
    ...

    $ ls output/release/lib/
    libiot_sdk.a

以上产生的`output/release/lib`就已经是可以直接用于`SIM7000C`开发板的二进制文件了, 具体如何使用, 可以参考页面[Platform SIM7000C](https://github.com/aliyun/iotkit-embedded/wiki/Platform-SIM7000C)

## 交叉编译到未适配硬件平台

对于未适配好硬件平台的情况, 对编译出目标平台的`libiot_sdk.a`, 需要经历如下几个步骤:

- 在`src/board/`目录下添加一个对应的配置文件, 文件名规范为`config.XXX.YYY`, 其中`XXX`部分就对应后面`src/ref-impl/hal/os/XXX`目录的HAL层代码
- 在配置文件中, 至少要指定交叉编译器`OVERRIDE_CC`, 静态库压缩器`OVERRIDE_AR`, 以及编译选项`CONFIG_ENV_CFLAGS`等
- 尝试编译SDK, 对可能出现的跨平台问题进行修正, 直到成功产生`libiot_sdk.a`
- 最后, 您需要以任何编译方式, 产生目标架构的`libiot_platform.a`, 若平台未适配, 则这个库对应的源代码在SDK中并未包含, 需要您根据[SDK跨平台移植](https://help.aliyun.com/document_detail/56047.html)页面自行实现`HAL_*()`接口

---
下面以某款目前未适配的`arm-linux`目标平台为例, 演示如何编译出该平台上可用的`libiot_sdk.a`

#### 1. 添加配置文件

    $ touch src/board/config.arm-linux.demo
    $ ls src/board/
    config.arm-linux.demo  config.mdm9206.sim7000c  config.ubuntu.x86  config.win7.mingw32  default_settings.mk

#### 2. 编辑配置文件, 设置编译选项和工具链, 以及跳过编译的目录

    $ vim src/board/config.arm-linux.demo

      1 CONFIG_ENV_CFLAGS = -Wall
      2
      3 OVERRIDE_CC = arm-linux-gnueabi-gcc-5
      4 OVERRIDE_AR = arm-linux-gnueabi-gcc-ar-5
      5
      6 CONFIG_src/ref-impl/hal         :=
      7 CONFIG_examples                 :=
      8 CONFIG_src/services/ota         :=
      9 CONFIG_tests                    :=
      10 CONFIG_src/tools/linkkit_tsl_convert :=

注意, 6-11行表示对这些目录跳过编译, 在编写未被适配平台的库时, 这在最初是必要的

#### 3. 选择配置文件

    $ make reconfig
    SELECT A CONFIGURATION:

    1) config.arm-linux.demo    3) config.ubuntu.x86
    2) config.mdm9206.sim7000c  4) config.win7.mingw32
    #? 1

    SELECTED CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo

    CONFIGURE .............................. [sample]
    CONFIGURE .............................. [src/coap]
    CONFIGURE .............................. [src/guider]
    CONFIGURE .............................. [src/http]
    CONFIGURE .............................. [src/log]
    CONFIGURE .............................. [src/mqtt]
    CONFIGURE .............................. [src/ota]
    CONFIGURE .............................. [src/platform]
    CONFIGURE .............................. [src/sdk-impl]
    CONFIGURE .............................. [src/sdk-tests]
    CONFIGURE .............................. [src/shadow]
    CONFIGURE .............................. [src/system]
    CONFIGURE .............................. [src/tfs]
    CONFIGURE .............................. [src/tls]
    CONFIGURE .............................. [src/utils]

#### 4. 交叉编译产生库文件`libiot_sdk.a`

    $ make
    BUILDING WITH EXISTING CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo

    [CC] sha1.o                     <= sha1.c
                                       makefile
    [CC] sha256.o                   <= sha256.c
                                       makefile
    ...
    ...
    [AR] libiot_sdk.a               <= src/log
                                       src/utils
                                       src/system
                                       src/sdk-impl
                                       src/guider
                                       src/mqtt
                                       src/ota
                                       src/shadow
                                       src/coap
                                       src/http

    =========================================================================
    o BUILD COMPLETE WITH FOLLOWING SWITCHES:
    ----
        FEATURE_MQTT_COMM_ENABLED    : y
        FEATURE_MQTT_SHADOW          : y
        FEATURE_MQTT_DIRECT          : y
        FEATURE_MQTT_DIRECT_NOTLS    : n
        FEATURE_COAP_COMM_ENABLED    : y
        FEATURE_COAP_DTLS_SUPPORT    : y
        FEATURE_OTA_ENABLED          : y
        FEATURE_OTA_SIGNAL_CHANNEL   : MQTT
        FEATURE_OTA_FETCH_CHANNEL    : HTTP
        FEATURE_MQTT_ID2_AUTH        : n
        FEATURE_MQTT_ID2_CRYPTO      : n
        FEATURE_MQTT_ID2_ENV         : online
        FEATURE_HTTP_COMM_ENABLED    : y

#### 5. 获取交叉编译的产物, 包括静态库和头文件

    $ ls -1 output/release/lib/
    libiot_sdk.a
    libiot_tls.a

这里, `libiot_sdk.a`文件就是编译好的物联网套件SDK, 已经是`ubuntu`格式了

另外, `libiot_tls.a`是一个裁剪过的加解密库, 您可以选择使用它, 也可以选择使用平台自带的加解密库, 以减小最终固件的尺寸

    $ ls -1 output/release/include/
    exports
    imports
    iot_export.h
    iot_import.h

这里, `iot_import.h`和`iot_export.h`就是使用SDK需要包含的头文件, 它们按功能点又包含不同的子文件, 分别列在`imports/`目录下和`exports/`目录下

## 开发未适配平台的HAL层

对于平台抽象层的库`libiot_platform.a`, 虽然说不指定任何编译和产生的方式, 但是如果你愿意的话, 当然仍然可以借助物联网套件设备端SDK的编译系统来开发和产生它

---
仍然以上一节中, 某款目前未适配的`arm-linux`目标平台为例, 假设这款平台和`Ubuntu`差别很小, 完全可以用`Ubuntu`上开发测试的HAL层代码作为开发的基础, 则可以这样做:

#### 1. 在`src/ref-impl/hal/`目录下复制一份HAL层实现代码

    $ cd src/ref-impl/hal/os/
    $ ls
    ubuntu  win7
    src/ref-impl/hal/os$ cp -rf ubuntu arm-linux
    src/ref-impl/hal/os$ ls
    arm-linux  ubuntu  win7

    src/ref-impl/hal/os$ tree -A arm-linux
    arm-linux
    +-- HAL_OS_linux.c
    +-- HAL_TCP_linux.c
    +-- HAL_UDP_linux.c

#### 2. 打开之前被关闭的`src/ref-impl/hal`编译开关

    $ vi src/board/config.arm-linux.demo

      1 CONFIG_ENV_CFLAGS = -Wall
      2
      3 OVERRIDE_CC = arm-linux-gnueabi-gcc-5
      4 OVERRIDE_AR = arm-linux-gnueabi-gcc-ar-5
      5
      6 #CONFIG_src/ref-impl/hal         :=
      7 CONFIG_examples                 :=
      8 CONFIG_src/services/ota         :=
      9 CONFIG_tests                    :=
      10 CONFIG_src/tools/linkkit_tsl_convert :=

可以看到在`CONFIG_src/ref-impl/hal =`这一行前添加了一个`#`符号, 代表这一行被注释掉了, 效果等同于被删掉, `src/ref-impl/hal`不会再被编译到

#### 3. 尝试编译被复制的HAL层代码

    $ make reconfig
    SELECT A CONFIGURATION:

    1) config.arm-linux.demo    3) config.ubuntu.x86
    2) config.mdm9206.sim7000c  4) config.win7.mingw32
    #? 1

    SELECTED CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo


    CONFIGURE .............................. [sample]
    CONFIGURE .............................. [src/coap]
    CONFIGURE .............................. [src/guider]
    CONFIGURE .............................. [src/http]
    CONFIGURE .............................. [src/log]
    CONFIGURE .............................. [src/mqtt]
    CONFIGURE .............................. [src/ota]
    CONFIGURE .............................. [src/platform]
    CONFIGURE .............................. [src/sdk-impl]
    CONFIGURE .............................. [src/sdk-tests]
    CONFIGURE .............................. [src/shadow]
    CONFIGURE .............................. [src/system]
    CONFIGURE .............................. [src/tfs]
    CONFIGURE .............................. [src/tls]
    CONFIGURE .............................. [src/utils]

    $ make
    ...
    ...
    [CC] HAL_TCP_linux.o            <= HAL_TCP_linux.c
                                       makefile
    [CC] HAL_OS_linux.o             <= HAL_OS_linux.c
                                       makefile
    [CC] HAL_UDP_linux.o            <= HAL_UDP_linux.c
                                       makefile
    [CC] HAL_DTLS_mbedtls.o         <= HAL_DTLS_mbedtls.c
                                       makefile
    [CC] HAL_TLS_mbedtls.o          <= HAL_TLS_mbedtls.c
                                       makefile
    [AR] libiot_platform.a          <= HAL_TCP_linux.o
                                       HAL_OS_linux.o
                                       HAL_UDP_linux.o
                                       HAL_DTLS_mbedtls.o
                                       HAL_TLS_mbedtls.o
    ...
    ...

可以看到我们进展的确实十分顺利, 被复制的代码`src/ref-impl/hal/os/arm-linux/*.c`确实直接编译成功了, 产生了`arm-linux`格式的`libiot_platform.a`

#### 4. 编辑配置文件, 尝试产生样例程序

这样有了`libiot_platform.a`, `libiot_tls.a`, 以及`libiot_sdk.a`, 其实以及可以尝试产生样例的可执行程序, 并运行一下试试了

方法和上一步一样, 打开`config.arm-linux.demo`里面的`CONFIG_sample`开关, 使得`example/`目录下的样例源码被编译出来

    $ vi src/configs/config.arm-linux.demo

      1 CONFIG_ENV_CFLAGS = -Wall
      2
      3 OVERRIDE_CC = arm-linux-gnueabi-gcc-5
      4 OVERRIDE_AR = arm-linux-gnueabi-gcc-ar-5
      5
      6 #CONFIG_src/ref-impl/hal         :=
      7 #CONFIG_examples                 :=
      8 CONFIG_src/services/ota         :=
      9 CONFIG_tests                    :=
      10 CONFIG_src/tools/linkkit_tsl_convert :=

可以看到在`CONFIG_examples =`这一行前添加了一个`#`符号, 代表这一行被注释掉了, 效果等同于被删掉, `example`目录不会再被编译到

#### 5. 重新载入配置文件, 编译可执行程序

    $ make reconfig
    SELECT A CONFIGURATION:

    1) config.arm-linux.demo    3) config.ubuntu.x86
    2) config.mdm9206.sim7000c  4) config.win7.mingw32
    #? 1

    SELECTED CONFIGURATION:

    VENDOR :   arm-linux
    MODEL  :   demo

    ...

    $ make
    ...
    [AR] libiot_sdk.a               <= src/log
                                       src/utils
                                       src/system
                                       src/sdk-impl
                                       src/guider
                                       src/mqtt
                                       src/ota
                                       src/shadow
                                       src/coap
                                       src/http
    [CC] mqtt-example.o             <= mqtt-example.c
                                       makefile
    [LD] mqtt-example               <= mqtt-example.o
    ...

如果有如上的编译输出, 则代表`mqtt-example`样例程序已经被成功的编译出来, 它们存放在`output/release/bin`目录下

    $ cd output/release/bin/
    $ ls
    coap-example  http-example  mqtt-example  mqtt_rrpc-example  ota_mqtt-example  shadow-example

    $ file *
    coap-example:      ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYS) ...
    http-example:      ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYS) ...
    mqtt-example:      ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYS) ...
    mqtt_rrpc-example: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYS) ...
    ota_mqtt-example:  ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYS) ...
    shadow-example:    ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYS) ...

可以用`file`命令验证, 这些可执行程序确实是交叉编译到`arm-linux`架构上的

#### 6. 尝试运行样例程序

接下来, 您就可以把样例程序例如`mqtt-example`, 用`SCP`, `TFTP`或者其它方式, 拷贝下载到您的目标开发板上运行调试了

- 如果一切顺利, 则证明`src/ref-impl/hal/os/arm-linux`部分的HAL层代码工作正常
- 如果样例程序运行有问题, 则需要再重点修改调试下`src/ref-impl/hal/os/arm-linux`部分的HAL层代码, 因为这些代码是我们从`Ubuntu`主机部分复制的, 完全可能并不适合`arm-linux`

如此反复直到确保`libiot_platform.a`的开发没问题为止

# 常见编译错误

Werror问题
```    
/home/kido/Project/Test/Code/iotkit-embedded-master/src/cmp/Link-CMP/src/iotx_cmp_common.c:61:19: error: ‘string_TIMESTAMP’ defined but not used [-Werror=unused-const-variable=]
 static const char string_TIMESTAMP[] CMP_READ_ONLY = "2524608000000";
                   ^~~~~~~~~~~~~~~~
```    

lrt   问题     
```    
按照物联网套件 linux环境下 SDK编译 iotkit-embedded 错误 提示如下，/home/luck7box/iotkit-embedded/.O/usr/lib/libiot_platform.a(HAL_OS_linux.o): In function `HAL_UptimeMs': /home/luck7box/iotkit-embedded/src/platform/os/ubuntu/HAL_OS_linux.c:116: undefined reference to `clock_gettime' collect2: ld returned 1 exit status make[3]: *** [mqtt-example] Error 1 make[2]: *** [mqtt-example] Error 2 make[1]: *** [sample] Error 2 make: *** [sub-mods] Error 2 
```    

下载代码问题
```    
关于C-SDK：
您好！我使用教程推荐的unbutu 16.04版本（在教程给出的链接下下载的）编译C-SDK时出现 缺失“mbedtls/ssl.h"错误提示
```    
