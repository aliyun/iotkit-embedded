### 1、使用SDK接入
我们将在8中下旬推出C版本CoAP SDK，敬请期待。

### 2、使用CoAP协议自主接入
使用CoAP自主接入流程：

* CoAP服务器地址 ${productKey}.iot-as-coap.cn-shanghai.aliyuncs.com:5684, 其中productKey请替换为您申请的产品Key。
* 目前只支持DTLS，必须走安全通道，[根证书](http://aliyun-iot.oss-cn-hangzhou.aliyuncs.com/cert_pub/root.crt?spm=5176.doc30539.2.1.1MRvV5&file=root.crt "根证书")。
* 设备在发送数据前，首先发起认证，获取设备的token。
* 每次上报数据时，都需要携带token信息，如果token失效需要重新认证获取token，token可以缓存本地，有效期48小时。

####2.1 设备认证(Auth)
    此接口用于传输数据前获取token，只需要请求一次：
        POST /auth
        Host: ${productKey}.iot-as-coap.cn-shanghai.aliyuncs.com
        Port: 5684
        Accept: application/json or application/cbor
        Content-Format: application/json or application/cbor
        payload: {"productKey":"ZG1EvTEa7NN","deviceName":"NlwaSPXsCpTQuh8FxBGH","clientId":"mylight1000002","sign":"bccb3d2618afe74b3eab12b94042f87b"}
    
### 参数说明：
    * Method: POST，只支持POST方法
    * URL: /auth，url地址
    * Accept：接收的数据编码方式，目前支持 application/json、 application/cbor
    * Content-Format： 上行数据的编码格式，目前支持 application/json、 application/cbor
    
##### payload内容：
JSON数据格式，具体属性值如下：

| 字段名称    | 是否必选 | 说明|
| :-----     | :-------------- | :-------------------------- | 
|productKey  | 必选 | productKey,从iot套件控制台获取 
| deviceName  | 必选 | deviceName,从iot套件控制台获取 
|sign         | 必选 | 签名,hmacmd5(deviceSecret,content), content = 将所有提交给服务器的参数（version,sign,resources,signmethod除外）, 按照字母顺序排序, 然后将参数值依次拼接，无拼接符号
|signmethod |    可选 | 算法类型，hmacmd5或hmacsha1
|clientId   |    必选 | 客户端自表示Id,64字符内
|timestamp  |    可选 | 时间戳，目前时间戳并不做窗口校验，只起到加盐的作用

    
    
##### 返回：
        response：{"token":"eyJ0b2tlbiI6IjBkNGUyNjkyZTNjZDQxOGU5MTA4Njg4ZDdhNWI3MjUxIiwiZXhwIjoxNDk4OTg1MTk1fQ.DeQLSwVX8iBjdazjzNHG5zcRECWcL49UoQfq1lXrJvI"}
    
##### 返回码（CoAP返回码）说明：
    
| Code  |   描述      |  Payload|备注|
| :-----| :---------: | :----------: | ----------:|
| 2.05  |  Content    | 认证通过：Token对象 |   正确请求|
| 4.00  |  Bad Request|  返回错误信息  | 请求发送的Payload非法|
| 4.04  |  Not Found  |  404 Not found  | 请求的路径不存在 |
| 4.05  |  Method Not Allowed |  支持的方法  | 请求方法不是指定值|
| 4.06  |  Not Acceptable  |  要求的Accept  | Accept不是指定的类型 |
| 4.15  |  Unsupported Content-Format | 支持的content信息 | 请求的content不是指定类型|
| 5.00  | Internal Server Error | 返回错误信息 | auth服务器超时或错误|



####2.2 上行数据
发送数据到某个topic，topic可以在控制台产品管理－>消息通信进行设置，比如对于topic  `/productkey/device/pub`, 您可以调用 ${productKey}.iot-as-coap.cn-shanghai.aliyuncs.com:5684/topic/productkey/device/pub 这个地址来上报数据，目前只支持pub权限的topic用于数据上报，示例：

        POST /topic/${topic}
        Host: ${productKey}.iot-as-coap.cn-shanghai.aliyuncs.com
        Port: 5683
        Accept: application/json or application/cbor
        Content-Format: application/json or application/cbor
        payload: ${your_data}
        CustomOptions: number:61(标识token)
### 参数说明
    * Method: POST，支持POST方法
    * URL: /topic/${your_topic}，your_topic替换为你自己的topic
    * Accept：接收的数据编码方式，目前只支持 application/json、 application/cbor
    * Content-Format： 上行数据的编码格式，服务端不做校验
    * CustomOptions： 设备认证（Auth）获取到token值，Option Number 使用61
    
##### payload内容：
    发往${your_topic}的内容，由业务方自定义，套件不会解析，只进行透传
    
##### 返回值：
        response: {"messageId":"880712545291464704","resp":null}
    resp为本次请求的服务端返回，客户端需要解析时根据不同场景的结果进行解析。
##### 返回码（CoAP返回码）说明：

| Code  |   描述       |  Payload| 备注|
| :-----| :---------: | :----------: | ----------:|
| 2.05  |  Content    |  发送结果 |   正确请求|
| 4.00  |  Bad Request|  请求错误说明  |  |
| 4.01  |  Unauthorized |  返回错误信息  | token非法或者过期，需求重新认证获取token |
| 4.02  |  Bad Option |  返回错误信息 | token不存在或者长度非法|
| 4.04  |  Not Found  |  返回错误信息  | 请求的路径不存在 |
| 4.05  |  Method Not Allowed |  返回错误信息  | 请求的方法错误，非post|
| 4.06  |  Not Acceptable  |  要求的Accept  | Accept不是指定的类型 |
| 4.14  |  Request-url Too Long  |  返回错误信息  | topic 设置过长，超过128 |
| 4.29  |  Too Many Requests  |  返回错误信息  | |
| 5.00  | Internal Server Error | 返回错误信息 | 一般是认证或者流转服务超时|

#### 限制条件及注意事项
    * TOPIC规范和MQTT的TOPIC一致，CoAP协议内 coap://host:port/topic/${your_topic},这个接口对于所有your_topic和MQTT topic是可以复用的，不支持`?query_String=xxx`形式传参。
    * 客户端缓存认证返回的token是请求的令牌，通过DTLS传输。
    * 传输的数据大小依赖于MTU的大小，最好在1K以内
