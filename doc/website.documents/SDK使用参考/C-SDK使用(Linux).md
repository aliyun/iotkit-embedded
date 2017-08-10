## 编译配置项说明

**请先下载最新版本设备端C语言SDK** [SDK下载](https://help.aliyun.com/document_detail/42648.html)

解压之后, 打开编译配置文件`make.settings`, 根据需要编辑配置项:
### 华东2站点

    BUILD_TYPE                  = debug

    PLATFORM_CC                 = gcc
    PLATFORM_AR                 = ar
    PLATFORM_OS                 = linux

    #
    # Uncomment below and specify PATH to your toolchain when cross-compile SDK
    #
    # PLATFORM_CC = /path/to/your/cross/gcc
    # PLATFORM_AR = /path/to/your/cross/ar

    FEATURE_MQTT_COMM_ENABLED   = y
    FEATURE_MQTT_DIRECT         = y
    FEATURE_MQTT_DIRECT_NOTLS   = n
    FEATURE_COAP_COMM_ENABLED   = y

具体含义参见下表:

    配置选项            | 含义
    --------------------|-----------------------------------------------
    BUILD_TYPE          | 指定编译SDK的版本类型, 支持`debug`, `release`
    PLATFORM_CC         | C源码编译器, 若交叉编译, 请保证交叉的`gcc/ar/strip`在同一目录
    PLATFORM_AR         | 静态库压缩器, 若交叉编译, 请保证交叉的`gcc/ar/strip`在同一目录
    PLATFORM_OS         | 指定目标平台的操作系统, SDK中有含一份目标平台为桌面linux的示例
    FEATURE_MQTT_COMM_ENABLED   | 是否使能MQTT通道功能的总开关
    FEATURE_MQTT_DIRECT         | 是否用MQTT直连模式代替HTTPS三方认证模式做设备认证
    FEATURE_MQTT_DIRECT_NOTLS   | 使用MQTT直连模式做设备认证时, 是否要关闭MQTT over TLS
    FEATURE_COAP_COMM_ENABLED   | 是否使能CoAP通道功能的总开关

### 杭州站点

      PLATFORM = linux                  // 指定编译平台名称
      PLATFORM_CC = gcc                 // 指定编译器 如: arm-none-linux-gnueabi-gcc
      PLATFORM_AR = ar                  // 指定备份压缩工具 如: arm-none-linux-gnueabi-ar
      MBEDTLS_LIB_ENABLE = y            // 是否需要编译mbedtls
      MQTT_MBED_LIB_ENABLE = y          // 是否需要编译mqtt
      CCP_LIB_ENABLE = n                // 是否需要编译ccp
      MQTTMBED_LIB_NAME = libmqtt_mbed  // mqtt静态库名称
      CCP_LIB_NAME = libccp             // ccp静态库名称
      IOT_SDK_LIB_NAME = libiotsdk      // sdk静态库名称
      PORTING_INCLUDE =                 // 移植时需要包含的头文件目录

> 使用MQTT协议不需要修改编译配置项, 使用CCP协议将`MQTT_MBED_LIB_ENABLE`改为`n`, `CCP_LIB_ENABLE`改为`y`, 保存退出即可.

> **如果目标程序运行环境是嵌入式Linux开发板, 需要按照下面的步骤说明安装交叉编译工具链, 如果目标程序运行环境是桌面Linux操作系统, 跳过交叉编译工具链安装步骤, 直接编译.**

## 交叉编译工具链安装(杭州站点SDK)
- 从开发板厂商获取交叉编译工具链压缩包, 比如`xxx_linux_gnu.tar.bz2`, 然后解压, bin文件里面包含工具链如`arm-none-linux-gnueabi-gcc`, `arm-none-linux-gnueabi-ar`等可执行文件.
- 把工具链路径添加到系统`PATH`环境变量中, 修改`~/.bashrc`文件, 在`PATH`变量中添加编译工具链安装路径

> PATH=$PATH:/home/user/sdk/arm-build-tool/bin

- 运行`.bashrc`文件, 进入用户主目录, 输入`source .bashrc`, 在终端输入`arm-none-`并按TAB键, 如果能够看到很多arm-none-linux前缀的命令, 则基本可以确定交叉编译器安装正确.

## 编译 & 运行
请参考[设备端快速开始](https://help.aliyun.com/document_detail/30530.html)
