# 阿里云物联网套件

物联网套件是阿里云专门为物联网领域的开发人员推出的, 其目的是帮助开发者搭建安全且性能强大的数据通道, 方便终端(如传感器, 执行器, 嵌入式设备或智能家电等等)和云端的双向通信.

-   **[官方文档首页](https://help.aliyun.com/product/30520.html)**
-   **[更多产品介绍](https://help.aliyun.com/document_detail/30522.html)**

# 特别声明

**本工程SDK代码并不是直接开箱可用于各种嵌入式平台的整套实现, 仅是嵌入式平台版本的二次开发基础和参考实现**.

- 这套代码目前是在`Ubuntu16.04` + `64位Linux` + `GNU Make`的主机环境下开发与测试, 编译产生的也是运行于64位Ubuntu的二进制静态库和可执行程序.
- 需要嵌入式设备连接阿里云时, 可以下载和修改本工程代码, 作为二次开发的基础, **必须自行修改和适配实际硬件**
- 关于如何把本工程代码改造到嵌入式平台上, 具体可阅读文档:

    **[doc/website.documents/Porting_Guider.md](https://help.aliyun.com/document_detail/56047.html)**

# 快速开始

为了最快速的运行参考实现, 实际体验物联网套件如何连接阿里云, 可参看文档

**[doc/website.documents/Quick_Start.md](https://help.aliyun.com/document_detail/30530.html)**

其中包含:

- 如何向阿里云注册产品品类和设备
- 如何填写到SDK中
- 如何编译和运行, 测试/观察建立连接和收发报文的过程

# 使用参考

**请阅读文档: [doc/website.documents/SDK_Usage_Reference/C-SDK_Usage.md](https://help.aliyun.com/document_detail/42655.html)**
