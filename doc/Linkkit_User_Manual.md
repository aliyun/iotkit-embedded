# <a name="目录">目录</a>
+ [快速开始](#快速开始)
    * [一. 开发环境准备](#一. 开发环境准备)
        - [1. 安装Ubuntu16.04](#1. 安装Ubuntu16.04)
        - [2. 安装必备软件](#2. 安装必备软件)
        - [3. 下载物联网套件SDK](#3. 下载物联网套件SDK)
    * [二. 在控制台创建设备](#二. 在控制台创建设备)
        - [1. 注册/登录阿里云账号](#1. 注册/登录阿里云账号)
        - [2. 访问物联网套件控制台](#2. 访问物联网套件控制台)
        - [3. 创建产品和设备（基础版）](#3. 创建产品和设备（基础版）)
        - [4. 创建产品和设备（高级版）](#4. 创建产品和设备（高级版）)
    * [三. 编译运行样例程序](#三. 编译运行样例程序)
        - [1. 样例程序（基础版）](#1. 样例程序（基础版）)
            + [1.1 填入设备信息](#1.1 填入设备信息)
            + [1.2 编译SDK产生样例程序](#1.2 编译SDK产生样例程序)
            + [1.3 执行样例程序](#1.3 执行样例程序)
            + [1.4 观察消息上报](#1.4 观察消息上报)
            + [1.5 观察消息下推](#1.5 观察消息下推)
            + [1.6 观察控制台日志](#1.6 观察控制台日志)
        - [2. 样例程序（高级版）](#2. 样例程序（高级版）)
            + [2.1 填入设备信息](#2.1 填入设备信息)
            + [2.2 编译SDK产生样例程序](#2.2 编译SDK产生样例程序)
            + [2.3 执行样例程序](#2.3 执行样例程序)
+ [设备端SDK的编译](#设备端SDK的编译)
    * [编译配置项说明](#编译配置项说明)
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
+ [硬件平台抽象层(HAL层)](#硬件平台抽象层(HAL层))
    * [设备端C-SDK分层](#设备端C-SDK分层)
    * [HAL层接口详述](#HAL层接口详述)
    * [必选实现](#必选实现)
    * [多线程相关](#多线程相关)
    * [MQTT和HTTP通道功能需要](#MQTT和HTTP通道功能需要)
    * [CoAP通道功能需要](#CoAP通道功能需要)
    * [本地通信功能需要](#本地通信功能需要)
    * [设备信息设置与获取需要](#设备信息设置与获取需要)
    * [OTA功能需要](#OTA功能需要)
    * [本地定时任务功能需要](#本地定时任务功能需要)
    * [WIFI配网功能需要](#WIFI配网功能需要)
+ [C-SDK提供的功能API说明](#C-SDK提供的功能API说明)
    * [API列表](#API列表)
        - [基础版API](#基础版API)
        - [linkkit单品API](#linkkit单品API)
        - [linkkit网关API](#linkkit网关API)
    * [API详细说明](#API详细说明)
        - [必选API](#必选API)
        - [CoAP功能相关](#CoAP功能相关)
        - [MQTT功能相关](#MQTT功能相关)
        - [OTA功能相关](#OTA功能相关)
        - [HTTP功能相关](#HTTP功能相关)
        - [设备影子相关](#设备影子相关)
        - [主子设备相关(老版本接口，不推荐使用)](#主子设备相关(老版本接口，不推荐使用))
        - [linkkit单品相关](#linkkit单品相关)
        - [linkkit_gateway网关相关](#linkkit_gateway网关相关)
+ [应用场景说明](#应用场景说明)
    * [1. MQTT站点配置](#1. MQTT站点配置)
        - [基础版](#基础版)
        - [高级版](#高级版)
    * [2. TSL静态集成和动态拉取](#2. TSL静态集成和动态拉取)
        - [TSL静态集成](#TSL静态集成)
        - [TSL动态拉取](#TSL动态拉取)
            + [高级版单品使用TSL动态拉取](#高级版单品使用TSL动态拉取)
            + [高级版网关使用TSL动态拉取](#高级版网关使用TSL动态拉取)


# <a name="快速开始">快速开始</a>

本节描述如何申请自己的设备，并结合本SDK快速体验该设备通过`MQTT`+`TLS/SSL`协议连接到阿里云，上报和接收业务报文。关于SDK的更多使用方式，请访问[官方WiKi](https://github.com/aliyun/iotkit-embedded/wiki)

> 实现原理：
>
> `MQTT协议`(`Message Queuing Telemetry Transport`，消息队列遥测传输)是IBM开发的一个即时通讯协议，是为大量计算能力有限，且工作在低带宽，不可靠的网络的远程传感器和控制设备通讯而设计的协议
>
> 利用MQTT协议是一种基于二进制消息的发布/订阅编程模式的消息协议，下面的应用程序先在阿里云IoT平台订阅(`Subscribe`)一个`Topic`成功, 然后自己向该`Topic`做发布(`Publish`)动作
> 阿里云IoT平台收到之后，就会原样推送回这个应用程序，因为该程序之前已经通过订阅(`Subscribe`)动作成为该`Topic`的一个接收者，发布到这个`Topic`上的任何消息，都会被推送到已订阅该`Topic`的所有终端上

## <a name="一. 开发环境准备">一. 开发环境准备</a>

### <a name="1. 安装Ubuntu16.04">1. 安装Ubuntu16.04</a>

本SDK的编译环境是`64位`的`Ubuntu16.04`，在其它Linux上尚未测试过，所以推荐安装与阿里开发者一致的发行版

如果您使用`Windows`操作系统，建议安装虚拟机软件`Virtualbox`，下载地址: [https://www.virtualbox.org/wiki/Downloads](https://www.virtualbox.org/wiki/Downloads)

然后安装64位的desktop版本`Ubuntu 16.04.x LTS`，下载地址: [https://www.ubuntu.com/download/desktop](https://www.ubuntu.com/download/desktop)

### <a name="2. 安装必备软件">2. 安装必备软件</a>

本SDK的开发编译环境使用如下软件: `make-4.1`, `git-2.7.4`, `gcc-5.4.0`, `gcov-5.4.0`, `lcov-1.12`, `bash-4.3.48`, `tar-1.28`, `mingw-5.3.1`

可使用如下命令行安装必要的软件:

    apt-get install -y build-essential make git gcc

### <a name="3. 下载物联网套件SDK">3. 下载物联网套件SDK</a>

当前物联网套件最新SDK版本为Linkkit 2.2.0，可从如下地址获取：
-   **[Linkkit 2.2.0下载地址](https://linkkit-sdk-download.oss-cn-shanghai.aliyuncs.com/linkkit2.2.tar.gz)**

## <a name="二. 在控制台创建设备">二. 在控制台创建设备</a>

### <a name="1. 注册/登录阿里云账号">1. 注册/登录阿里云账号</a>

访问阿里云[登录页面](https://account.aliyun.com/login/login.htm), 点击[免费注册](https://account.aliyun.com/register/register.htm), 免费获得一个阿里云账号. 若您已有账号, 可直接登录

### <a name="2. 访问物联网套件控制台">2. 访问物联网套件控制台</a>

登入之后, 鼠标悬停在**产品**上, 弹出层叠菜单，并单击**阿里云Link Platform**

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/%E4%BA%A7%E5%93%81-%E9%98%BF%E9%87%8C%E4%BA%91Link%20Platform.png)

然后, 点击**立即开通**，即可进入IoT控制台

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/%E4%BA%A7%E5%93%81-%E9%98%BF%E9%87%8C%E4%BA%91Link%20Platform-%E7%AB%8B%E5%8D%B3%E5%BC%80%E9%80%9A.png)

### <a name="3. 创建产品和设备（基础版）">3. 创建产品和设备（基础版）</a>

进入IoT控制台后，点击页面左侧导航栏的**产品管理**，再点击右侧的**创建产品**，如下图所示：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81.png)

在弹出的创建产品中，选择基础版，填写产品信息：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81-%E8%AF%A6%E7%BB%86%E5%8F%82%E6%95%B0.png)

填写**产品名称**，选择**节点类型**。

填写好产品信息后，点击**确认**即可生成该产品：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81-%E7%A1%AE%E8%AE%A4.png)

点击产品右侧的**查看**，可跳转到产品详情页面：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85.png)

在该页面中，有五个主要的选项卡：

- 产品信息：展示产品相关信息，其中ProductKey用于标示产品的品类，该产品下所有设备的ProductKey均一致
- 消息通信：展示产品用于上下行数据的主要Topic
- 服务端订阅：在这里可以选择设备消息类型并推送到MNS队列中，用户服务端从队列里获得设备数据。这样简化服务端订阅设备数据的流程，让客户的服务端能够简单方便并高可靠的获得设备数据。
- 日志服务：此处可浏览设备的历史上下行消息

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

至此，基础版产品与设备创建完成。

此外，为了example演示，我们在产品的消息通信中定义一个自定义的topic，用于向服务端发送数据和从服务端接收数据：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81-%E6%B6%88%E6%81%AF%E9%80%9A%E4%BF%A1-%E8%87%AA%E5%AE%9A%E4%B9%89topic-data.png)

### <a name="4. 创建产品和设备（高级版）">4. 创建产品和设备（高级版）</a>

进入IoT控制台后，点击页面左侧导航栏的**产品管理**，再点击右侧的**创建产品**，如下图所示：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81.png)

在弹出的创建产品中，选择高级版，填写产品信息：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81-%E8%AF%A6%E7%BB%86%E5%8F%82%E6%95%B0.png)

在此需要填写以下信息：

- 填写**产品名称**
- 选择**节点类型**
- 选择**设备类型**，如果没有需要的设备类型，可以填空，稍后在产品**功能定义**时再定义具体的功能
- 选择**数据格式**，目前有**Alink JSON**和**透传/自定义**两种格式可选（在这里我们使用Alink JSON格式作为示例）  
    **Alink JSON**：使用标准的Alink协议格式来进行物联网套件与云端的服务、属性、事件的数据交换  
    **透传/自定义**：使用物联网套件的透传接口来进行与云端的数据交换，在云端需要用户完成透传数据与Alink格式的转换脚本
- **ID2**：是否使用ID2认证

填写好产品信息后，点击**确认**即可生成该产品：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E5%88%9B%E5%BB%BA%E4%BA%A7%E5%93%81-%E7%A1%AE%E8%AE%A4.png)

点击产品右侧的**查看**，可跳转到产品详情页面：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85.png)

在该页面中，有五个主要的选项卡：

- 产品信息：展示产品相关信息，其中ProductKey用于标示产品的品类，该产品下所有设备的ProductKey均一致
- 消息通信：展示产品用于上下行数据的主要Topic
- 功能定义：在这里可以定义设备的物模型，定义物的服务、属性、事件
- 日志服务：此处可浏览设备的历史上下行消息
- 在线调试：此处可对该产品下的设备进行在线调试，如下发服务到设备，设置设备的属性，观察设备的事件上报

产品创建好后，接下来可以创建设备了，点击**产品详情**页面中**设备数**旁的**前往管理**，即可看到当前产品下的设备列表，目前为空：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E7%AE%A1%E7%90%86.png)

点击上图右侧的**添加设备**，开始创建设备：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E7%AE%A1%E7%90%86-%E6%B7%BB%E5%8A%A0%E8%AE%BE%E5%A4%87.png)

在填写好**DeviceName**后，点击确认即可创建该设备，生成设备的**三元组**：

- `ProductKey`：标识产品的品类，相同产品的所有设备ProductKey均相同
- `DeviceName`：标识产品下的每个设备，相同产品的所有设备DeviceName均不相同
- `DeviceSecret`：设备密钥，每个设备均不相同
- **三元组**用于标识阿里云上的每个设备，用于连接阿里云服务器时完成设备认证

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E7%AE%A1%E7%90%86-%E6%B7%BB%E5%8A%A0%E8%AE%BE%E5%A4%87-%E7%94%9F%E6%88%90%E4%B8%89%E5%85%83%E7%BB%84.png)

至此，高级版产品与设备创建完成。

稍后会展示高级版服务、属性、事件的示例程序，所以在这里我们回到**产品管理**，选择我们刚才创建的产品，进入**产品详情**页，选择**功能定义**选项卡：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E5%8A%9F%E8%83%BD%E5%AE%9A%E4%B9%89.png)

选择右侧的**新增**，我们先创建一个属性：  

该属性是一个字符串属性，最大长度为2048个字节

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E5%8A%9F%E8%83%BD%E5%AE%9A%E4%B9%89-%E6%96%B0%E5%A2%9E%E5%AD%97%E7%AC%A6%E4%B8%B2%E5%B1%9E%E6%80%A7.png)

点击**确认**，这样一个属性就创建好了

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E5%8A%9F%E8%83%BD%E5%AE%9A%E4%B9%89-%E6%96%B0%E5%A2%9E%E5%AD%97%E7%AC%A6%E4%B8%B2%E5%B1%9E%E6%80%A7-%E5%88%9B%E5%BB%BA%E6%88%90%E5%8A%9F.png)

接下来我们创建一个服务：  

关于服务的创建要注意的是，服务只能由服务端下发，设备端被动接收，每个服务有自己的输入参数和输出参数，说明如下：  

- 服务的**输入参数**：指的是当从云端下发服务时，云端向设备端发送的数据内容
- 服务的**输出参数**：指的是当设备端收到从云端下发的服务时，如果有需要对该服务返回一些业务数据，那么就是输出参数的内容  

在这个服务中我们创建一个输入参数和一个输出参数

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E5%8A%9F%E8%83%BD%E5%AE%9A%E4%B9%89-%E6%96%B0%E5%A2%9E%E6%9C%8D%E5%8A%A1.png)

最后，再创建一个事件：

关于事件要注意的是，事件只能由设备端主动上报给服务端，每个事件只有输出参数，表示从设备端需要上报的数据内容  

在这个事件中我们创建一个输出参数

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E5%8A%9F%E8%83%BD%E5%AE%9A%E4%B9%89-%E6%96%B0%E5%A2%9E%E4%BA%8B%E4%BB%B6.png)

至此，我们创建的产品中服务、属性、事件各有一个：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E5%8A%9F%E8%83%BD%E5%AE%9A%E4%B9%89-%E5%AE%8C%E6%88%90.png)


## <a name="三. 编译运行样例程序">三. 编译运行样例程序</a>

### <a name="1. 样例程序（基础版）">1. 样例程序（基础版）</a>


#### <a name="1.1 填入设备信息">1.1 填入设备信息</a>

编辑文件`iotx-sdk-c/examples/mqtt/mqtt-example.c`, 编辑如下代码段, 填入之前**创建产品和设备**步骤中得到的**设备三元组**:

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BB%A3%E7%A0%81%E7%A4%BA%E4%BE%8B-mqtt-example-%E4%B8%89%E5%85%83%E7%BB%84%E4%BF%AE%E6%94%B9.png)

#### <a name="1.2 编译SDK产生样例程序">1.2 编译SDK产生样例程序</a>

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

#### <a name="1.3 执行样例程序">1.3 执行样例程序</a>

    $ ./output/release/bin/mqtt-example 
    [inf] iotx_device_info_init(39): device_info created successfully!
    [dbg] iotx_device_info_set(49): start to set device info!
    [dbg] iotx_device_info_set(63): device_info set successfully!
    [inf] guider_print_dev_guider_info(279): ....................................................
    [inf] guider_print_dev_guider_info(280):           ProductKey : a1ExpAkj9Hi
    [inf] guider_print_dev_guider_info(281):           DeviceName : Example1
    [inf] guider_print_dev_guider_info(282):             DeviceID : a1ExpAkj9Hi.Example1
    [inf] guider_print_dev_guider_info(284): ....................................................
    [inf] guider_print_dev_guider_info(285):        PartnerID Buf : ,partner_id=example.demo.partner-id
    [inf] guider_print_dev_guider_info(286):         ModuleID Buf : ,module_id=example.demo.module-id
    [inf] guider_print_dev_guider_info(287):           Guider URL : 
    [inf] guider_print_dev_guider_info(289):       Guider SecMode : 2 (TLS + Direct)
    [inf] guider_print_dev_guider_info(291):     Guider Timestamp : 2524608000000
    [inf] guider_print_dev_guider_info(292): ....................................................
    [inf] guider_print_dev_guider_info(298): ....................................................
    [inf] guider_print_conn_info(256): -----------------------------------------
    [inf] guider_print_conn_info(257):             Host : a1ExpAkj9Hi.iot-as-mqtt.cn-shanghai.aliyuncs.com
    [inf] guider_print_conn_info(258):             Port : 1883
    [inf] guider_print_conn_info(261):         ClientID : a1ExpAkj9Hi.Example1|securemode=2,timestamp=2524608000000,signmethod=hmacsha1,gw=0,ext=0,partner_id=example.demo.partner-id,module_id=example.demo.module-id|
    [inf] guider_print_conn_info(263):       TLS PubKey : 0x437636 ('-----BEGIN CERTI ...')
    [inf] guider_print_conn_info(266): -----------------------------------------
    [inf] IOT_MQTT_Construct(3005):      CONFIG_MQTT_SUBTOPIC_MAXNUM : 65535
    [dbg] IOT_MQTT_Construct(3007): sizeof(iotx_mc_client_t) = 1573144!
    [inf] iotx_mc_init(2098): MQTT init success!
    [inf] _ssl_client_init(142): Loading the CA root certificate ...
    [inf] _ssl_client_init(149):  ok (0 skipped)
    [inf] _TLSConnectNetwork(315): Connecting to /a1ExpAkj9Hi.iot-as-mqtt.cn-shanghai.aliyuncs.com/1883...
    [inf] mbedtls_net_connect_timeout(257): setsockopt SO_SNDTIMEO timeout: 10s
    [inf] mbedtls_net_connect_timeout(260): connecting IP_ADDRESS: 106.15.100.2
    [inf] _TLSConnectNetwork(328):  ok
    [inf] _TLSConnectNetwork(333):   . Setting up the SSL/TLS structure...
    [inf] _TLSConnectNetwork(343):  ok
    [inf] _TLSConnectNetwork(382): Performing the SSL/TLS handshake...
    [inf] _TLSConnectNetwork(390):  ok
    [inf] _TLSConnectNetwork(394):   . Verifying peer X.509 certificate..
    [inf] _real_confirm(90): certificate verification result: 0x00
    [wrn] MQTTConnect(204): NOT USING pre-malloced buf 0xc4a010, malloc per packet
    [dbg] MQTTConnect(204): ALLOC: curr buf = 0xc56890, curr buf_size = 320, required payload_len = 256
    [dbg] MQTTConnect(224): FREED: curr buf = (nil), curr buf_size = 0
    [inf] iotx_mc_connect(2449): mqtt connect success!
    ...
    ...
    mqtt_client|309 :: packet-id=7, publish topic msg={"attr_name":"temperature", "attr_value":"1"}
    [dbg] iotx_mc_cycle(1591): PUBACK
    event_handle|132 :: publish success, packet-id=7
    [dbg] iotx_mc_cycle(1608): PUBLISH
    [dbg] iotx_mc_handle_recv_PUBLISH(1412):         Packet Ident : 00035641
    [dbg] iotx_mc_handle_recv_PUBLISH(1413):         Topic Length : 26
    [dbg] iotx_mc_handle_recv_PUBLISH(1417):           Topic Name : /a1ExpAkj9Hi/Example1/data
    [dbg] iotx_mc_handle_recv_PUBLISH(1420):     Payload Len/Room : 45 / 992
    [dbg] iotx_mc_handle_recv_PUBLISH(1421):       Receive Buflen : 1024
    [dbg] iotx_mc_handle_recv_PUBLISH(1432): delivering msg ...
    [dbg] iotx_mc_deliver_message(1170): topic be matched
    _demo_message_arrive|166 :: ----
    _demo_message_arrive|167 :: packetId: 35641
    _demo_message_arrive|171 :: Topic: '/a1ExpAkj9Hi/Example1/data' (Length: 26)
    _demo_message_arrive|175 :: Payload: '{"attr_name":"temperature", "attr_value":"1"}' (Length: 45)
    _demo_message_arrive|176 :: ----
    ...
    ...
    main|361 :: out of sample! 

#### <a name="1.4 观察消息上报">1.4 观察消息上报</a>

如下日志信息显示样例程序正在通过`MQTT`的`Publish`类型消息, 上报业务数据到`/${prodcutKey}/${deviceName}/data`

    mqtt_client|309 :: packet-id=7, publish topic msg={"attr_name":"temperature", "attr_value":"1"}

#### <a name="1.5 观察消息下推">1.5 观察消息下推</a>

如下日志信息显示该消息因为是到达已被订阅的`Topic`, 所以又被服务器原样推送到样例程序, 并进入相应的回调函数

    _demo_message_arrive|166 :: ----
    _demo_message_arrive|167 :: packetId: 35641
    _demo_message_arrive|171 :: Topic: '/a1ExpAkj9Hi/Example1/data' (Length: 26)
    _demo_message_arrive|175 :: Payload: '{"attr_name":"temperature", "attr_value":"1"}' (Length: 45)
    _demo_message_arrive|176 :: ----

#### <a name="1.6 观察控制台日志">1.6 观察控制台日志</a>

可以登录物联网套件控制台, 到**产品管理**, 找到刚才创建的产品，点击**查看**，选择**日志服务**选项卡，可以看到刚才上报的消息（Message ID为7）

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E6%97%A5%E5%BF%97%E6%9C%8D%E5%8A%A1.png)

### <a name="2. 样例程序（高级版）">2. 样例程序（高级版）</a>

高级版以单品API为例演示设备服务、属性、事件的数据流转

#### <a name="2.1 填入设备信息">2.1 填入设备信息</a>
高级版单品的exmaple位于`iotx-sdk-c/examples/linkkit/linkkit_example_solo.c`，该example用到的TSL位于同目录下的`example_tsl_solo.data`。  

接下来我们需要将SDK默认的设备信息更换成我们在前一章节中创建的高级版设备，需要替换设备的三元组以及设备的TSL  

进入**产品管理**，选择我们刚才创建的设备`AdvUserExample`，进入`产品详情`，选择`功能定义`选项卡，这里可以看到之前定义的服务、属性和事件。  

点击右侧的`查看物模型`，如下图所示：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E7%89%A9%E6%A8%A1%E5%9E%8B.png)

点击`导出模型文件`将物模型导出

此时可以运行如下命令先编译SDK

    $ make distclean
    $ make

在`iotx-sdk-c/output/release/bin`目录下会产生一个转义工具`linkkit_tsl_convert`，将刚才下载的物模型文件**model.json**拷贝到这里，执行如下命令：  
    
    $ ./linkkit_tsl_convert -i model.json
    
命令执行成功后会在当前目录下产生一个`conv.txt`文件，这个就是转义好的物模型字符串了。（转义工具网上很多，也可自行寻找）  

用`conv.txt`文件中的字符串替换`iotx-sdk-c/examples/linkkit/example_tsl_solo.data`中变量`TSL_STRING`的字符串即可。

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E5%8D%95%E5%93%81%E6%9B%BF%E6%8D%A2TSL.png)

然后将`iotx-sdk-c/examples/linkkit/linkkit_example_solo.c`中的三元组替换成刚才创建的设备

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E5%8D%95%E5%93%81%E6%9B%BF%E6%8D%A2%E4%B8%89%E5%85%83%E7%BB%84.png)

这样三元组和TSL都已换成我们刚才创建的设备。  

之前我们在产品的**功能定义**中定义了一个服务、一个属性和一个事件，与`linkkit_example_solo.c`中的相同，所以不需要做出修改。

#### <a name="2.2 编译SDK产生样例程序">2.2 编译SDK产生样例程序</a>

运行如下命令:

    $ make distclean
    $ make

编译成功完成后, 生成的样例程序为当前目录的`output/release/bin`目录下的`linkkit-example-solo`

#### <a name="2.3 执行样例程序">2.3 执行样例程序</a>

    ./output/release/bin/linkkit-example-solo 
    main|686 :: start!
    
    linkkit_example|587 :: linkkit start
    [dbg] _dm_mgr_search_dev_by_devid(52): Device Not Found, devid: 0
    [inf] iotx_cm_init(77): cm verstion 0.3
    [inf] iotx_device_info_init(39): device_info created successfully!
    [dbg] iotx_device_info_set(49): start to set device info!
    [dbg] iotx_device_info_set(63): device_info set successfully!
    [inf] guider_print_dev_guider_info(279): ....................................................
    [inf] guider_print_dev_guider_info(280):           ProductKey : a1csED27mp7
    [inf] guider_print_dev_guider_info(281):           DeviceName : AdvExample1
    [inf] guider_print_dev_guider_info(282):             DeviceID : a1csED27mp7.AdvExample1
    [inf] guider_print_dev_guider_info(284): ....................................................
    [inf] guider_print_dev_guider_info(285):        PartnerID Buf : ,partner_id=example.demo.partner-id
    [inf] guider_print_dev_guider_info(286):         ModuleID Buf : ,module_id=example.demo.module-id
    [inf] guider_print_dev_guider_info(287):           Guider URL : 
    [inf] guider_print_dev_guider_info(289):       Guider SecMode : 2 (TLS + Direct)
    [inf] guider_print_dev_guider_info(291):     Guider Timestamp : 2524608000000
    [inf] guider_print_dev_guider_info(292): ....................................................
    [inf] guider_print_dev_guider_info(298): ....................................................
    [inf] guider_print_conn_info(256): -----------------------------------------
    [inf] guider_print_conn_info(257):             Host : a1csED27mp7.iot-as-mqtt.cn-shanghai.aliyuncs.com
    [inf] guider_print_conn_info(258):             Port : 1883
    [inf] guider_print_conn_info(261):         ClientID : a1csED27mp7.AdvExample1|securemode=2,timestamp=2524608000000,signmethod=hmacsha1,gw=0,ext=0,partner_id=example.demo.partner-id,module_id=example.demo.module-id|
    [inf] guider_print_conn_info(263):       TLS PubKey : 0x47ffd6 ('-----BEGIN CERTI ...')
    [inf] guider_print_conn_info(266): -----------------------------------------
    [inf] iotx_cm_init(126): cm context initialize
    [inf] linked_list_insert(120): linked list(cm event_cb list) insert node@0x18a2760,size:1
    [inf] linked_list_insert(120): linked list(cm connectivity list) insert node@0x18a28c0,size:1
    [err] iotx_cm_add_connectivity(113): Add Connectivity Success, Type: 1
    [inf] iotx_cm_conn_mqtt_init(358):            CONFIG_MQTT_TX_MAXLEN : 1024
    [inf] iotx_cm_conn_mqtt_init(359):            CONFIG_MQTT_RX_MAXLEN : 5000
    [wrn] iotx_cm_conn_mqtt_init(369): WITH_MQTT_DYNBUF = 1, skipping malloc sendbuf of 1024 bytes
    [inf] IOT_MQTT_Construct(3005):      CONFIG_MQTT_SUBTOPIC_MAXNUM : 65535
    [dbg] IOT_MQTT_Construct(3007): sizeof(iotx_mc_client_t) = 1573144!
    [inf] iotx_mc_init(2098): MQTT init success!
    [inf] _ssl_client_init(142): Loading the CA root certificate ...
    [inf] _ssl_client_init(149):  ok (0 skipped)
    [inf] _TLSConnectNetwork(315): Connecting to /a1csED27mp7.iot-as-mqtt.cn-shanghai.aliyuncs.com/1883...
    [inf] mbedtls_net_connect_timeout(257): setsockopt SO_SNDTIMEO timeout: 10s
    [inf] mbedtls_net_connect_timeout(260): connecting IP_ADDRESS: 106.15.100.2
    [inf] _TLSConnectNetwork(328):  ok
    [inf] _TLSConnectNetwork(333):   . Setting up the SSL/TLS structure...
    [inf] _TLSConnectNetwork(343):  ok
    [inf] _TLSConnectNetwork(382): Performing the SSL/TLS handshake...
    [inf] _TLSConnectNetwork(390):  ok
    [inf] _TLSConnectNetwork(394):   . Verifying peer X.509 certificate..
    [inf] _real_confirm(90): certificate verification result: 0x00
    [dbg] MQTTConnect(204): ALLOC: curr buf = 0x18af8f0, curr buf_size = 320, required payload_len = 256
    [dbg] MQTTConnect(224): FREED: curr buf = (nil), curr buf_size = 0
    [inf] iotx_mc_connect(2449): mqtt connect success!
    ...
    ...

属性
---
对于属性示例程序会每隔30s上报一次所有属性，因为我们定义了一个属性`DeviceStatus`，所以应该看到如下日志：

    ```
    [dbg] iotx_dm_post_property_end(450): Current Property Post Payload, Length: 19, Payload: {"DeviceStatus":""}
    [dbg] _dm_mgr_search_dev_by_devid(46): Device Found, devid: 0
    [inf] dm_msg_request_all(265): DM Send Message, URI: /sys/a1csED27mp7/AdvExample1/thing/event/property/post, Payload: {"id":"1","version":"1.0","params":{"DeviceStatus":""},"method":"thing.event.property.post"}
    [inf] iotx_cm_conn_mqtt_publish(531): mqtt publish: topic=/sys/a1csED27mp7/AdvExample1/thing/event/property/post, topic_msg={"id":"4","version":"1.0","params":{"DeviceStatus":""},"method":"thing.event.property.post"}
    ```
    
可以看出，由于我们没有设置属性的值，所以默认该值为空字符串，此时在物联网控制台上可以查询到刚才的上报记录：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E6%97%A5%E5%BF%97%E6%9C%8D%E5%8A%A1-%E5%B1%9E%E6%80%A7%E4%B8%8A%E6%8A%A5.png)

从上图可以看出，一条Property Post消息已上报至服务端

此时可从服务端主动向这个属性set一个值，打开`产品管理`->`产品详情`->`在线调试选项卡`，选择我们要调试的设备：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E8%B0%83%E8%AF%95-%E8%AE%BE%E7%BD%AE%E5%B1%9E%E6%80%A7.png)

如上图所示，选择`DeviceStatus`这个属性，然后选择`设置`，在下方的输入框中将"Hello World"填入该属性的值，然后点击发送指令，  
此时在设备端的日志中可以看到从服务端set下来的值：

    [dbg] iotx_mc_cycle(1608): PUBLISH
    [dbg] iotx_mc_handle_recv_PUBLISH(1412):         Packet Ident : 00000000
    [dbg] iotx_mc_handle_recv_PUBLISH(1413):         Topic Length : 55
    [dbg] iotx_mc_handle_recv_PUBLISH(1417):           Topic Name : /sys/a1csED27mp7/AdvExample1/thing/service/property/set
    [dbg] iotx_mc_handle_recv_PUBLISH(1420):     Payload Len/Room : 112 / 4940
    [dbg] iotx_mc_handle_recv_PUBLISH(1421):       Receive Buflen : 5000
    [dbg] iotx_mc_handle_recv_PUBLISH(1432): delivering msg ...
    [dbg] iotx_mc_deliver_message(1170): topic be matched
    [inf] iotx_cloud_conn_mqtt_event_handle(180): event_type 12
    [inf] iotx_cloud_conn_mqtt_event_handle(325): mqtt received: topic=/sys/a1csED27mp7/AdvExample1/thing/service/property/set, topic_msg={"method":"thing.service.property.set","id":"65822254","params":{"DeviceStatus":"HelloWorld"},"version":"1.0.0"}
    
这样，一条从服务端设置属性的命令就到达设备端了，由于在example里收到这条属性后，会调用`linkkit_post_property`将该属性值上报给服务端，所以有如下的日志：

    [dbg] iotx_dm_post_property_end(450): Current Property Post Payload, Length: 29, Payload: {"DeviceStatus":"HelloWorld"}
    [dbg] _dm_mgr_search_dev_by_devid(46): Device Found, devid: 0
    [inf] dm_msg_request_all(265): DM Send Message, URI: /sys/a1csED27mp7/AdvExample1/thing/event/property/post, Payload: {"id":"34","version":"1.0","params":{"DeviceStatus":"HelloWorld"},"method":"thing.event.property.post"}
    [inf] iotx_cm_conn_mqtt_publish(531): mqtt publish: topic=/sys/a1csED27mp7/AdvExample1/thing/event/property/post, topic_msg={"id":"34","version":"1.0","params":{"DeviceStatus":"HelloWorld"},"method":"thing.event.property.post"}
    
只有当设备端主动上报属性值到服务端后，才能在服务端查询到这个属性值。  
在`设备调试`中，选择`DeviceStatus`这个属性，然后选择`获取`，在下方的输入框中就可以看到刚才设置的属性了：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E8%B0%83%E8%AF%95-%E8%8E%B7%E5%8F%96%E5%B1%9E%E6%80%A7.png)

事件
---

示例程序中`Error`事件是每45s上报一次，日志如下：

    [inf] dm_msg_request_all(265): DM Send Message, URI: /sys/a1csED27mp7/AdvExample1/thing/event/Error/post, Payload: {"id":"36","version":"1.0","params":{"ErrorCode":0},"method":"thing.event.Error.post"}
    [inf] iotx_cm_conn_mqtt_publish(531): mqtt publish: topic=/sys/a1csED27mp7/AdvExample1/thing/event/Error/post, topic_msg={"id":"36","version":"1.0","params":{"ErrorCode":0},"method":"thing.event.Error.post"}

相应地，在物联网控制台的`日志服务`或者`在线调试`选项卡中可以看到对应的信息：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E8%B0%83%E8%AF%95-%E4%BA%8B%E4%BB%B6%E4%B8%8A%E6%8A%A5.png)
![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E8%B0%83%E8%AF%95-%E8%8E%B7%E5%8F%96%E4%BA%8B%E4%BB%B6.png)

服务
---

同样，在物联网控制台中打开`设备调试`选项卡，选择我们创建的服务`Custom`，由于该服务的输入参数数据类型为int型，标识为`transparency`，所以在下方的输入框中填入参数：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-ADV-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E8%AE%BE%E5%A4%87%E8%B0%83%E8%AF%95-%E6%9C%8D%E5%8A%A1%E4%B8%8B%E5%8F%91.png)

此时在设备端可以看到如下日志：

    [dbg] iotx_mc_cycle(1608): PUBLISH
    [dbg] iotx_mc_handle_recv_PUBLISH(1412):         Packet Ident : 00000000
    [dbg] iotx_mc_handle_recv_PUBLISH(1413):         Topic Length : 49
    [dbg] iotx_mc_handle_recv_PUBLISH(1417):           Topic Name : /sys/a1csED27mp7/AdvExample1/thing/service/Custom
    [dbg] iotx_mc_handle_recv_PUBLISH(1420):     Payload Len/Room : 95 / 4946
    [dbg] iotx_mc_handle_recv_PUBLISH(1421):       Receive Buflen : 5000
    [dbg] iotx_mc_handle_recv_PUBLISH(1432): delivering msg ...
    [dbg] iotx_mc_deliver_message(1170): topic be matched
    [inf] iotx_cloud_conn_mqtt_event_handle(180): event_type 12
    [inf] iotx_cloud_conn_mqtt_event_handle(325): mqtt received: topic=/sys/a1csED27mp7/AdvExample1/thing/service/Custom, topic_msg={"method":"thing.service.Custom","id":"65850626","params":{"transparency":5},"version":"1.0.0"}
    [inf] iotx_cm_cloud_conn_response_callback(121): rsp_type 11
    [inf] iotx_cm_cloud_conn_response_handler(525): URI = /sys/a1csED27mp7/AdvExample1/thing/service/Custom{"method":"thing.service.Custom","id":"65850626","params":{"transparency":5},"version":"1.0.0"}
    [inf] dm_disp_event_new_data_received_handler(1289): IOTX_CM_EVENT_NEW_DATA_RECEIVED
    [inf] dm_cmw_topic_callback(13): DMGR TOPIC CALLBACK
    [inf] dm_cmw_topic_callback(20): DMGR Receive Message: /sys/a1csED27mp7/AdvExample1/thing/service/Custom{"method":"thing.service.Custom","id":"65850626","params":{"transparency":5},"version":"1.0.0"}
    ...
    ...
    [dbg] iotx_dm_send_service_response(597): Current Service Response Payload, Length: 19, Payload: {"Contrastratio":6}
    [dbg] _dm_mgr_search_dev_by_devid(46): Device Found, devid: 0
    [dbg] dm_mgr_upstream_thing_service_response(2098): Current Service Name: thing/service/Custom_reply
    [dbg] dm_msg_response_with_data(384): Send URI: /sys/a1csED27mp7/AdvExample1/thing/service/Custom_reply, Payload: {"id":"65850626","code":200,"data":{"Contrastratio":6}}
    [inf] iotx_cm_conn_mqtt_publish(531): mqtt publish: topic=/sys/a1csED27mp7/AdvExample1/thing/service/Custom_reply, topic_msg={"id":"65850626","code":200,"data":{"Contrastratio":6}}

我们可以看到，设备端example已经收到从云端下发的服务`Custom`，其中服务的输入参数`transparency`的值为5

而在example中，当收到`transparency`后，会将该输入参数的值+1赋给输出参数`Contrastratio`，从上面的日志中可以看到`Contrastratio`的值被设成了6，然后上报给服务端。  

关于单品example中服务、属性、事件的说明就此结束

> 关于SDK的更多使用方式, 请访问[阿里云物联网平台帮助文档](https://help.aliyun.com/product/30520.html)

# <a name="设备端SDK的编译">设备端SDK的编译</a>

设备端SDK目前支持以下的编译方式:

- 在`Linux`上或者`MacOS`上以`GNU Make` + `各种工具链`编译, 产生`各种目标架构`的SDK
- 在`Linux`上或者`Windows`上或者`MacOS`上以`CMake` + `Mingw32`编译`, 产生Windows架构`的SDK
- 在`Windows`上以`VS Code 2017`编译, 产生`Windows架构`的SDK
- 在`Windows`上以`QT Creator`编译, 产生`Windows架构`的SDK
- 在`Windows`上以`Visual Studio 2017`编译, 产生`Windows架构`的SDK

## <a name="编译配置项说明">编译配置项说明</a>

解压之后, 打开编译配置文件`make.settings`, 根据需要编辑配置项，使用不同的编译配置，编译输出的SDK内容以及examples都有所不同。

    FEATURE_OTA_ENABLED          = y          # 是否打开linkkit中OTA功能的分开关
    FEATURE_MQTT_COMM_ENABLED    = y          # 是否打开MQTT通道的总开关
    FEATURE_MQTT_DIRECT          = y          # 是否打开MQTT直连的分开关
    FEATURE_COAP_COMM_ENABLED    = y          # 是否打开CoAP通道的总开关
    FEATURE_HTTP_COMM_ENABLED    = y          # 是否打开HTTP通道的总开关
    FEATURE_HTTP2_COMM_ENABLED   = n          # 是否打开HTTP/2通道的总开关
    FEATURE_SDK_ENHANCE          = y          # 是否打开Linkkit高级版功能的总开关
    FEATURE_SUPPORT_TLS          = y          # 选择TLS安全连接的开关，此开关与iTLS开关互斥
    FEATURE_SUPPORT_ITLS         = n          # 选择iTLS安全连接的开关，此开关与TLS开关互斥，ID2功能需要打开此开关
    FEATURE_SUBDEVICE_ENABLED    = n          # 是否打开主子设备功能的总开关
    FEATURE_ALCS_ENABLED         = y          # 是否打开本地通信功能的总开关
    FEATURE_OTA_ENABLED          = y          # 切换网关和单品的总开关

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

# <a name="硬件平台抽象层(HAL层)">硬件平台抽象层(HAL层)</a>

## <a name="设备端C-SDK分层">设备端C-SDK分层</a>

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

+ 所有HAL层函数的声明都在 `include/iot_import.h` 这个头文件中列出
+ 各功能点引入的HAL层接口依赖在`include/imports/iot_import_*.h`中列出
+ 这些`include/imports`目录下的子文件, 都被`include/iot_import.h`包含
+ 这个部分的函数声明, 对应在`src/ref-impl/hal/os/ubuntu/`有提供一份`Ubuntu`桌面系统上的参考实现, 会被编译成`output/release/lib/libiot_hal.a`

## <a name="HAL层接口详述">HAL层接口详述</a>

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


# <a name="C-SDK提供的功能API说明">C-SDK提供的功能API说明</a>

以下为C-SDK提供的功能和对应的API, 用于让用户编写业务逻辑, 封装AT命令时, 也是基于这些API

更加准确详细和权威的描述, 以代码`include/iot_export.h`, 以及`include/exports/*.h`中的注释为准

如何使用这些API编写应用逻辑, 以代码`examples/*/*.c`的示例程序为准

## <a name="API列表">API列表</a>
如下列出当前SDK代码提供的所有面向用户的API函数:

### <a name="基础版API">基础版API</a>

    $ cd include
    $ grep -o "IOT_[A-Z][_a-zA-Z]*[^_]\> *(" iot_export.h exports/*.h|sed 's!.*:\(.*\)(!\1!'|cat -n

     1  IOT_OpenLog
     2  IOT_CloseLog
     3  IOT_SetLogLevel
     4  IOT_DumpMemoryStats
     5  IOT_SetupConnInfo
     6  IOT_Ioctl
     7  IOT_ALCS_Construct
     8  IOT_ALCS_Cloud_Init
     9  IOT_ALCS_Destroy
    10  IOT_ALCS_Yield
    11  IOT_ALCS_Send
    12  IOT_ALCS_Send_Response
    13  IOT_ALCS_Register_Resource
    14  IOT_ALCS_Observe_Notify
    15  IOT_ALCS_Unregister_Resource
    16  IOT_ALCS_Add_Sub_Device
    17  IOT_ALCS_Remove_Sub_Device
    18  IOT_CoAP_Init
    19  IOT_CoAP_Deinit
    20  IOT_CoAP_DeviceNameAuth
    21  IOT_CoAP_Yield
    22  IOT_CoAP_SendMessage
    23  IOT_CoAP_GetMessagePayload
    24  IOT_CoAP_GetMessageCode
    25  IOT_HTTP_Init
    26  IOT_HTTP_DeInit
    27  IOT_HTTP_DeviceNameAuth
    28  IOT_HTTP_SendMessage
    29  IOT_HTTP_Disconnect
    30  IOT_Linkkit_Open
    31  IOT_Linkkit_Ioctl
    32  IOT_Linkkit_Start
    33  IOT_Linkkit_Yield
    34  IOT_Linkkit_Close
    35  IOT_Linkkit_Login
    36  IOT_Linkkit_Logout
    37  IOT_Linkkit_Post
    38  IOT_MQTT_Construct
    39  IOT_MQTT_Destroy
    40  IOT_MQTT_Yield
    41  IOT_MQTT_LogPost
    42  IOT_MQTT_CheckStateNormal
    43  IOT_MQTT_Subscribe
    44  IOT_MQTT_Unsubscribe
    45  IOT_MQTT_Publish
    46  IOT_OTA_Init
    47  IOT_OTA_Deinit
    48  IOT_OTA_ReportVersion
    49  IOT_OTA_RequestImage
    50  IOT_OTA_ReportProgress
    51  IOT_OTA_GetConfig
    52  IOT_OTA_IsFetching
    53  IOT_OTA_IsFetchFinish
    54  IOT_OTA_FetchYield
    55  IOT_OTA_Ioctl
    56  IOT_OTA_GetLastError
    57  IOT_Shadow_Construct
    58  IOT_Shadow_Destroy
    59  IOT_Shadow_Yield
    60  IOT_Shadow_RegisterAttribute
    61  IOT_Shadow_DeleteAttribute
    62  IOT_Shadow_PushFormat_Init
    63  IOT_Shadow_PushFormat_Add
    64  IOT_Shadow_PushFormat_Finalize
    65  IOT_Shadow_Push
    66  IOT_Shadow_Push_Async
    67  IOT_Shadow_Pull
    68  IOT_Gateway_Generate_Message_ID
    69  IOT_Gateway_Construct
    70  IOT_Gateway_Destroy
    71  IOT_Subdevice_Register
    72  IOT_Subdevice_Unregister
    73  IOT_Subdevice_Login
    74  IOT_Subdevice_Logout
    75  IOT_Gateway_Get_TOPO
    76  IOT_Gateway_Get_Config
    77  IOT_Gateway_Publish_Found_List
    78  IOT_Gateway_Yield
    79  IOT_Gateway_Subscribe
    80  IOT_Gateway_Unsubscribe
    81  IOT_Gateway_Publish
    82  IOT_Gateway_RRPC_Register
    83  IOT_Gateway_RRPC_Response

### <a name="linkkit单品API">linkkit单品API</a>

    $ cd include
    $ grep -o "linkkit_[_a-z]* *(" iot_export.h exports/*.h|sed 's!.*:\(.*\)(!\1!'|sort -u|grep -v gateway|cat -n

     1  linkkit_answer_service
     2  linkkit_cota_init
     3  linkkit_dispatch
     4  linkkit_end
     5  linkkit_fota_init
     6  linkkit_get_value
     7  linkkit_invoke_cota_get_config
     8  linkkit_invoke_cota_service
     9  linkkit_invoke_fota_service
    10  linkkit_invoke_raw_service
    11  linkkit_is_end
    12  linkkit_is_try_leave
    13  linkkit_post_property
    14  linkkit_set_opt
    15  linkkit_set_tsl
    16  linkkit_set_value
    17  linkkit_start
    18  linkkit_trigger_event
    19  linkkit_trigger_extended_info_operate
    20  linkkit_try_leave
    21  linkkit_yield

### <a name="linkkit网关API">linkkit网关API</a>

    $ cd include
    $ grep -o "linkkit_gateway_[_a-z]* *(" iot_export.h exports/*.h|sed 's!.*:\(.*\)(!\1!'|sort -u|cat -n

     1  linkkit_gateway_delete_extinfos
     2  linkkit_gateway_exit
     3  linkkit_gateway_fota_init
     4  linkkit_gateway_get_default_params
     5  linkkit_gateway_get_devinfo
     6  linkkit_gateway_get_num_devices
     7  linkkit_gateway_init
     8  linkkit_gateway_invoke_fota_service
     9  linkkit_gateway_post_extinfos
    10  linkkit_gateway_post_property_json
    11  linkkit_gateway_post_property_json_sync
    12  linkkit_gateway_post_rawdata
    13  linkkit_gateway_set_event_callback
    14  linkkit_gateway_setopt
    15  linkkit_gateway_start
    16  linkkit_gateway_stop
    17  linkkit_gateway_subdev_create
    18  linkkit_gateway_subdev_destroy
    19  linkkit_gateway_subdev_login
    20  linkkit_gateway_subdev_logout
    21  linkkit_gateway_subdev_register
    22  linkkit_gateway_subdev_unregister
    23  linkkit_gateway_trigger_event_json
    24  linkkit_gateway_trigger_event_json_sync

## <a name="API详细说明">API详细说明</a>

### <a name="必选API">必选API</a>

| 序号  | 函数名                       | 说明                                                               |
|-------|------------------------------|--------------------------------------------------------------------|
|  1    | IOT_OpenLog                  | 开始打印日志信息(log), 接受一个const char *为入参, 表示模块名字    |
|  2    | IOT_CloseLog                 | 停止打印日志信息(log), 入参为空                                    |
|  3    | IOT_SetLogLevel              | 设置打印的日志等级, 接受入参从1到5, 数字越大, 打印越详细           |
|  4    | IOT_DumpMemoryStats          | 调试函数, 打印内存的使用统计情况, 入参为1-5, 数字越大, 打印越详细  |
|  5    | IOT_Ioctl                    | 设置SDK运行时可配置选项，详情见API注释                          |

### <a name="CoAP功能相关">CoAP功能相关</a>

| 序号  | 函数名                       | 说明                                                                           |
|-------|------------------------------|--------------------------------------------------------------------------------|
|  1    | IOT_CoAP_Init                | CoAP实例的构造函数, 入参为`iotx_coap_config_t`结构体, 返回创建的CoAP会话句柄   |
|  2    | IOT_CoAP_Deinit              | CoAP实例的摧毁函数, 入参为`IOT_CoAP_Init()`所创建的句柄                        |
|  3    | IOT_CoAP_DeviceNameAuth      | 基于控制台申请的`DeviceName`, `DeviceSecret`, `ProductKey`做设备认证           |
|  4    | IOT_CoAP_GetMessageCode      | CoAP会话阶段, 从服务器的`CoAP Response`报文中获取`Respond Code`                |
|  5    | IOT_CoAP_GetMessagePayload   | CoAP会话阶段, 从服务器的`CoAP Response`报文中获取报文负载                      |
|  6    | IOT_CoAP_SendMessage         | CoAP会话阶段, 连接已成功建立后调用, 组织一个完整的CoAP报文向服务器发送         |
|  7    | IOT_CoAP_Yield               | CoAP会话阶段, 连接已成功建立后调用, 检查和收取服务器对`CoAP Request`的回复报文 |

### <a name="MQTT功能相关">MQTT功能相关</a>

| 序号  | 函数名                       | 说明                                                                           |
|-------|------------------------------|--------------------------------------------------------------------------------|
|  1    | IOT_SetupConnInfo            | MQTT连接前的准备, 基于`DeviceName + DeviceSecret + ProductKey`产生MQTT连接的用户名和密码等 |
|  2    | IOT_MQTT_CheckStateNormal    | MQTT连接后, 调用此函数检查长连接是否正常                                       |
|  3    | IOT_MQTT_Construct           | MQTT实例的构造函数, 入参为`iotx_mqtt_param_t`结构体, 连接MQTT服务器, 并返回被创建句柄 |
|  4    | IOT_MQTT_Destroy             | MQTT实例的摧毁函数, 入参为`IOT_MQTT_Construct()`创建的句柄                     |
|  5    | IOT_MQTT_Publish             | MQTT会话阶段, 组织一个完整的`MQTT Publish`报文, 向服务端发送消息发布报文       |
|  6    | IOT_MQTT_Subscribe           | MQTT会话阶段, 组织一个完整的`MQTT Subscribe`报文, 向服务端发送订阅请求         |
|  7    | IOT_MQTT_Unsubscribe         | MQTT会话阶段, 组织一个完整的`MQTT UnSubscribe`报文, 向服务端发送取消订阅请求   |
|  8    | IOT_MQTT_Yield               | MQTT会话阶段, MQTT主循环函数, 内含了心跳的维持, 服务器下行报文的收取等         |

### <a name="OTA功能相关">OTA功能相关</a>

| 序号  | 函数名                       | 说明                                                                                   |
|-------|------------------------------|----------------------------------------------------------------------------------------|
|  1    | IOT_OTA_Init                 | OTA实例的构造函数, 创建一个OTA会话的句柄并返回                                         |
|  2    | IOT_OTA_Deinit               | OTA实例的摧毁函数, 销毁所有相关的数据结构                                              |
|  3    | IOT_OTA_Ioctl                | OTA实例的输入输出函数, 根据不同的命令字可以设置OTA会话的属性, 或者获取OTA会话的状态    |
|  4    | IOT_OTA_GetLastError         | OTA会话阶段, 若某个`IOT_OTA_*()`函数返回错误, 调用此接口可获得最近一次的详细错误码     |
|  5    | IOT_OTA_ReportVersion        | OTA会话阶段, 向服务端汇报当前的固件版本号                                              |
|  6    | IOT_OTA_FetchYield           | OTA下载阶段, 在指定的`timeout`时间内, 从固件服务器下载一段固件内容, 保存在入参buffer中 |
|  7    | IOT_OTA_IsFetchFinish        | OTA下载阶段, 判断迭代调用`IOT_OTA_FetchYield()`是否已经下载完所有的固件内容            |
|  8    | IOT_OTA_IsFetching           | OTA下载阶段, 判断固件下载是否仍在进行中, 尚未完成全部固件内容的下载                    |
|  9    | IOT_OTA_ReportProgress       | 可选API, OTA下载阶段, 调用此函数向服务端汇报已经下载了全部固件内容的百分之多少         |
|  10   | IOT_OTA_RequestImage         | 可选API，向服务端请求固件下载                                                          |
|  11   | IOT_OTA_GetConfig            | 可选API，向服务端请求远程配置                                                          |

### <a name="HTTP功能相关">HTTP功能相关</a>

| 序号  | 函数名                       | 说明                                                                                   |
|-------|------------------------------|----------------------------------------------------------------------------------------|
|  1    | IOT_HTTP_Init                | Https实例的构造函数, 创建一个HTTP会话的句柄并返回                                      |
|  2    | IOT_HTTP_DeInit              | Https实例的摧毁函数, 销毁所有相关的数据结构                                            |
|  3    | IOT_HTTP_DeviceNameAuth      | 基于控制台申请的`DeviceName`, `DeviceSecret`, `ProductKey`做设备认证                   |
|  4    | IOT_HTTP_SendMessage         | Https会话阶段, 组织一个完整的HTTP报文向服务器发送,并同步获取HTTP回复报文               |
|  5    | IOT_HTTP_Disconnect          | Https会话阶段, 关闭HTTP层面的连接, 但是仍然保持TLS层面的连接                           |

### <a name="设备影子相关">设备影子相关</a>

| 序号  | 函数名                          | 说明                                                                            |
|-------|---------------------------------|---------------------------------------------------------------------------------|
|  1    | IOT_Shadow_Construct            | 建立一个设备影子的MQTT连接, 并返回被创建的会话句柄                              |
|  2    | IOT_Shadow_Destroy              | 摧毁一个设备影子的MQTT连接, 销毁所有相关的数据结构, 释放内存, 断开连接          |
|  3    | IOT_Shadow_Pull                 | 把服务器端被缓存的JSON数据下拉到本地, 更新本地的数据属性                        |
|  4    | IOT_Shadow_Push                 | 把本地的数据属性上推到服务器缓存的JSON数据, 更新服务端的数据属性                |
|  5    | IOT_Shadow_Push_Async           | 和`IOT_Shadow_Push()`接口类似, 但是异步的, 上推后便返回, 不等待服务端回应       |
|  6    | IOT_Shadow_PushFormat_Add       | 向已创建的数据类型格式中增添成员属性                                            |
|  7    | IOT_Shadow_PushFormat_Finalize  | 完成一个数据类型格式的构造过程                                                  |
|  8    | IOT_Shadow_PushFormat_Init      | 开始一个数据类型格式的构造过程                                                  |
|  9    | IOT_Shadow_RegisterAttribute    | 创建一个数据类型注册到服务端, 注册时需要`*PushFormat*()`接口创建的数据类型格式  |
| 10    | IOT_Shadow_DeleteAttribute      | 删除一个已被成功注册的数据属性                                                  |
| 11    | IOT_Shadow_Yield                | MQTT的主循环函数, 调用后接受服务端的下推消息, 更新本地的数据属性                |

### <a name="主子设备相关(老版本接口，不推荐使用)">主子设备相关(老版本接口，不推荐使用)</a>

| 序号  | 函数名                          | 说明                                                                            |
|-------|---------------------------------|---------------------------------------------------------------------------------|
|  1    | IOT_Gateway_Construct           | 建立一个主设备，建立MQTT连接, 并返回被创建的会话句柄                            |
|  2    | IOT_Gateway_Destroy             | 摧毁一个主设备的MQTT连接, 销毁所有相关的数据结构, 释放内存, 断开连接            |
|  3    | IOT_Subdevice_Login             | 子设备上线，通知云端建立子设备session                                           |
|  4    | IOT_Subdevice_Logout            | 子设备下线，销毁云端建立子设备session及所有相关的数据结构, 释放内存             |
|  5    | IOT_Gateway_Yield               | MQTT的主循环函数, 调用后接受服务端的下推消息                                    |
|  6    | IOT_Gateway_Subscribe           | 通过MQTT连接向服务端发送订阅请求                                                |
|  7    | IOT_Gateway_Unsubscribe         | 通过MQTT连接向服务端发送取消订阅请求                                            |
|  8    | IOT_Gateway_Publish             | 通过MQTT连接服务端发送消息发布报文                                              |
|  9    | IOT_Gateway_RRPC_Register       | 注册设备的RRPC回调函数，接收云端发起的RRPC请求                                  |
| 10    | IOT_Gateway_RRPC_Response       | 对云端的RRPC请求进行应答                                                        |
| 11    | IOT_Gateway_Generate_Message_ID | 生成消息id                                                                      |
| 12    | IOT_Gateway_Get_TOPO            | 向topo/get topic发送包并等待回复（TOPIC_GET_REPLY 回复）                        |
| 13    | IOT_Gateway_Get_Config          | 向conifg/get topic发送包并等待回复（TOPIC_CONFIG_REPLY 回复）                   |
| 14    | IOT_Gateway_Publish_Found_List  | 发现设备列表上报                                                                |

### <a name="linkkit单品相关">linkkit单品相关</a>

| 序号  | 函数名                          | 说明                                                                            |
|-------|---------------------------------|---------------------------------------------------------------------------------|
|  1    | linkkit_start                   | 启动 linkkit 服务，与云端建立连接并安装回调函数                                 |
|  2    | linkkit_end                     | 停止 linkkit 服务，与云端断开连接并回收资源                                     |
|  3    | linkkit_dispatch                | 事件分发函数，触发 linkkit_start 安装的回调                                     |
|  4    | linkkit_yield                   | linkkit 主循环函数，内含了心跳的维持，服务器下行报文的收取等;如果允许多线程，请不要调用此函数     |
|  5    | linkkit_set_value               | 根据identifier设置物对象的 TSL 属性，如果标识符为struct类型、event output类型或者service output类型，使用点'.'分隔字段；例如"identifier1.identifier2"指向特定的项    |
|  6    | linkkit_get_value               | 根据identifier获取物对象的 TSL 属性                                             |
|  7    | linkkit_set_tsl                 | 从本地读取 TSL 文件,生成物的对象并添加到 linkkit 中                             |
|  8    | linkkit_answer_service          | 对云端服务请求进行回应                                                          |
|  9    | linkkit_invoke_raw_service      | 向云端发送裸数据                                                                |
| 10    | linkkit_trigger_event           | 上报设备事件到云端                                                              |
| 11    | linkkit_fota_init               | 初始化 OTA-fota 服务，并安装回调函数(需编译设置宏 SERVICE_OTA_ENABLED )         |
| 12    | linkkit_invoke_fota_service     | 执行fota服务                                                                    |
| 13    | linkkit_cota_init               | 初始化 OTA-cota 服务，并安装回调函数(需编译设置宏 SERVICE_OTA_ENABLED SERVICE_COTA_ENABLED )     |
| 14    | linkkit_invoke_cota_get_config  | 设备请求远程配置                                                                |
| 15    | linkkit_invoke_cota_service     | 执行cota服务                                                                    |
| 16    | linkkit_post_property           | 上报设备属性到云端                                                              |
| 17    | linkkit_set_opt                 | 设置设备属性和服务上报参数                                                      |
| 18    | linkkit_try_leave               | 设置linkkit离开标志                                                             |
| 19    | linkkit_is_try_leave            | 获取linkkit离开标志                                                             |
| 20    | linkkit_is_end                  | 获取linkkit结束标志                                                             |
| 21    | linkkit_trigger_extended_info_operate | 设备拓展信息上报或删除                                                    |

### <a name="linkkit_gateway网关相关">linkkit_gateway网关相关</a>
| 序号  | 函数名                          | 说明                                                                            |
|-------|---------------------------------|---------------------------------------------------------------------------------|
|  1    | linkkit_gateway_get_default_params       | 获取默认的网关配置参数<br>返回值：默认参数结构体指针，将作为`linkkit_gateway_init`的入参用于初始化网关
|  2    | linkkit_gateway_setopt                   | 修改网关配置参数
|  3    | linkkit_gateway_set_event_callback       | 注册网关事件回调函数，加载用户数据 
|  4    | linkkit_gateway_init                     | 网关初始化，
|  5    | linkkit_gateway_exit                     | 网关反初始化
|  6    | linkkit_gateway_start                    | 启动网关服务，与云端服务器建立连接
|  7    | linkkit_gateway_stop                     | 停止网关服务，与云端服务器断开连接
|  8    | linkkit_gateway_subdev_register          | 向云端注册productKey、deviceName指定的子设备，并将子设备加入网关的拓扑关系
|  9    | linkkit_gateway_subdev_unregister        | 向云端注销productKey、deviceName指定的子设备，并将子设备从网关的拓扑关系移除
|  10   | linkkit_gateway_subdev_create            | 创建子设备，并注册用户回调函数，载入用户数据
|  11   | linkkit_gateway_subdev_destroy           | 删除子设备，回收资源
|  12   | linkkit_gateway_subdev_login             | 子设备上线，云端将可以访问子设备
|  13   | linkkit_gateway_subdev_logout            | 子设备下线，云端将无法访问子设备
|  14   | linkkit_gateway_get_devinfo              | 获取设备信息
|  15   | linkkit_gateway_trigger_event_json_sync  | 上报网关或子设备事件，同步接口
|  16   | linkkit_gateway_trigger_event_json       | 上报网关或子设备事件，异步接口，上报处理结束将调用用户回调函数
|  17   | linkkit_gateway_post_property_json_sync  | 上报网关或子设备属性，同步接口
|  18   | linkkit_gateway_post_property_json       | 上报网关或子设备属性，异步接口，上报处理结束将调用用户回调函数
|  19   | linkkit_gateway_post_rawdata             | 上报网关或子设备的裸数据
|  20   | linkkit_gateway_fota_init                | FOTA服务初始化，并注册回调函数
|  21   | linkkit_gateway_invoke_fota_service      | 执行FOTA服务
|  22   | linkkit_gateway_post_extinfos            | 上报拓展信息，拓展信息在控制台上以标签信息呈现
|  23   | linkkit_gateway_delete_extinfos          | 删除拓展信息，
|  24   | linkkit_gateway_get_num_devices          | 获取注册到SDK的设备总数，包括网关和所有子设备

网关API详细调用方法请查看`examples/linkkit/linkkit_example_gateway.c`

# <a name="应用场景说明">应用场景说明</a>
## <a name="1. MQTT站点配置">1. MQTT站点配置</a>
在使用阿里云物联网套件连接阿里云时，需要指定MQTT需要连接的站点，在基础版和高级版中，站点配置方法如下：

### <a name="基础版">基础版</a>
在`iotx-sdk-c/include/iot_export.h`中，枚举类型`iotx_cloud_domain_types_t`定义了当前可连接的MQTT站点：

    /* domain type */
    typedef enum IOTX_CLOUD_DOMAIN_TYPES {
        /* Shanghai */
        IOTX_CLOUD_DOMAIN_SH,
    
        /* Singapore */
        IOTX_CLOUD_DOMAIN_SG,
    
        /* Japan */
        IOTX_CLOUD_DOMAIN_JP,
    
        /* America */
        IOTX_CLOUD_DOMAIN_US,
    
        /* Germany */
        IOTX_CLOUD_DOMAIN_GER,
    
        /* Maximum number of domain */
        IOTX_CLOUD_DOMAIN_MAX
    } iotx_cloud_domain_types_t;
    
在使用基础版MQTT接口连接阿里云时，使用`IOT_Ioctl`的`IOTX_IOCTL_SET_DOMAIN`选项设置要连接的站点，然后再使用`IOT_SetupConnInfo`来建立设备到阿里云的连接

### <a name="高级版">高级版</a>
在`iotx-sdk-c/include/exports/linkkit_export.h`中，枚举类型`linkkit_cloud_domain_type_t`定义了当前可连接的MQTT站点：

    typedef enum {
        /* shanghai */
        linkkit_cloud_domain_shanghai,
        /* singapore */
        linkkit_cloud_domain_singapore,
        /* japan */
        linkkit_cloud_domain_japan,
        /* america */
        linkkit_cloud_domain_america,
        /* germany */
        linkkit_cloud_domain_germany,
    
        linkkit_cloud_domain_max,
    } linkkit_cloud_domain_type_t;
    
在使用高级版接口连接阿里云时，在`linkkit_start`中传入需要连接的站点即可。

- 注意事项：如果在阿里云物联网控制台申请的三元组与连接时使用的域名不符合，连接站点时会出现认证错误（错误码-35）。
*例如：在阿里云物联网控制台的华东2站点申请的三元组，在物联网套件中应该连接华东2（上海）站点*
            
## <a name="2. TSL静态集成和动态拉取">2. TSL静态集成和动态拉取</a>

物模型指将物理空间中的实体数字化，并在云端构建该实体的数据模型。在物联网平台中，定义物模型即定义产品功能。完成功能定义后，系统将自动生成该产品的物模型。

物模型描述产品是什么，能做什么，可以对外提供哪些服务。

物模型以 JSON 格式表述，称之为 TSL（即 Thing Specification Language）。在产品的功能定义页面，单击查看物模型即可查看 JSON 格式的 TSL：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/LP-%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86-%E4%BA%A7%E5%93%81%E8%AF%A6%E6%83%85-%E7%89%A9%E6%A8%A1%E5%9E%8B.png)

此时有两种方式在物联网套件中使用TSL：

- 静态集成：指将TSL直接写到代码中，这时需要上图中选择导出物模型，将设备的TSL（JSON格式）导出到文件，并手动集成到代码中
- 动态拉取：指在物联网套件运行时去云端拉取TSL

### <a name="TSL静态集成">TSL静态集成</a>
如上所述，将设备的TSL（JSON格式）导出到文件后（默认文件名为model.json），由于C语言需要将字符串中的双引号`"`进行转义，所以需要将导出的TSL文件进行转义。

在网上有很多这样的转义工具，物联网套件中也自带了一个可以转义JSON的小工具。

当SDK编译完成后，使用`iotx-sdk-c/output/release/bin`目录下的`linkkit_tsl_convert`可以完成该转义任务。使用方法如下：

     $./linkkit_tsl_convert -i model.json

上述命令执行完成后，会在调用命令的当前目录生成一个`conv.txt`文件，里面就是转义后的JSON字符串。目前仅高级版单品支持静态集成TSL，集成方法如下：

高级版单品使用TSL静态集成
---
默认情况下，物联网套件编译单品版本的example，源代码位于`iotx-sdk-c/examples/linkkit`目录下的`linkkit_example_solo.c`，同目录下的`example_tsl_solo.data`为存放静态集成TSL的文件，
将刚才`conv.txt`中的内容复制，替换`example_tsl_solo.data`文件中的`TSL_STRING`变量，重新编译即可（不要忘了将`linkkit_example_solo.c`中的三元组替换成该TSL对应产品下设备的三元组）。

### <a name="TSL动态拉取">TSL动态拉取</a>
如果使用动态拉取TSL，就不用去替换代码中的静态TSL。

需要注意的是，使用动态拉取TSL时，在TSL占用空间较大（TSL定义的服务、属性、事件越多，TSL越大）的情况下，有可能需要更改物联网套件默认的MQTT接收buffer长度。

可以按如下两种方法进行配置更改：

- 修改平台配置文件，在当前使用平台的config.xxx.xxx文件（位于iotx-sdk-c/src/board目录下）中，修改`CONFIG_ENV_CFLAGS`中的`CONFIG_MQTT_RX_MAXLEN`编译选项。

> 以ubuntu为例，如下图所示：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/iotkit-MQTT%E9%BB%98%E8%AE%A4%E6%8E%A5%E6%94%B6%E9%95%BF%E5%BA%A6%E5%9C%A8Makefile%E4%B8%AD%E7%9A%84%E9%85%8D%E7%BD%AE.png)

- 在代码中修改默认值，在`iotx-sdk-c/include/imports/iot_import_config.h`文件中修改`CONFIG_MQTT_RX_MAXLEN`即可，如下图所示：

![image](https://linkkit-export.oss-cn-shanghai.aliyuncs.com/iotkit-MQTT%E9%BB%98%E8%AE%A4%E6%8E%A5%E6%94%B6%E9%95%BF%E5%BA%A6%E5%9C%A8iot_import_config.h%E4%B8%AD%E7%9A%84%E9%85%8D%E7%BD%AE.png)

- 注意事项：在Makefile中设置的`CONFIG_MQTT_RX_MAXLEN`值会覆盖掉`iot_import_config.h`中设置的值

#### <a name="高级版单品使用TSL动态拉取">高级版单品使用TSL动态拉取</a>
默认情况下，在高级版单品的example（源代码位于`iotx-sdk-c/examples/linkkit`目录下的`linkkit_example_solo.c`）中，使用的是TSL静态集成。

若要使用TSL动态拉取，只需要将`linkkit_start`的第二个入参`get_tsl_from_cloud`设为`1`即可。

#### <a name="高级版网关使用TSL动态拉取">高级版网关使用TSL动态拉取</a>
目前，高级版网关仅使用TSL动态拉取。
