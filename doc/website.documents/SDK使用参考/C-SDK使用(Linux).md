##编译配置项说明

* 请先下载设备端SDK

打开编译配置文件，根据需要编辑配置项：
###华东2站点
```
  BUILD_TYPE                = debug     //指定编译版本类型，支持：debug、release
  PLATFORM_CC               = gcc       //指定编译器
  PLATFORM_AR               = ar        //指定备份压缩工具
  PLATFORM_OS               = linux     //指定OS平台（对应于OS实现所在的文件夹名）
  PLATFORM_NETWORK          = linuxsock //指定socket（对应于socket实现所在的文件夹名）
  PLATFORM_SSL              = mbedtls   //指定mbedtls库（对应于ssl实现所在的文件夹名）

  ALIOT_SDK_LIB_NAME        = libaliot_sdk      //指定SDK核心代码生成的库名字
  ALIOT_PLATFORM_LIB_NAME   = libaliot_platform //指定SDK平台相关代码生成的库名字
```


###杭州站点
```
  PLATFORM = linux            // 指定编译平台名称
  PLATFORM_CC = gcc           // 指定编译器 如:arm-none-linux-gnueabi-gcc
  PLATFORM_AR = ar            // 指定备份压缩工具 如：arm-none-linux-gnueabi-ar
  MBEDTLS_LIB_ENABLE = y      // 是否需要编译mbedtls
  MQTT_MBED_LIB_ENABLE = y    // 是否需要编译mqtt
  CCP_LIB_ENABLE = n          // 是否需要编译ccp
  MQTTMBED_LIB_NAME = libmqtt_mbed   // mqtt静态库名称
  CCP_LIB_NAME = libccp              // ccp静态库名称
  IOT_SDK_LIB_NAME = libiotsdk       // sdk静态库名称
  PORTING_INCLUDE =           // 移植时需要包含的头文件目录
```
> 使用MQTT协议不需要修改编译配置项，使用CCP协议将MQTT_MBED_LIB_ENABLE改为n，CCP_LIB_ENABLE改为y，保存退出即可。

>**如果运行环境是嵌入式Linux开发板，需要按照下面的步骤说明安装交叉编译工具链，如果运行环境是Linux操作系统，跳过交叉编译工具链安装步骤，直接编译。**

##交叉编译工具链安装
- 从开发板厂商获取交叉编译工具链压缩包，比如xxx_linux_gnu.tar.bz2,然后解压，bin文件里面包含工具链如arm-none-linux-gnueabi-gcc、arm-none-linux-gnueabi-
ar等可执行文件。
- 把工具链路径添加到系统PATH环境变量中，修改~/.bashrc文件，在PATH变量中添加编译工具链安装路径
>PATH=$PATH:/home/user/sdk/arm-build-tool/bin
- 运行.bashrc文件，进入用户主目录，输入source .bashrc， 在终端输入 arm-none-并按 TAB键，如果能够看到很多arm-none-linux前缀的命令，则基本可以确定交叉编译器安装正确。

##编译 & 运行
请参考[设备端快速开始](~~30530~~)
