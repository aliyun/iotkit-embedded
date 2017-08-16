#阿里云MQTT协议说明

##MQTT协议版本

 目前阿里云支持MQTT标准协议接入(兼容3.1.1和3.1版本协议).
 具体的协议请参考 [MQTT 3.1.1](http://mqtt.org) 或 [MQTT 3.1](http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html) 协议文档.

##MQTT通道
 支持 TLSV1.1，TLSV1.2 版本的协议来建立安全连接
 
##TOPIC规范
默认情况下创建一个产品后，这产品下的所有设备都拥有以下topic权限：
   * /${productKey}/${deviceName}/update       **pub**
   * /${productKey}/${deviceName}/update/error **pub**
   * /${productKey}/${deviceName}/get          **sub**
   * /sys/${productKey}/${deviceName}/thing/gateway/+/  **pub&sub**
   
```
/${productKey}/${deviceName}/ TOPIC类您可以在控制台自己扩展和定义
其中sys开头的模版属于系统约定的应用协议通信标准，作为系统预留的topic，是不允许用户自定义的，并且对于约定的topic要符合阿里云ALink数据标准，比如上面第四条为网关使用topic类用于网关场景.
```
   
   我们把这类每个topic规则称为topic类，topic类已经做了设备维度隔离，您无需担心topic被跨设备越权问题。
   
```
 pub代表数据上报到这个topic的权限，sub代表订阅这个topic的权限.
```
