# 阿里云物联网套件

物联网套件是阿里云专门为物联网领域的开发人员推出的，其目的是帮助开发者搭建安全且性能强大的数据通道，方便终端(如传感器，执行器，嵌入式设备或智能家电等等)和云端的双向通信。

# 快速开始

本节描述如何申请自己的设备，并结合本SDK快速体验该设备通过`MQTT`+`TLS/SSL`协议连接到阿里云，上报和接收业务报文。关于SDK的更多使用方式，请访问[官方WiKi](https://github.com/aliyun/iotkit-embedded/wiki)

> 实现原理：
>
> `MQTT协议`(`Message Queuing Telemetry Transport`，消息队列遥测传输)是IBM开发的一个即时通讯协议，是为大量计算能力有限，且工作在低带宽，不可靠的网络的远程传感器和控制设备通讯而设计的协议
>
> 利用MQTT协议是一种基于二进制消息的发布/订阅编程模式的消息协议，下面的应用程序先在阿里云IoT平台订阅(`Subscribe`)一个`Topic`成功, 然后自己向该`Topic`做发布(`Publish`)动作
> 阿里云IoT平台收到之后，就会原样推送回这个应用程序，因为该程序之前已经通过订阅(`Subscribe`)动作成为该`Topic`的一个接收者，发布到这个`Topic`上的任何消息，都会被推送到已订阅该`Topic`的所有终端上

## 一. 开发环境准备

#### **1. 安装Ubuntu16.04**

本SDK的编译环境是`64位`的`Ubuntu16.04`，在其它Linux上尚未测试过，所以推荐安装与阿里开发者一致的发行版

如果您使用`Windows`操作系统，建议安装虚拟机软件`Virtualbox`，下载地址: [https://www.virtualbox.org/wiki/Downloads](https://www.virtualbox.org/wiki/Downloads)

然后安装64位的desktop版本`Ubuntu 16.04.x LTS`，下载地址: [https://www.ubuntu.com/download/desktop](https://www.ubuntu.com/download/desktop)

#### **2. 安装必备软件**

本SDK的开发编译环境使用如下软件: `make-4.1`, `git-2.7.4`, `gcc-5.4.0`, `gcov-5.4.0`, `lcov-1.12`, `bash-4.3.48`, `tar-1.28`, `mingw-5.3.1`

可使用如下命令行安装必要的软件:

    apt-get install -y build-essential make git gcc

## 二. 在控制台创建设备

#### **1. 注册/登录阿里云账号**

访问阿里云[登录页面](https://account.aliyun.com/login/login.htm), 点击[免费注册](https://account.aliyun.com/register/register.htm), 免费获得一个阿里云账号. 若您已有账号, 可直接登录

#### **2. 访问物联网套件控制台**

登入之后, 鼠标悬停在**产品**上, 弹出层叠菜单，并单击**阿里云Link Platform**

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/%E4%BA%A7%E5%93%81-%E9%98%BF%E9%87%8C%E4%BA%91Link%20Platform.png)

然后, 点击**立即开通**

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/%E4%BA%A7%E5%93%81-%E9%98%BF%E9%87%8C%E4%BA%91Link%20Platform-%E7%AB%8B%E5%8D%B3%E5%BC%80%E9%80%9A.png)

或直接访问[https://www.aliyun.com/product/iot](https://www.aliyun.com/product/iot), 之后点击**立即开通**, 或者**管理控制台**, 登入[控制台主界面](https://iot.console.aliyun.com/)

#### **3. 创建产品和设备**

进入IoT控制台后，点击页面左侧导航栏的**产品管理**，再点击右侧的**创建产品**，如下图所示：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81.png)

在弹出的创建产品中，填写产品信息：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81-%E8%AF%A6%E7%BB%86%E5%8F%82%E6%95%B0.png)

以基础版为例，填写**产品名称**，选择**节点类型**
填写好产品信息后，点击**确认**即可生成该产品：
![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81-%E7%A1%AE%E8%AE%A4.png)

点击产品右侧的**查看**，可跳转到产品详情页面：
![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85.png)

在该页面中，有五个主要的选项卡：
- 产品信息：展示产品相关信息，其中ProductKey用于标示产品的品类，该产品下所有设备的ProductKey均一致
- 消息通信：展示产品用于上下行数据的主要Topic
- 功能定义：在该选项卡中可定义产品的三要素（服务、属性、事件）
- 日志服务：此处可浏览设备的历史上下行消息
- 设备调试：这里可以对产品下的各个设备进行调试，如进行服务下发和属性设置

产品创建好后，接下来可以创建设备了，点击**产品详情**页面中**设备数**旁的**前往管理**，即可看到当前产品下的设备列表，目前为空：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E7%AE%A1%E7%90%86.png)

点击上图右侧的**添加设备**，开始创建设备：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E7%AE%A1%E7%90%86-%E6%B7%BB%E5%8A%A0%E8%AE%BE%E5%A4%87.png)

在填写好**DeviceName**后，点击确认即可创建该设备，生成设备的**三元组**：
- `ProductKey`：标识产品的品类，相同产品的所有设备ProductKey均相同
- `DeviceName`：标识产品下的每个设备，相同产品的所有设备DeviceName均不相同
- `DeviceSecret`：设备密钥，每个设备均不相同
- **三元组**用于标识阿里云上的每个设备，用于连接阿里云服务器时完成设备认证

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E7%AE%A1%E7%90%86-%E6%B7%BB%E5%8A%A0%E8%AE%BE%E5%A4%87-%E7%94%9F%E6%88%90%E4%B8%89%E5%85%83%E7%BB%84.png)

至此，产品与设备创建完成。

## 三. 编译样例程序

#### **1. 下载SDK**

当前物联网套件最新SDK版本为Linkkit 2.2.0，可从如下地址获取：
-   **[Linkkit 2.2.0下载地址](https://linkkit-sdk-download.oss-cn-shanghai.aliyuncs.com/linkkit2.2.tar.gz)**

#### **2. 填入设备信息**

编辑文件`iotx-sdk-c/examples/mqtt/mqtt-example.c`, 编辑如下代码段, 填入之前**创建产品和设备**步骤中得到的**设备三元组**:

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/%E4%BA%A7%E5%93%81%E6%A0%B7%E4%BE%8B-mqtt-example-%E4%B8%89%E5%85%83%E7%BB%84.png)

#### **3. 编译SDK产生样例程序**

运行如下命令:

    $ make distclean
    $ make

编译成功完成后, 生成的样例程序在当前目录的`output/release/bin`目录下:

    $ tree output/release
    output/release/
    ├── bin
    │   ├── http-example
    │   ├── linkkit-example-sched
    │   ├── linkkit-example-solo
    │   ├── linkkit_tsl_convert
    │   ├── mqtt-example
    │   ├── mqtt_multi_thread-example
    │   ├── mqtt_rrpc-example
    │   ├── ota_mqtt-example
    │   ├── sdk-testsuites
    │   └── uota_app-example
    ...
    ...

## 四. 运行样例程序

#### **1. 执行样例程序**

    $ ./output/release/bin/mqtt-example 
    [inf] iotx_device_info_init(39): device_info created successfully!
    [dbg] iotx_device_info_set(49): start to set device info!
    [dbg] iotx_device_info_set(63): device_info set successfully!
    [inf] guider_print_dev_guider_info(279): ....................................................
    [inf] guider_print_dev_guider_info(280):           ProductKey : a1Om9G9DMDx
    [inf] guider_print_dev_guider_info(281):           DeviceName : Example1
    [inf] guider_print_dev_guider_info(282):             DeviceID : a1Om9G9DMDx.Example1
    [inf] guider_print_dev_guider_info(284): ....................................................
    [inf] guider_print_dev_guider_info(285):        PartnerID Buf : ,partner_id=example.demo.partner-id
    [inf] guider_print_dev_guider_info(286):         ModuleID Buf : ,module_id=example.demo.module-id
    [inf] guider_print_dev_guider_info(287):           Guider URL : 
    [inf] guider_print_dev_guider_info(289):       Guider SecMode : 2 (TLS + Direct)
    [inf] guider_print_dev_guider_info(291):     Guider Timestamp : 2524608000000
    [inf] guider_print_dev_guider_info(292): ....................................................
    [inf] guider_print_dev_guider_info(298): ....................................................
    [inf] guider_print_conn_info(256): -----------------------------------------
    [inf] guider_print_conn_info(257):             Host : a1Om9G9DMDx.iot-as-mqtt.cn-shanghai.aliyuncs.com
    [inf] guider_print_conn_info(258):             Port : 1883
    [inf] guider_print_conn_info(261):         ClientID : a1Om9G9DMDx.Example1|securemode=2,timestamp=2524608000000,signmethod=hmacsha1,gw=0,ext=0,partner_id=example.demo.partner-id,module_id=example.demo.module-id|
    [inf] guider_print_conn_info(263):       TLS PubKey : 0x437636 ('-----BEGIN CERTI ...')
    [inf] guider_print_conn_info(266): -----------------------------------------
    [inf] IOT_MQTT_Construct(3005):      CONFIG_MQTT_SUBTOPIC_MAXNUM : 65535
    [dbg] IOT_MQTT_Construct(3007): sizeof(iotx_mc_client_t) = 1573144!
    [inf] iotx_mc_init(2098): MQTT init success!
    [inf] _ssl_client_init(142): Loading the CA root certificate ...
    [inf] _ssl_client_init(149):  ok (0 skipped)
    [inf] _TLSConnectNetwork(315): Connecting to /a1Om9G9DMDx.iot-as-mqtt.cn-shanghai.aliyuncs.com/1883...
    [inf] mbedtls_net_connect_timeout(257): setsockopt SO_SNDTIMEO timeout: 10s
    [inf] mbedtls_net_connect_timeout(260): connecting IP_ADDRESS: 139.196.135.135
    [inf] _TLSConnectNetwork(328):  ok
    [inf] _TLSConnectNetwork(333):   . Setting up the SSL/TLS structure...
    [inf] _TLSConnectNetwork(343):  ok
    [inf] _TLSConnectNetwork(382): Performing the SSL/TLS handshake...
    [inf] _TLSConnectNetwork(390):  ok
    [inf] _TLSConnectNetwork(394):   . Verifying peer X.509 certificate..
    [inf] _real_confirm(90): certificate verification result: 0x00
    [wrn] MQTTConnect(204): NOT USING pre-malloced buf 0x1394010, malloc per packet
    [dbg] MQTTConnect(204): ALLOC: curr buf = 0x13a0890, curr buf_size = 320, required payload_len = 256
    [dbg] MQTTConnect(224): FREED: curr buf = (nil), curr buf_size = 0
    [inf] iotx_mc_connect(2449): mqtt connect success!
    ...
    ...
    main|361 :: out of sample! 

#### **2. 观察消息上报**

如下日志信息显示样例程序正在通过`MQTT`的`Publish`类型消息, 上报业务数据到`/${prodcutKey}/${deviceName}/data`

    mqtt_client|256 :: packet-id=3, publish topic msg={"attr_name":"temperature", "attr_value":"1"}

#### **3. 观察消息下推**

如下日志信息显示该消息因为是到达已被订阅的`Topic`, 所以又被服务器原样推送到样例程序, 并进入相应的回调函数

    _demo_message_arrive|136 :: ----
    _demo_message_arrive|140 :: Topic: '/2UCRZpAbCGC/ExampleDev/data' (Length: 28)
    _demo_message_arrive|144 :: Payload: '{"attr_name":"temperature", "attr_value":"1"}' (Length: 45)
    _demo_message_arrive|145 :: ----

#### **4. 观察控制台日志**

可以登录物联网套件控制台, 到[设备页面](https://iot.console.aliyun.com/#/product/detail), 找到刚才填写在SDK中的设备并点击进入, 点左边导航栏的**日志服务**, 可以看到刚才被上报的消息

![image](https://raw.githubusercontent.com/wiki/aliyun/iotkit-embedded/pics/iotconsole-publog.png)

# 关于SDK的更多使用方式, 请访问[官方WiKi](https://github.com/aliyun/iotkit-embedded/wiki)
