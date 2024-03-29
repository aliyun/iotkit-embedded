# 用户手册

[官网](https://help.aliyun.com/document_detail/96575.html)

# C-SDK 简介

设备厂商在设备上集成 `C-SDK` 后, 可以将设备安全的接入到阿里云IoT物联网平台, 从而使设备可以被阿里云IoT物联网平台进行管理

设备需要支持TCP/IP协议栈或串口通信, 以及C99标准的C库才能集成SDK, zigbee/433/KNX这样的非IP设备需要通过网关设备接入到阿里云IoT物联网平台, 网关设备需要集成C-SDK


# 快速开始

用户可以通过[快速的体验C-SDK](https://help.aliyun.com/document_detail/96624.html)感受如何将设备连接到阿里云物联网平台, 并如何将设备的数据发送到平台/以及如何从物联网平台接收数据


# 移植说明
C-SDK与OS/硬件平台无关, 全部部分用C编写, 它定义了HAL层来对接与硬件相关的功能, 因此在使用C-SDK时用户需要去实现相关的HAL函数

目前C-SDK已提供了在Linux/Windows上HAL的参考实现, 同时对于一些常见的OS或者模组也进行了适配, 可以[访问此处](https://help.aliyun.com/document_detail/97556.html)查看如何在相应平台上进行SDK的编译与集成
