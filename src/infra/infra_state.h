/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __INFRA_STATE_H__
#define __INFRA_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define STATE_BASE                                  (0x0000)

/* General: 0x0000 ~ 0x00FF */
/* API works as expected and returns sucess */
/* API 接口按预期完成工作, 并返回成功 */
#define STATE_SUCCESS                               (STATE_BASE - 0x0000)

/* General: 0x0000 ~ 0x00FF */

/* User Input: 0x0100 ~ 0x01FF */
#define STATE_USER_INPUT_BASE                       (-0x0100)

/* User input parameters contain unacceptable NULL pointer */
/* 用户传递给API的参数中含有不合理的空指针参数 */
#define STATE_USER_INPUT_NULL_POINTER               (STATE_USER_INPUT_BASE - 0x0001)
/* Some user input parameter(s) has value out of acceptable range */
/* 用户传递给API的参数中含有超出合理范围的取值 */
#define STATE_USER_INPUT_OUT_RANGE                  (STATE_USER_INPUT_BASE - 0x0002)
/* User input parameters contain unacceptable productKey */
/* 用户传递给API的参数中含有不合法的产品标识(productKey) */
#define STATE_USER_INPUT_PK                         (STATE_USER_INPUT_BASE - 0x0003)
/* User input parameters contain unacceptable productSecret */
/* 用户传递给API的参数中含有不合法的产品密钥(productSecret) */
#define STATE_USER_INPUT_PS                         (STATE_USER_INPUT_BASE - 0x0004)
/* User input parameters contain unacceptable deviceName */
/* 用户传递给API的参数中含有不合法的设备名称(deviceName) */
#define STATE_USER_INPUT_DN                         (STATE_USER_INPUT_BASE - 0x0005)
/* User input parameters contain unacceptable deviceSecret */
/* 用户传递给API的参数中含有不合法的设备密钥(deviceSecret) */
#define STATE_USER_INPUT_DS                         (STATE_USER_INPUT_BASE - 0x0006)
/* User input parameters contain unacceptable HTTP domain name */
/* 用户传递给API的参数中含有不合理的域名, SDK无法对其完成HTTP连接 */
#define STATE_USER_INPUT_HTTP_DOMAIN                (STATE_USER_INPUT_BASE - 0x0007)
/* User input parameters contain unacceptable MQTT domain name */
/* 用户传递给API的参数中含有不合理的域名, SDK无法对其完成MQTT连接 */
#define STATE_USER_INPUT_MQTT_DOMAIN                (STATE_USER_INPUT_BASE - 0x0008)
/* User input parameters contain unacceptable HTTP port */
/* 用户传递给API的参数中含有不合理的HTTP目的端口号 */
#define STATE_USER_INPUT_HTTP_PORT                  (STATE_USER_INPUT_BASE - 0x0009)
/* User input parameters contain unacceptable HTTP timeout */
/* 用户传递给API的参数中含有不合理的HTTP超时时间 */
#define STATE_USER_INPUT_HTTP_TIMEOUT               (STATE_USER_INPUT_BASE - 0x000A)
/* User input parameters contain unacceptable HTTP options */
/* 用户传递给API的参数中含有不合理的HTTP连接选项 */
#define STATE_USER_INPUT_HTTP_OPTION                (STATE_USER_INPUT_BASE - 0x000B)
/* User input parameters contain unacceptable post data */
/* 用户传递给API的参数中, 对HTTP POST含有不合理的报文描述, 如长度为负或空指针等 */
#define STATE_USER_INPUT_HTTP_POST_DATA             (STATE_USER_INPUT_BASE - 0x000C)
/* User input parameters contain unacceptable HTTP URL */
/* 用户传递给API的参数中含有不合理的HTTP目的URL */
#define STATE_USER_INPUT_HTTP_URL                   (STATE_USER_INPUT_BASE - 0x000D)
/* User input parameters contain unacceptable HTTP path */
/* 用户传递给API的参数中含有不合理的HTTP资源路径 */
#define STATE_USER_INPUT_HTTP_PATH                  (STATE_USER_INPUT_BASE - 0x000E)
/* User input parameters contain unacceptable PK/PS/DN/DS */
/* 用户传递给API的参数中含有不合理的产品标识/密钥或设备名称/密钥 */
#define STATE_USER_INPUT_META_INFO                  (STATE_USER_INPUT_BASE - 0x000F)
/* User input parameters contain unacceptable deviceID */
/* 用户传递给API的参数中含有不合理的 deviceID 设备标识符 */
#define STATE_USER_INPUT_DEVID                      (STATE_USER_INPUT_BASE - 0x0010)
/* User input parameters contain unacceptable device type */
/* 用户传递给API的参数中含有不合理的设备类型, 既不是master又不是slave */
#define STATE_USER_INPUT_DEVICE_TYPE                (STATE_USER_INPUT_BASE - 0x0011)

/* User input parameters contain unacceptable value */
/* 用户传递给API的参数中含有其它不合理的取值 */
#define STATE_USER_INPUT_INVALID                    (STATE_USER_INPUT_BASE - 0x0012)
/* User Input: 0x0100 ~ 0x01FF */

/* System: 0x0200 ~ 0x02FF */
#define STATE_SYS_DEPEND_BASE                       (-0x0200)

/* SDK run into exception when invoking HAL_Malloc() */
/* SDK调用的系统适配接口 HAL_Malloc() 返回异常, 未能成功分配内存 */
#define STATE_SYS_DEPEND_MALLOC                     (STATE_SYS_DEPEND_BASE - 0x0001)
/* SDK run into exception when invoking HAL_Kv_Get() */
/* SDK调用的系统适配接口 HAL_Kv_Get() 返回异常, 未能成功根据Key获取Value */
#define STATE_SYS_DEPEND_KV_GET                     (STATE_SYS_DEPEND_BASE - 0x0002)
/* SDK run into exception when invoking HAL_Kv_Set() */
/* SDK调用的系统适配接口 HAL_Kv_Set() 返回异常, 未能成功根据Key写入Value */
#define STATE_SYS_DEPEND_KV_SET                     (STATE_SYS_DEPEND_BASE - 0x0003)
/* SDK run into exception when invoking HAL_Kv_Del() */
/* SDK调用的系统适配接口 HAL_Kv_Del() 返回异常, 未能成功根据Key删除KV对 */
#define STATE_SYS_DEPEND_KV_DELETE                  (STATE_SYS_DEPEND_BASE - 0x0004)
/* SDK run into exception when invoking HAL_Timer_Create() */
/* SDK调用的系统适配接口 HAL_Timer_Create() 返回异常, 未能成功创建一个定时器 */
#define STATE_SYS_DEPEND_TIMER_CREATE               (STATE_SYS_DEPEND_BASE - 0x0005)
/* SDK run into exception when invoking HAL_Timer_Start() */
/* SDK调用的系统适配接口 HAL_Timer_Start() 返回异常, 未能成功启动指定的定时器的计时 */
#define STATE_SYS_DEPEND_TIMER_START                (STATE_SYS_DEPEND_BASE - 0x0006)
/* SDK run into exception when invoking HAL_Timer_Stop() */
/* SDK调用的系统适配接口 HAL_Timer_Stop() 返回异常, 未能成功停止指定的定时器的计时 */
#define STATE_SYS_DEPEND_TIMER_STOP                 (STATE_SYS_DEPEND_BASE - 0x0007)
/* SDK run into exception when invoking HAL_Timer_Delete() */
/* SDK调用的系统适配接口 HAL_Timer_Delete() 返回异常, 未能销毁指定的定时器 */
#define STATE_SYS_DEPEND_TIMER_DELETE               (STATE_SYS_DEPEND_BASE - 0x0008)
/* SDK run into exception when invoking HAL_MutexCreate() */
/* SDK调用的系统适配接口 HAL_MutexCreate() 返回异常, 未能创建一个互斥锁 */
#define STATE_SYS_DEPEND_MUTEX_CREATE               (STATE_SYS_DEPEND_BASE - 0x0009)
/* SDK run into exception when invoking HAL_MutexLock() */
/* SDK调用的系统适配接口 HAL_MutexLock() 返回异常, 未能成功申请互斥锁 */
#define STATE_SYS_DEPEND_MUTEX_LOCK                 (STATE_SYS_DEPEND_BASE - 0x000A)
/* SDK run into exception when invoking HAL_MutexUnlock() */
/* SDK调用的系统适配接口 HAL_MutexUnlock() 返回异常, 未能成功申释放互斥锁 */
#define STATE_SYS_DEPEND_MUTEX_UNLOCK               (STATE_SYS_DEPEND_BASE - 0x000B)
/* SDK run into exception when TX or RX through lower network layer */
/* SDK调用的系统适配接口 HAL_TCP_Read|Write() 或 HAL_SSL_Read|Write() 返回异常, 底层连接已关闭而未能成功进行网络收发 */
#define STATE_SYS_DEPEND_NWK_CLOSE                  (STATE_SYS_DEPEND_BASE - 0x000C)
/* SDK run into timeout when TX or RX through lower network layer */
/* SDK调用的系统适配接口 HAL_TCP_Read() 或 HAL_SSL_Read() 未能成功在预期的时间内得到网络回应 */
#define STATE_SYS_DEPEND_NWK_TIMEOUT                (STATE_SYS_DEPEND_BASE - 0x000D)
/* SDK run into invalid handler when lookup network lower layer connection */
/* SDK进行底层网络收发通信时, 看到系统对TCP连接或TLS连接的标识句柄是非法的 */
#define STATE_SYS_DEPEND_NWK_INVALID_HANDLE         (STATE_SYS_DEPEND_BASE - 0x000E)
/* SDK run into exception when RX through lower network layer */
/* SDK调用的系统适配接口 HAL_TCP_Read() 或 HAL_SSL_Read() 返回异常, 未能成功读取一段内容 */
#define STATE_SYS_DEPEND_NWK_READ_ERROR             (STATE_SYS_DEPEND_BASE - 0x000F)

/* System: 0x0200 ~ 0x02FF */

/* MQTT: 0x0300 ~ 0x03FF */
#define STATE_MQTT_BASE                             (-0x0300)

/* Deserialized CONNACK from MQTT server says protocol version is unacceptable */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示不能接受请求中的MQTT协议版本 */
#define STATE_MQTT_CONNACK_VERSION_UNACCEPT         (STATE_MQTT_BASE - 0x0001)
/* Deserialized CONNACK from MQTT server says identifier is rejected */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示不能接受请求中的设备标识符 */
#define STATE_MQTT_CONNACK_IDENT_REJECT             (STATE_MQTT_BASE - 0x0002)
/* Deserialized CONNACK from MQTT server says service is not available */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示当前MQTT服务不可用 */
#define STATE_MQTT_CONNACK_SERVICE_NA               (STATE_MQTT_BASE - 0x0003)
/* Deserialized CONNACK from MQTT server says it failed to authorize */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示当前对请求设备未能完成鉴权 */
#define STATE_MQTT_CONNACK_NOT_AUTHORIZED           (STATE_MQTT_BASE - 0x0004)
/* Deserialized CONNACK from MQTT server says username/password is invalid */
/* 设备发出MQTT建连请求后, 服务器应答的报文表示对设备身份鉴权不通过 */
#define STATE_MQTT_CONNACK_BAD_USERDATA             (STATE_MQTT_BASE - 0x0005)
/* Skip current reconnect attemption until next timer expiration */
/* 跳过当前的重连动作, 因为上一次的重连过去的时间太短, 稍晚再进行连接 */
#define STATE_MQTT_WAIT_RECONN_TIMER                (STATE_MQTT_BASE - 0x0006)
/* Reserved buffer is too short when generate device signature for hostname */
/* 计算设备签名时, 为域名预留的字符串缓冲区太短, 不足容纳输出内容 */
#define STATE_MQTT_SIGN_HOSTNAME_BUF_SHORT          (STATE_MQTT_BASE - 0x0007)
/* Reserved buffer is too short when generate device signature for username */
/* 计算设备签名时, 为MQTT的用户名预留的字符串缓冲区太短, 不足容纳输出内容 */
#define STATE_MQTT_SIGN_USERNAME_BUF_SHORT          (STATE_MQTT_BASE - 0x0008)
/* Reserved buffer is too short when generate device signature for clientId */
/* 计算设备签名时, 为MQTT的clientId预留的字符串缓冲区太短, 不足容纳输出内容 */
#define STATE_MQTT_SIGN_CLIENTID_BUF_SHORT          (STATE_MQTT_BASE - 0x0009)
/* Reserved buffer is too short for signature generate source */
/* 计算设备签名时, 为拼接签名来源内容预留的字符串缓冲区太短, 不足容纳输出内容 */
#define STATE_MQTT_SIGN_SOURCE_BUF_SHORT            (STATE_MQTT_BASE - 0x000A)
/* SDK run into exception when invoking lower layer wrapper_mqtt_init() */
/* SDK调用的MQTT建连底层接口 wrapper_mqtt_init() 异常, 初始化失败 */
#define STATE_MQTT_WRAPPER_INIT_FAIL                (STATE_MQTT_BASE - 0x000B)
/* Failed to serialize connect request */
/* 序列化MQTT建连请求报文异常 */
#define STATE_MQTT_SERIALIZE_CONN_ERROR             (STATE_MQTT_BASE - 0x000C)
/* Failed to serialize acknowledge message of publish */
/* 序列化对服务器下推报文的应答消息异常 */
#define STATE_MQTT_SERIALIZE_PUBACK_ERROR           (STATE_MQTT_BASE - 0x000D)
/* Failed to serialize ping request */
/* 序列化MQTT心跳请求报文异常 */
#define STATE_MQTT_SERIALIZE_PINGREQ_ERROR          (STATE_MQTT_BASE - 0x000E)
/* Failed to serialize subscribe request */
/* 序列化MQTT订阅请求报文异常 */
#define STATE_MQTT_SERIALIZE_SUB_ERROR              (STATE_MQTT_BASE - 0x000F)
/* Failed to serialize unsubscribe request */
/* 序列化MQTT取消订阅请求报文异常 */
#define STATE_MQTT_SERIALIZE_UNSUB_ERROR            (STATE_MQTT_BASE - 0x0010)
/* Failed to serialize publish message */
/* 序列化MQTT上报消息报文异常 */
#define STATE_MQTT_SERIALIZE_PUB_ERROR              (STATE_MQTT_BASE - 0x0011)
/* Failed to deserialize connect response */
/* 反序列化MQTT服务器返回的建连应答报文异常 */
#define STATE_MQTT_DESERIALIZE_CONNACK_ERROR        (STATE_MQTT_BASE - 0x0012)
/* Failed to deserialize subscribe response */
/* 反序列化MQTT服务器返回的订阅应答报文异常 */
#define STATE_MQTT_DESERIALIZE_SUBACK_ERROR         (STATE_MQTT_BASE - 0x0013)
/* Failed to deserialize publish response */
/* 反序列化MQTT服务器下推的消息报文异常 */
#define STATE_MQTT_DESERIALIZE_PUB_ERROR            (STATE_MQTT_BASE - 0x0014)
/* Failed to deserialize unsubscribe response */
/* 反序列化MQTT服务器返回的取消订阅应答报文异常 */
#define STATE_MQTT_DESERIALIZE_UNSUBACK_ERROR       (STATE_MQTT_BASE - 0x0015)
/* Failed to read MQTT packet from network */
/* 未能成功从网络上读取服务器下发的MQTT报文内容 */
#define STATE_MQTT_PACKET_READ_ERROR                (STATE_MQTT_BASE - 0x0016)
/* Failed to interpret CONNACK from MQTT server */
/* 未能成功解析从MQTT服务器返回的建连请求应答 */
#define STATE_MQTT_CONNACK_UNKNOWN_ERROR            (STATE_MQTT_BASE - 0x0017)
/* Reserved buffer is too short when retrieve content from network */
/* 从网络协议栈读取MQTT下推报文时, 预留的缓冲区太短, 不足容纳待收取内容 */
#define STATE_MQTT_RX_BUFFER_TOO_SHORT              (STATE_MQTT_BASE - 0x0018)
/* Reserved buffer is too short when compose content going to network */
/* 组装MQTT上报消息准备发给网络协议栈时, 为报文内容预留的缓冲区太短, 不足容纳待发送内容 */
#define STATE_MQTT_TX_BUFFER_TOO_SHORT              (STATE_MQTT_BASE - 0x0019)
/* Reserved buffer is too short when compose topic for MQTT outgoing message */
/* 组装MQTT上报消息准备发给网络协议栈时, 为Topic字段预留的缓冲区太短, 不足容纳待发送内容 */
#define STATE_MQTT_TOPIC_BUF_TOO_SHORT              (STATE_MQTT_BASE - 0x001A)
/* Retried time exceeds maximum when perform IOT_MQTT_Construct() */
/* 单独的一次 IOT_MQTT_Construct() 接口工作中, 已多次重试仍未连上服务端, 停止尝试返回 */
#define STATE_MQTT_CONN_RETRY_EXCEED_MAX            (STATE_MQTT_BASE - 0x001B)
/* Re-publish QoS1 message exceeds maximum */
/* 对QoS1类型的MQTT上报消息已多次重试, 仍未得到服务端收到的回应, 放弃重新发送 */
#define STATE_MQTT_QOS1_REPUB_EXCEED_MAX            (STATE_MQTT_BASE - 0x001C)
/* Invalid QoS input when publish MQTT message */
/* 对MQTT消息上报时, 遇到不合理的QoS参数取值 */
#define STATE_MQTT_PUB_QOS_INVALID                  (STATE_MQTT_BASE - 0x001D)
/* Skip current action because MQTT connection is break */
/* 因为当前MQTT连接已断开, 跳过当前可能的发送或接收动作 */
#define STATE_MQTT_IN_OFFLINE_STATUS                (STATE_MQTT_BASE - 0x001E)
/* Receive MQTT packet from network can not be interpret */
/* 从网络上收取到的MQTT报文, 无法解析其报文类型 */
#define STATE_MQTT_RECV_UNKNOWN_PACKET              (STATE_MQTT_BASE - 0x001F)
/* MQTT connection instance exceed allowed maximum */
/* 当前已创建的MQTT连接数量超出限制, 已无法再新建连接 */
#define STATE_MQTT_CLI_EXCEED_MAX                   (STATE_MQTT_BASE - 0x0020)
/* MQTT subscribe instance exceed allowed maximum */
/* 当前已订阅的MQTT主题数量超出限制, 已无法再新建订阅 */
#define STATE_MQTT_SUB_EXCEED_MAX                   (STATE_MQTT_BASE - 0x0021)
/* Invalid MQTT topic can not be interpret */
/* 遇到不合理的MQTT主题格式, 无法进行订阅或发布等动作 */
#define STATE_MQTT_UNEXP_TOPIC_FORMAT               (STATE_MQTT_BASE - 0x0022)
/* Wait too long time after syncronized subscribe request sent */
/* 进行同步的订阅请求时, 已超出限定时长而未得到服务端回应 */
#define STATE_MQTT_SYNC_SUB_TIMEOUT                 (STATE_MQTT_BASE - 0x0023)
/* Last connect request has not been responsed in async protocol stack */
/* 在异步协议栈上工作时, 上次的连接请求尚未得到服务端回应, 跳过当前连接请求 */
#define STATE_MQTT_ASYNC_STACK_CONN_IN_PROG         (STATE_MQTT_BASE - 0x0024)
/* Currrent SDK configuration does not support working with async protocol stack */
/* 当前SDK未被配置为支持在异步协议栈上工作 */
#define STATE_MQTT_ASYNC_STACK_NOT_SUPPORT          (STATE_MQTT_BASE - 0x0025)
/* Got unknown event when work with async protocol stack */
/* SDK在异步协议栈上工作时, 接收到来自协议栈的事件, 但不能理解 */
#define STATE_MQTT_ASYNC_STACK_UNKNOWN_EVENT        (STATE_MQTT_BASE - 0x0026)

/* MQTT: 0x0300 ~ 0x03FF */

/* WiFi Provision: 0x0400 ~ 0x04FF */
#define STATE_WIFI_BASE                             (-0x0400)

/* WiFi Provision: 0x0400 ~ 0x04FF */

/* COAP: 0x0500 ~ 0x05FF */
#define STATE_COAP_BASE                             (-0x0500)

/* COAP: 0x0500 ~ 0x05FF */

/* HTTP: 0x0600 ~ 0x06FF */
#define STATE_HTTP_BASE                             (-0x0600)
/* Got HTTP response from server, says dynamic register request failed */
/* 使用动态注册/一型一密功能时, 从服务端返回的报文表示, 申请密钥失败 */
#define STATE_HTTP_DYNREG_FAIL_RESP                 (STATE_HTTP_BASE - 0x0001)
/* Got HTTP response from server, which contains invalid deviceSecret */
/* 使用动态注册/一型一密功能时, 从服务端返回的报文中, 解析得到的密钥不合理 */
#define STATE_HTTP_DYNREG_INVALID_DS                (STATE_HTTP_BASE - 0x0002)
/* Reserved buffer is too short when compose pre-auth HTTP request */
/* 组装预认证消息准备发给服务端时, 为HTTP上行报文预留的缓冲区太短, 不足容纳待发送内容 */
#define STATE_HTTP_PREAUTH_REQ_BUF_SHORT            (STATE_HTTP_BASE - 0x0003)
/* Reserved buffer is too short when retrieve pre-auth HTTP response */
/* 收取预认证请求的应答报文时, 为HTTP回应报文预留的缓冲区太短, 不足容纳待接收内容 */
#define STATE_HTTP_PREAUTH_INVALID_RESP             (STATE_HTTP_BASE - 0x0004)
/* Content length field does not exist in HTTP response from server */
/* 从HTTP服务器返回的报文中, 没有找到 Content-Length 字段 */
#define STATE_HTTP_RESP_MISSING_CONTENT_LEN         (STATE_HTTP_BASE - 0x0005)

/* HTTP: 0x0600 ~ 0x06FF */

/* OTA: 0x0700 ~ 0x07FF */
#define STATE_OTA_BASE                              (-0x0700)

/* OTA: 0x0700 ~ 0x07FF */

/* Bind: 0x0800 ~ 0x08FF */
#define STATE_BIND_BASE                             (-0x0800)
/* Got external token configuration */
/* 绑定功能模块接收到来自外部的Token传入 */
#define STATE_BIND_SET_APP_TOKEN                    (STATE_BIND_BASE - 0x0001)
/* Skip current reset report since last reset already success */
/* 跳过当前的复位请求发送, 因为上次的复位请求已被服务端接受, 并处理成功 */
#define STATE_BIND_ALREADY_RESET                    (STATE_BIND_BASE - 0x0002)
/* Sending bind token to cloud */
/* 正在将绑定token发送给服务端 */
#define STATE_BIND_REPORT_TOKEN                     (STATE_BIND_BASE - 0x0003)
/* Sent bind token to cloud does not get response before timeout */
/* 绑定token发送给服务端后, 已超过限定的时长, 仍未得到回应 */
#define STATE_BIND_REPORT_TOKEN_TIMEOUT             (STATE_BIND_BASE - 0x0004)
/* Sent bind token to cloud gets success response */
/* 绑定token发送给服务端后, 已得到服务端成功接收的回应 */
#define STATE_BIND_REPORT_TOKEN_SUCCESS             (STATE_BIND_BASE - 0x0005)
/* Failed to initialize CoAP server in LAN */
/* 绑定功能模块未能成功开启CoAP服务, 该服务用于局域网内的token传递 */
#define STATE_BIND_COAP_INIT_FAIL                   (STATE_BIND_BASE - 0x0006)
/* Sending bind token in LAN */
/* 正在主动将绑定token在局域网内广播 */
#define STATE_BIND_NOTIFY_TOKEN_SENT                (STATE_BIND_BASE - 0x0007)
/* Got token query request from external devices in same LAN */
/* 正在接收到同一局域网内来自其它电子设备的token查询请求 */
#define STATE_BIND_RECV_TOKEN_QUERY                 (STATE_BIND_BASE - 0x0008)
/* Responding bind token in LAN */
/* 正在响应局域网内的token查询请求, 将token发送给其它电子设备 */
#define STATE_BIND_SENT_TOKEN_RESP                  (STATE_BIND_BASE - 0x0009)
/* Skip current bind action since offline reset in progress */
/* 暂不进行当前的token上云, 因为尚有一次未完成的离线reset正在进行中 */
#define STATE_BIND_RST_IN_PROGRESS                  (STATE_BIND_BASE - 0x000A)
/* Got invalid MQTT respond from server after token report */
/* 将绑定token上报给服务端后, 接收到的回应报文不合法 */
#define STATE_BIND_MQTT_RSP_INVALID                 (STATE_BIND_BASE - 0x000B)
/* Got invalid CoAP request from external devices in same LAN */
/* 接收到来自同一局域网内其它电子设备的token查询请求, 但请求报文不合法 */
#define STATE_BIND_COAP_REQ_INVALID                 (STATE_BIND_BASE - 0x000C)

/* Bind: 0x0800 ~ 0x08FF */

/* Device Model: 0x0900 ~ 0x09FF */
#define STATE_DEV_MODEL_BASE                        (-0x0900)

/* Master device already open so skip current open action */
/* 主设备已经打开, 不必再次初始化 */
#define STATE_DEV_MODEL_MASTER_ALREADY_OPEN         (STATE_DEV_MODEL_BASE - 0x0001)
/* Master device already connect so skip current connect action */
/* 主设备已经连接服务端, 不必再次连接 */
#define STATE_DEV_MODEL_MASTER_ALREADY_CONNECT      (STATE_DEV_MODEL_BASE - 0x0002)
/* Master device not open yet so skip current action */
/* 当前动作需要主设备open成功, 但这尚未完成 */
#define STATE_DEV_MODEL_MASTER_NOT_OPEN_YET         (STATE_DEV_MODEL_BASE - 0x0003)
/* Master device not connect yet so skip current action */
/* 当前动作需要主设备连接服务端成功, 但这尚未完成 */
#define STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET      (STATE_DEV_MODEL_BASE - 0x0004)
/* Requested device already created */
/* 当前请求创建的设备已经存在, 不必再次创建 */
#define STATE_DEV_MODEL_DEVICE_ALREADY_EXIST        (STATE_DEV_MODEL_BASE - 0x0005)
/* Requested device not found in list */
/* 当前请求对应的设备未找到, 无法对其操作 */
#define STATE_DEV_MODEL_DEVICE_NOT_FOUND            (STATE_DEV_MODEL_BASE - 0x0006)
/* Internal error happens in device model function */
/* 物模型/子设备管理模块发生内部错误 */
#define STATE_DEV_MODEL_INTERNAL_ERROR              (STATE_DEV_MODEL_BASE - 0x0007)
/* Internal error about file descriptor happens in device model function */
/* 物模型/子设备管理模块发生fd有关的内部错误 */
#define STATE_DEV_MODEL_INTERNAL_FD_ERROR           (STATE_DEV_MODEL_BASE - 0x0008)
/* Internal event about MQTT connect happens in device model function */
/* 物模型/子设备管理模块发现MQTT连接已经建立, 跳过再次建连动作 */
#define STATE_DEV_MODEL_INTERNAL_MQTT_DUP_INIT      (STATE_DEV_MODEL_BASE - 0x0009)
/* Internal error about MQTT unconnect happens in device model function */
/* 物模型/子设备管理模块发生MQTT连接未建立的内部错误 */
#define STATE_DEV_MODEL_INTERNAL_MQTT_NOT_INIT_YET  (STATE_DEV_MODEL_BASE - 0x000A)
/* Failed to open handler for cloud abstract layer */
/* 物模型模块中发生连接抽象层初始化失败 */
#define STATE_DEV_MODEL_CM_OPEN_FAILED              (STATE_DEV_MODEL_BASE - 0x000B)
/* Failed to find file descriptor in cloud abstract layer */
/* 物模型模块中未找到连接抽象层的fd */
#define STATE_DEV_MODEL_CM_FD_NOT_FOUND             (STATE_DEV_MODEL_BASE - 0x000C)
/* Failed to connect MQTT in device model function */
/* 物模型模块中发生MQTT连接建立失败 */
#define STATE_DEV_MODEL_MQTT_CONNECT_FAILED         (STATE_DEV_MODEL_BASE - 0x000D)
/* Got unexpected MQTT message from server in device model */
/* 物模型模块中接收到来自服务端的MQTT下推消息, 但报文内容不符合预期 */
#define STATE_DEV_MODEL_RECV_UNEXP_MQTT_PUB         (STATE_DEV_MODEL_BASE - 0x000E)
/* Got MQTT message from server but its JSON format is wrong */
/* 物模型模块中接收到来自服务端的MQTT下推消息, 但报文内容不是合理的JSON格式 */
#define STATE_DEV_MODEL_WRONG_JSON_FORMAT           (STATE_DEV_MODEL_BASE - 0x000F)
/* Service respond does not have correct request context in device model */
/* 物模型模块中发送服务回应报文给服务端时, 没有找到对应的服务端请求上下文 */
#define STATE_DEV_MODEL_SERVICE_CTX_NOT_EXIST       (STATE_DEV_MODEL_BASE - 0x0010)
/* OTA service is not enabled in device model */
/* 物模型模块中发现OTA功能未开启 */
#define STATE_DEV_MODEL_OTA_NOT_ENABLED             (STATE_DEV_MODEL_BASE - 0x0011)
/* OTA service is not initialized correctly in device model */
/* 物模型模块中发现OTA功能未能正确初始化 */
#define STATE_DEV_MODEL_OTA_NOT_INITED              (STATE_DEV_MODEL_BASE - 0x0012)
/* OTA service is initialized but failed in device model */
/* 物模型模块中发现对OTA功能初始化失败 */
#define STATE_DEV_MODEL_OTA_INIT_FAILED             (STATE_DEV_MODEL_BASE - 0x0013)
/* OTA for some sub-device is not finished yet so skip device switching */
/* 子设备OTA时, 由于当前仍有其它子设备OTA尚未完成, 而跳过当前操作 */
#define STATE_DEV_MODEL_OTA_STILL_IN_PROGRESS       (STATE_DEV_MODEL_BASE - 0x0014)
/* OTA firmware downloaded failed to pass integrity check */
/* 物模型模块中发现OTA下载的固件, 未能通过完整性校验 */
#define STATE_DEV_MODEL_OTA_IMAGE_CHECK_FAILED      (STATE_DEV_MODEL_BASE - 0x0015)
/* ALCS function failed to initialize itself */
/* 本地控制功能初始化失败 */
#define STATE_DEV_MODEL_ALCS_CONSTRUCT_FAILED       (STATE_DEV_MODEL_BASE - 0x0016)
/* Gateway/Sub-device management function is not configured on */
/* SDK当前未被配置为打开子设备管理/网关功能 */
#define STATE_DEV_MODEL_GATEWAY_NOT_ENABLED         (STATE_DEV_MODEL_BASE - 0x0017)

/* Device Model: 0x0900 ~ 0x09FF */

#ifdef __cplusplus
}
#endif
#endif  /* __INFRA_STATE_H__ */

