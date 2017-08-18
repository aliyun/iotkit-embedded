我们提供2种模式
1. MQTT客户端域名直连（资源受限设备推荐） 
2. 先HTTPS发送授权后再连接MQTT（一些特殊增值服务，比如设备级别的引流）


#一 使用MQTT域名连接模式
您可以使用我们的DEMO进行快速移植，如果不用我们SDK，完全使用开源MQTT包自主接入(若使用第三方代码, 阿里云不提供技术支持)，可以参考以下流程：
1. 如果使用TLS，需要 [下载根证书](http://aliyun-iot.oss-cn-hangzhou.aliyuncs.com/cert_pub/root.crt "下载根证书")
2. 使用MQTT客户端连接服务器，如果您自主接入可以使用[开源MQTT客户端参考](https://github.com/mqtt/mqtt.github.io/wiki/libraries "开源MQTT客户端参考")，如果您对MQTT不了解，可以参考 http://mqtt.org 相关文档。(若使用第三方代码, 阿里云不提供技术支持)
3. MQTT 连接使用说明
```
连接域名: ${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com:1883， ${productKey}请替换为您的产品key，mqtt的Connect报文参数如下：
mqttClientId: clientId+"|securemode=3,signmethod=hmacsha1,timestamp=132323232|"
mqttUsername: deviceName+"&"+productKey
mqttPassword: sign_hmac(deviceSecret,content)sign签名需要把以下参数按字典序排序后，再根据signmethod加签。
其中clientId是客户端自表示id，建议mac或sn，64字符内；timestamp当前时间毫秒值，可选；mqttClientId格式中||内为扩展参数，signmethod代表签名算法类型，securemode代表目前安全模式，可选值有2 （TLS直连模式）、3（TCP直连模式）,参考以下示例：
```
4. 举例：
```
如果clientId = 12345，deviceName = device， productKey = pk， timestamp = 789，signmethod=hmacsha1，deviceSecret=secret，那么使用tcp方式提交给mqtt参数分别如下：
mqttclientId=12345|securemode=3,signmethod=hmacsha1,timestamp=789|
username=device&pk
password=hmacsha1("secret",clientId12345deviceNamedeviceproductKeypktimestamp789").toHexString(); //最后是二进制转16制字符串，大小写不敏感。 这个例子结果为 FAFD82A3D602B37FB0FA8B7892F24A477F851A14
注意上面3个参数分别是mqtt Connect登录报文的mqttClientId,mqttUsername,mqttPasswrod
```



#二 使用HTTPS认证再连接模式
##设备认证流程：
- 设备认证走https，认证域名为https://iot-auth.cn-shanghai.aliyuncs.com/auth/devicename
- 认证请求参数信息：
<table>
<tr>
<td width="20%">productKey</td>
<td width="20%">必选</td>
<td width="60%">productKey,从iot套件控制台获取</td>
</tr>
<tr>
<td width="20%">deviceName</td>
<td width="20%">必选</td>
<td width="60%">deviceName,从iot套件控制台获取</td>
</tr>
<tr>
<td width="20%">sign</td>
<td width="20%">必选</td>
<td width="60%">签名,hmacmd5(deviceSecret,content), content = 将所有提交给服务器的参数（version,sign,resources,signmethod除外）, 按照字母顺序排序, 然后将参数值依次拼接，无拼接符号</td>
</tr>

<tr>
<td width="20%">signmethod</td>
<td width="20%">可选</td>
<td width="60%">算法类型，hmacmd5或hmacsha1</td>
</tr>
<tr>
<td width="20%">clientId</td>
<td width="20%">必选</td>
<td width="60%">客户端自表示Id,64字符内</td>
</tr>
<tr>
<td width="20%">timestamp</td>
<td width="20%">可选</td>
<td width="60%">时间戳，目前时间戳并不做窗口校验，只起到加盐的作用</td>
</tr>
<tr>
<td width="20%">resources</td>
<td width="20%">可选</td>
<td width="60%">希望获取的资源描述，如mqtt。 多个资源名称用`逗号`隔开.</td>
</tr>
</table>
- 响应参数
<table>
<tr>
<td width="20%">iotId</td>
<td width="20%">必选</td>
<td width="60%">服务器颁发的一个连接标记，用于赋值给MQTT connect报文中的username</td>
</tr>
<tr>
<td width="20%">iotToken</td>
<td width="20%">必选</td>
<td width="60%">token有效期为7天，赋值给MQTT connect报文中的password</td>
</tr>
<tr>
<td width="20%">[resources]</td>
<td width="20%">可选</td>
<td width="60%">资源信息，扩展信息比如mqtt服务器地址、CA证书信息等</td>
</tr>
</table>

- x-www-form-urlencoded请求举例

```
POST /auth/devicename HTTP/1.1
Host: iot-auth.cn-shanghai.alibaba.com
Content-Type: application/x-www-form-urlencoded
Content-Length: 123

productKey=123&sign=123&timestamp=123&version=default&clientId=123&resouces=mqtt&deviceName=test
sign = hmac_md5(deviceSecret, clientId123deviceNametestproductKey123timestamp123)
```
- 请求响应

```
HTTP/1.1 200 OK
Server: Tengine
Date: Wed, 29 Mar 2017 13:08:36 GMT
Content-Type: application/json;charset=utf-8
Connection: close

{
    "code" : 200,
    "data" : {
        "iotId" : "42Ze0mk3556498a1AlTP",
        "iotToken" : "0d7fdeb9dc1f4344a2cc0d45edcb0bcb",
        "resources" : {
            "mqtt" : {
                "host" : "xxx.iot-as-mqtt.cn-shanghai.aliyuncs.com",
                "port" : 1883
            }
        }
    },
    "message" : "success"
}
```

##设备接入流程：

- 下载IoT根证书 [root.crt](http://docs-aliyun.cn-hangzhou.oss.aliyun-inc.com/assets/attach/30539/cn_zh/1495715052139/root.crt "root.crt"), 建议使用TLS1.2。
- 指定阿里云MQTT服务器地址 **${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com** 端口**1883**进行通信，其中productKey请替换为您的产品key。
- 采用TLS建立连接，客户端验证服务器通过CA证书完成，而服务器验证客户端通过MQTT协议体内connect报文中的username=iotId，password=iotToken，如果iotId或iotToken非法，mqtt connect失败，收到的connect ack为3。

**需要注意的事项**

>   在进行MQTT CONNECT协议设置的时候，

    Connect指令中的KeepAlive时间需要设置超过60秒以上，否则会拒绝连接。
    如果同一个设备多个连接可能会导致客户端互相上下线，MQTT默认开源SDK会自动重连，您可以通过日志服务看到设备行为。

**错误码**
```
401: request auth error，在这个场景里面通常在签名匹配不通过时返回
460: param error，参数异常
500: unknow error，一些未知异常
5001: meta device not found，指定的设备不存在
6200: auth type mismatch，未授权认证类型错误
```
