# <a name="目录">目录</a>
+ [编译配置项说明](#编译配置项说明)
+ [编译和运行](#编译和运行)
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

# <a name="编译配置项说明">编译配置项说明</a>

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

# <a name="编译和运行">编译和运行</a>
请参考`Cross_Build.md`

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
     6  IOT_SetupConnInfoSecure
     7  IOT_SetupDomain
     8  IOT_ALCS_Construct
     9  IOT_ALCS_Cloud_Init
    10  IOT_ALCS_Destroy
    11  IOT_ALCS_Yield
    12  IOT_ALCS_Send
    13  IOT_ALCS_Send_Response
    14  IOT_ALCS_Register_Resource
    15  IOT_ALCS_Observe_Notify
    16  IOT_ALCS_Unregister_Resource
    17  IOT_ALCS_Add_Sub_Device
    18  IOT_ALCS_Remove_Sub_Device
    19  IOT_CoAP_Init
    20  IOT_CoAP_Deinit
    21  IOT_CoAP_DeviceNameAuth
    22  IOT_CoAP_Yield
    23  IOT_CoAP_SendMessage
    24  IOT_CoAP_GetMessagePayload
    25  IOT_CoAP_GetMessageCode
    26  IOT_HTTP_Init
    27  IOT_HTTP_DeInit
    28  IOT_HTTP_DeviceNameAuth
    29  IOT_HTTP_SendMessage
    30  IOT_HTTP_Disconnect
    31  IOT_MQTT_Construct
    32  IOT_MQTT_ConstructSecure
    33  IOT_MQTT_Destroy
    34  IOT_MQTT_Yield
    35  IOT_MQTT_CheckStateNormal
    36  IOT_MQTT_Subscribe
    37  IOT_MQTT_Unsubscribe
    38  IOT_MQTT_Publish
    39  IOT_OTA_Init
    40  IOT_OTA_Deinit
    41  IOT_OTA_ReportVersion
    42  IOT_OTA_RequestImage
    43  IOT_OTA_ReportProgress
    44  IOT_OTA_GetConfig
    45  IOT_OTA_IsFetching
    46  IOT_OTA_IsFetchFinish
    47  IOT_OTA_FetchYield
    48  IOT_OTA_Ioctl
    49  IOT_OTA_GetLastError
    50  IOT_Shadow_Construct
    51  IOT_Shadow_Destroy
    52  IOT_Shadow_Yield
    53  IOT_Shadow_RegisterAttribute
    54  IOT_Shadow_DeleteAttribute
    55  IOT_Shadow_PushFormat_Init
    56  IOT_Shadow_PushFormat_Add
    57  IOT_Shadow_PushFormat_Finalize
    58  IOT_Shadow_Push
    59  IOT_Shadow_Push_Async
    60  IOT_Shadow_Pull
    61  IOT_Gateway_Generate_Message_ID
    62  IOT_Gateway_Construct
    63  IOT_Gateway_Destroy
    64  IOT_Subdevice_Register
    65  IOT_Subdevice_Unregister
    66  IOT_Subdevice_Login
    67  IOT_Subdevice_Logout
    68  IOT_Gateway_Get_TOPO
    69  IOT_Gateway_Get_Config
    70  IOT_Gateway_Publish_Found_List
    71  IOT_Gateway_Yield
    72  IOT_Gateway_Subscribe
    73  IOT_Gateway_Unsubscribe
    74  IOT_Gateway_Publish
    75  IOT_Gateway_RRPC_Register
    76  IOT_Gateway_RRPC_Response

### <a name="linkkit单品API">linkkit单品API</a>

    $ cd include
    $ grep -o "linkkit_[_a-z]* *(" iot_export.h exports/*.h|sed 's!.*:\(.*\)(!\1!'|sort -u|grep -v gateway|cat -n

     1  linkkit_answer_service
     2  linkkit_cota_init
     3  linkkit_dispatch
     4  linkkit_end
     5  linkkit_fota_init
     6  linkkit_get_default_params
     7  linkkit_get_value
     8  linkkit_invoke_cota_get_config
     9  linkkit_invoke_cota_service
    10  linkkit_invoke_fota_service
    11  linkkit_invoke_raw_service
    12  linkkit_is_end
    13  linkkit_is_try_leave
    14  linkkit_post_property
    15  linkkit_post_rawdata
    16  linkkit_set_opt
    17  linkkit_set_tsl
    18  linkkit_set_value
    19  linkkit_start
    20  linkkit_subdev_create
    21  linkkit_trigger_event
    22  linkkit_trigger_extended_info_operate
    23  linkkit_try_leave
    24  linkkit_yield

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
|  5    | IOT_SetupDomain              | 设置云服务器域名，目前支持国内，新加坡，日本，美国，德国等区域     |

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
|  2    | IOT_SetupConnInfoSecure      | MQTT连接前的准备, 基于`ID2 + DeviceSecret + ProductKey`产生MQTT连接的用户名和密码等,ID2模式启用|
|  3    | IOT_MQTT_CheckStateNormal    | MQTT连接后, 调用此函数检查长连接是否正常                                       |
|  4    | IOT_MQTT_Construct           | MQTT实例的构造函数, 入参为`iotx_mqtt_param_t`结构体, 连接MQTT服务器, 并返回被创建句柄 |
|  5    | IOT_MQTT_ConstructSecure     | MQTT实例的构造函数, 入参为`iotx_mqtt_param_t`结构体, 连接MQTT服务器, 并返回被创建句柄，ID2模式启用|
|  6    | IOT_MQTT_Destroy             | MQTT实例的摧毁函数, 入参为`IOT_MQTT_Construct()`创建的句柄                     |
|  7    | IOT_MQTT_Publish             | MQTT会话阶段, 组织一个完整的`MQTT Publish`报文, 向服务端发送消息发布报文       |
|  8    | IOT_MQTT_Subscribe           | MQTT会话阶段, 组织一个完整的`MQTT Subscribe`报文, 向服务端发送订阅请求         |
|  9    | IOT_MQTT_Unsubscribe         | MQTT会话阶段, 组织一个完整的`MQTT UnSubscribe`报文, 向服务端发送取消订阅请求   |
|  10   | IOT_MQTT_Yield               | MQTT会话阶段, MQTT主循环函数, 内含了心跳的维持, 服务器下行报文的收取等         |

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
