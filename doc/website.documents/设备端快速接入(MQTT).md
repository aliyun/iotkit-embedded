# 设备端快速接入(MQTT)

本文以Linux下C语言版为例，演示如何让设备连接到阿里云IoT，并通过MQTT协议的PUB/SUB实现一个简单的M2M通信。

## 第一步： 在控制台中创建设备

登录[IoT控制台](http://iot.console.aliyun.com)，创建产品及在产品下创建设备，具体步骤如下：
   + 创建产品，可得productKey、productSecret（华东2站点无productSecret）
   + 在产品下创建设备，可得deviceName，deviceSecret
   + 定义topic:$(PRODUCT_KEY)/$(DEVICE_NAME)/data，并设置权限为：设备具有发布与订阅（此步骤重要）

>**注意：**
>请根据所选站点（华东2，杭州）创建相应的产品与设备。

具体请参考[控制台使用手册](~~42714~~)文档中的`创建产品`、`添加设备`以及`获取设备Topic`部分。

> **备注:**
> 本文以Linux开发环境为例，演示如何使设备连接阿里云IoT并实现消息的收发。阿里云IoT的C版SDK提供了Linux下的接入协议MQTT、TLS库以及demo例程，需要自己编译SDK，执行demo实现物联功能。

## 第二步： 填充设备参数
> **备注:**
> 如果您还没有SDK源码，请到[SDK下载页面](~~42648~~)文档。

请将sample程序文件的设备参数替换为您控制台申请到的设备参数。

###华东2站点：
请将sample/mqtt/mqtt.c中的以下设备参数替换：：
```
//The product and device information from IOT console
//TODO Please Enter your product and device information into the following macro.
#define PRODUCT_KEY         "**********"
#define DEVICE_NAME         "***********"
#define DEVICE_SECRET       "***************************"
```
###杭州站点：
请将examples/linux/mqtt/demo.c中的以下设备参数替换：
```
 #define PRODUCT_KEY    "控制台上的productKey"
 #define PRODUCT_SECRET "控制台上的productSecret"
 #define DEVICE_NAME    "控制台申请的deviceName"
 #define DEVICE_SECRET  "控制台申请的deviceSecret"
```
>**注意：**
>topic需要以"/"开始，例如"/23298421/deviceName/update"

完成编辑并保存后，进行下一步

## 第三步： 编译SDK

* 返回顶层目录
* 执行make指令，编译SDK
```
命令如下：
make clean
make
```
编译成功后，在相应目录生成可执行程序。

## 第四步： 执行样例程序
###华东2站点
执行目录sample/mqtt下的可执行程序：
```c
./sample/mqtt/mqtt
```
样例程序的基本逻辑流程为：
1. 创建一个MQTT客户端
2. 订阅主题$(PRODUCT_KEY)/$(DEVICE_NAME)/data
3. 循环向该主题发布消息

###杭州站点
```c
./examples/linux/mqtt/demo
```

示例代码内有详细connect、pub、sub例子，请参考代码注释。

##其它
###设备影子（仅华东2站点支持）
* 请先阅读[设备影子](~~53930~~)介绍文档
* 设备影子sample代码在代码目录./sample/deviceshadow/deviceshadow.c
* 执行编译时，将在相应的目录生成可执行程序
* 具体使用方法请参考代码注释

###说明
 * 编译后在SDK的目录下会生成静态库文件 libiotsdk.a，用户可以通过此静态库使用SDK提供的功能搭建自己的设备端业务。
 * SDK中包含mqtt协议、mbedTLS库等模块，默认条件下编译生成的静态库包含上述两个模块。
 * 如果用户对SDK的编译内容进行定制，需要修改make.settings文件，具体请参照详细手册说明。

###java 版本  
 JAVA版请参考[JAVA-SDK使用(MQTT)](~~42693~~)文档。
