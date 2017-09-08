## 编译配置项说明

**请先下载最新版本设备端C语言SDK** [SDK下载](https://help.aliyun.com/document_detail/42648.html)

解压之后, 打开编译配置文件`make.settings`, 根据需要编辑配置项:

    BUILD_TYPE                  = debug

    PLATFORM_CC                 = gcc
    PLATFORM_AR                 = ar
    PLATFORM_OS                 = linux

    #
    # Uncomment below and specify PATH to your toolchain when cross-compile SDK
    #
    # PLATFORM_CC = /path/to/your/cross/gcc
    # PLATFORM_AR = /path/to/your/cross/ar

    FEATURE_MQTT_COMM_ENABLED   = y     # 是否打开MQTT通道的总开关
    FEATURE_MQTT_DIRECT         = y     # 是否打开MQTT直连的分开关
    FEATURE_MQTT_DIRECT_NOTLS   = n     # 是否打开MQTT直连无TLS的分开关
    FEATURE_COAP_COMM_ENABLED   = y     # 是否打开CoAP通道的总开关
    FEATURE_HTTP_COMM_ENABLED   = y     # 是否打开HTTP通道的总开关

具体含义参见下表:

| 配置选项                    | 含义                                                            |
|-----------------------------|-----------------------------------------------------------------|
| BUILD_TYPE                  | 指定编译SDK的版本类型, 支持`debug`, `release`                   |
| PLATFORM_CC                 | C源码编译器, 若交叉编译, 请保证交叉的`gcc/ar/strip`在同一目录   |
| PLATFORM_AR                 | 静态库压缩器, 若交叉编译, 请保证交叉的`gcc/ar/strip`在同一目录  |
| PLATFORM_OS                 | 指定目标平台的操作系统, SDK中有含一份目标平台为桌面linux的示例  |
| FEATURE_MQTT_COMM_ENABLED   | 是否使能MQTT通道功能的总开关                                    |
| FEATURE_MQTT_DIRECT         | 是否用MQTT直连模式代替HTTPS三方认证模式做设备认证               |
| FEATURE_MQTT_DIRECT_NOTLS   | 使用MQTT直连模式做设备认证时, 是否要关闭MQTT over TLS           |
| FEATURE_COAP_COMM_ENABLED   | 是否使能CoAP通道功能的总开关                                    |
| FEATURE_HTTP_COMM_ENABLED   | 是否使能Https通道功能的总开关                                   |


## 编译 & 运行
请参考[设备端快速开始](https://help.aliyun.com/document_detail/30530.html)

## C-SDK提供的功能API说明

以下为华东2站点V2.0版本的C-SDK提供的功能和对应的API, 用于让用户编写业务逻辑, 封装AT命令时, 也是基于这些API

更加准确详细和权威的描述, 以代码`src/sdk-impl/iot_export.h`, 以及`src/sdk-impl/exports/*.h`中的注释为准

如何使用这些API编写应用逻辑, 以代码`sample/*/*.c`的示例程序为准

**必选API**

| 序号  | 函数名                       | 说明                                                               |
|-------|------------------------------|--------------------------------------------------------------------|
|  1    | IOT_OpenLog                  | 开始打印日志信息(log), 接受一个const char *为入参, 表示模块名字    |
|  2    | IOT_SetLogLevel              | 设置打印的日志等级, 接受入参从1到5, 数字越大, 打印越详细           |
|  3    | IOT_CloseLog                 | 停止打印日志信息(log), 入参为空                                    |
|  4    | IOT_DumpMemoryStats          | 调试函数, 打印内存的使用统计情况, 入参为1-5, 数字越大, 打印越详细  |

**CoAP功能相关**

| 序号  | 函数名                       | 说明                                                                           |
|-------|------------------------------|--------------------------------------------------------------------------------|
|  1    | IOT_CoAP_Init                | CoAP实例的构造函数, 入参为`iotx_coap_config_t`结构体, 返回创建的CoAP会话句柄   |
|  2    | IOT_CoAP_Deinit              | CoAP实例的摧毁函数, 入参为`IOT_CoAP_Init()`所创建的句柄                        |
|  3    | IOT_CoAP_DeviceNameAuth      | 基于控制台申请的`DeviceName`, `DeviceSecret`, `ProductKey`做设备认证           |
|  4    | IOT_CoAP_GetMessageCode      | CoAP会话阶段, 从服务器的`CoAP Response`报文中获取`Respond Code`                |
|  5    | IOT_CoAP_GetMessagePayload   | CoAP会话阶段, 从服务器的`CoAP Response`报文中获取报文负载                      |
|  6    | IOT_CoAP_SendMessage         | CoAP会话阶段, 连接已成功建立后调用, 组织一个完整的CoAP报文向服务器发送         |
|  7    | IOT_CoAP_Yield               | CoAP会话阶段, 连接已成功建立后调用, 检查和收取服务器对`CoAP Request`的回复报文 |

**MQTT功能相关**

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

**OTA功能相关(模组实现时的可选功能)**

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

**HTTP功能相关**

| 序号  | 函数名                       | 说明                                                                                   |
|-------|------------------------------|----------------------------------------------------------------------------------------|
|  1    | IOT_HTTP_Init                | Https实例的构造函数, 创建一个HTTP会话的句柄并返回                                      |
|  2    | IOT_HTTP_DeInit              | Https实例的摧毁函数, 销毁所有相关的数据结构                                            |
|  3    | IOT_HTTP_DeviceNameAuth      | 基于控制台申请的`DeviceName`, `DeviceSecret`, `ProductKey`做设备认证                   |
|  4    | IOT_HTTP_SendMessage         | Https会话阶段, 组织一个完整的HTTP报文向服务器发送,并同步获取HTTP回复报文               |

**设备影子相关(模组实现时的可选功能)**

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

