华东2站点
=========

# 版本V2.01

- 发布日期: 2017/10/10
- 开发语言: C语言
- 开发环境: 64位Linux, GNU Make
- 下载链接: [RELEASED_V2_01_20171010.zip](https://github.com/aliyun/iotkit-embedded/archive/RELEASED_V2_01_20171010.zip)
- 更新内容:
    - **新增CoAP+OTA:** 允许配置成基于CoAP通知方式的OTA
    - **新增HTTP+TLS:** 在MQTT/CoAP之外, 新增HTTP的通道
    - **细化OTA状态:** 优化OTA部分代码, 使云端可以更细化的区分设备的OTA固件下载状态
    - **ArmCC支持:** 修正了SDK在ArmCC编译器编译时会出现的报错

# 版本V2.0

- 发布日期: 2017/08/21
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

# JAVA版本

| 支持协议 | 更新历史 | 下载链接 |
|:-------|:-------|:-------|
| MQTT | **2017-05-27：**<br/>支持华东2节点的设备认证流程，同时添加java端设备影子demo|[iotx-sdk-mqtt-java](http://aliyun-iot.oss-cn-hangzhou.aliyuncs.com/iotx-sdk-java/iotx-sdk-mqtt-java-20170526.zip "iotx-sdk-mqtt-java") MQTT的JAVA版只是使用开源库实现的一个demo，仅用于参考|

