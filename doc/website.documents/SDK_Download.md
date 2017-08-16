华东2站点
=========

# 版本V2.0

- 发布日期: 2017/XX/YY
- 开发语言: C语言
- 开发环境: 64位Linux, GNU Make
- 下载链接: [IoT-SDK_V2.0.tar.bz2](http://aliyun-iot.oss-cn-hangzhou.aliyuncs.com/iot-sdk-c/IoT-SDK_V2.0.tar.bz2)
- 更新内容:
    - **新增MQTT直连:** 支持更快更轻的连接IoT套件, 去掉对HTTPS/HTTP的依赖, 可看[公告](https://help.aliyun.com/document_detail/57164.html?spm=5176.doc53930.6.638.LNVRIp)
    - **新增CoAP通道:** 基于UDP, 在纯上报数据场景更节省资源, 可看[公告](https://help.aliyun.com/document_detail/57566.html?spm=5176.doc57164.6.637.tXyBoU)
    - **新增OTA通道:** 提供一系列OTA相关的API, 可查询/触发/下载用户自主上传的固件
    - **升级构建系统:** 支持更灵活的组织和配置SDK


# 版本V1.0.1

- 发布日期: 2017/06/29
- 开发语言: C语言
- 开发环境: 64位Linux, GNU Make
- 下载链接: [aliot_sdk_v1.0.1_release.7z](http://aliyun-iot.oss-cn-hangzhou.aliyuncs.com/iot-sdk-c/aliot_sdk_v1.0.1_release.7z)
- 更新内容:
    - **华东2站点:** 第一个正式配合华东2站点的设备端SDK, 全源码发布
    - **新增设备影子功能:** 具体可参看[设备影子介绍页面](https://help.aliyun.com/document_detail/53930.html)

#杭州站点 
##C版本
| 支持协议 | 更新历史 | 下载链接 |
|:-------|:-------|:-------|
| MQTT&CCP |  **2017-05-11：**<br/>1. 增加对接受数长度的判断，防止recv buffer接收溢出 <br/>2. 保活重连清除pingMark标志，保证重连后能正常触发保活机制 <br/>3. httpclient_recv() 增加对接受超时的判断，并且不将“超时”事件当做异常状态去处理<br/>4. SDK的版本号修改为1.0.1<br/>——————————————————<br/>**2017-01-24：**<br/>1.修改字符串拼接异常 <br/>2. 适配IOT HUB设备模板，demo中增加三个固定topic，修改鉴权域名 <br/>3. 增加串口通信接口<br/>4. 支持大端模式<br/>5. 修改sub时messageHandlers的匹配规则<br/>6. 修改ping保活机制，在没有pong消息时不再重复ping操作<br/>7. 增加linux下线程名称<br/>8. 修改linux下时间计算数据溢出导致recv阻塞问题<br/>9. 新增对openssl的支持<br/>——————————————————<br/>**2016-11-18：**<br/>1. 增加pub和sub中topic校验 <br/>2. mbedtls与mqtt分离，通过make.setting配置不编译tls <br/>3. 修改mqtt中有关auth的接口，改变了接口参数<br/>4. 增加云端设备删除后的异常处理<br/>5. 修改接收pub ack的轮询方式，修改为使用同步信号<br/>6. 修改mqtt下设备多次重连处理，增加重新获取服务器信息<br/>7. 增加tools/convert_line_end目录下换行符转换工具，提供unix到windows换行符的转换<br/>——————————————————<br/>**2016-10-14：**<br/>1. 新增mico 3.0、ucos-ii和winows系统适配，新增系统适配Readme <br/>2. 新增MQTT协议和mbedtls解耦层，支持选用自带SSL库 <br/>3. 新增输出日志文件，支持在有文件系统下使用<br/>4. 修改MQTT、CCP协议中网络重连时的关闭方式，避免重连异常事件发生，避免关闭socket时lwip1.4.x版本协议栈的异常发生<br/>——————————————————<br/>**2016-08-17：**<br/>1. 新增MQTT和CCP中有关QOS==1时的通信保障 <br/>2. 修改CCP内存释放以及timeout设置问题<br/>3. 日志打印级别错误的修改<br/>——————————————————<br/>**2016-07-28：**<br/>1. 修复MQTT断开连接异常的问题<br/>2. 鉴权参数增加SDK版本号<br/>3. 更新SDK的README文档<br/>——————————————————<br/>**2016-07-22：**<br/>1. 静态库大小压缩近40%<br/>2. CCP支持多线程消息发送<br/>3. 修复CCP保活时长设置无效的问题<br/>4. 修复CCP中payload支持二进制数据的问题<br/>5. 修复MQTT重连的问题<br/>——————————————————<br/>**2016-07-15：**<br/>beta版发布 | [获取最新版本](http://aliyun-iot.oss-cn-hangzhou.aliyuncs.com/aliyun-iot-device-sdk-c-2017-05-11.zip) |


#华东2节点

##java版本

| 支持协议 | 更新历史 | 下载链接 |
|:-------|:-------|:-------|
| MQTT | **2017-05-27：**<br/>支持华东2节点的设备认证流程，同时添加java端设备影子demo|[iotx-sdk-mqtt-java](http://aliyun-iot.oss-cn-hangzhou.aliyuncs.com/iotx-sdk-java/iotx-sdk-mqtt-java-20170526.zip "iotx-sdk-mqtt-java") |



#杭州节点
##JAVA版本
| 支持协议 | 更新历史 | 下载链接 |
|:-------|:-------|:-------|
| MQTT | **2017-02-10：**<br/>支持域名方式认证，去除http依赖<br/>———————————————————<br/>**2016-07-26：**<br/>鉴权参数中的设备ID替换为设备名称<br/>———————————————————<br/>**2016-06-13：**<br/>beta版发布 |[使用方法](~~42693~~) |
| CCP | **2016-07-12：**<br/>beta版发布<br/>**2016-09-18：**<br/>修复rrpc问题 | [获取最新版本](http://aliyun-iot.oss-cn-hangzhou.aliyuncs.com/java-sdk-ccp.zip)  注意 ccp的java版只是个简单实现,没有经过性能、平台适配等测试，只作为调试使用，不建议作为生产使用|
