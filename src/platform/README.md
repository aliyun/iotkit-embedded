SDK移植说明
================
IOT device SDK可以兼容多种操作系统，其中包括linux、windows、freertos、mico、ucos。
在platform目录下SDK已经实现了在以上操作系统下的移植，以下将介绍platform目录中各文件的功能以及移植的主要实现，如果用户使用非以上操作系统，则可以参照此文档的介绍自己完成移植工作。
#     **1.aliyun_iot_platform_memory文件**
此文件主要对不同操作系统中动态内存相关接口进行抽象，一般可以使用libc中的malloc等实现抽象接口，但出于效率有些操作系统不使用libc，那么可以使用自定义函数来实现。但要保证抽象接口的形式不变。
## **void* aliyun_iot_memory_malloc(INT32 size)**
动态开辟内存函数
## **void aliyun_iot_memory_free(void* ptr)** 
释放动态申请的内存    

# **2.aliyun_iot_platform_network文件**
此文件主要对不同操作系统中网络相关接口进行抽象，以上操作系统中抽象接口都是使用标准socket接口来实现，但是不同的协议栈功能上有差异实现时需要注意，例如lwip协议栈以、windows的TCPIP协议栈和linux的协议栈在阻塞和非阻塞接口操作上有差异。
## **INT32 aliyun_iot_network_create(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type)**
客户端网络资源创建、连接。此接口实现了DNS获取域名网络地址、端口，创建socket，连接服务端等操作
## **INT32 aliyun_iot_network_bind(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type)**
服务端网络资源创建、连接。此接口实现了DNS获取域名网络地址、端口，创建socket，bind，连接服务端等操作
## **INT32 aliyun_iot_network_send(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags)**
发送网络数据接口。此接口实现指定数据的网络发送
## **INT32 aliyun_iot_network_recv(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags)**
接收网络数据接口。此接口实现指定数据的网络接收
## **INT32 aliyun_iot_network_select(INT32 fd,IOT_NET_TRANS_TYPE_E type,int timeoutMs,IOT_NET_FD_ISSET_E* result)**
有可读可写事件判断接口。此接口实现传入的文件描述符当前是否可读写
## **INT32 aliyun_iot_network_settimeout(INT32 fd,int timeoutMs,IOT_NET_TRANS_TYPE_E type)**
设置socketopt的超时参数。通过setsocketopt接口设置收发数据的超时时间
## **INT32 aliyun_iot_network_get_nonblock(INT32 fd)**
获取socket非阻塞状态。不同操作系统获取此参数接口实现不同，如果有系统不支持非阻塞状态则可以不进行实现。
## **INT32 aliyun_iot_network_set_nonblock(INT32 fd)**
设置socket非阻塞状态。注意不同操作系统设置非阻塞的实现不同。
## **INT32 aliyun_iot_network_set_block(INT32 fd)**
设置socket阻塞状态。注意不同操作系统设置阻塞的实现不同。
## **INT32 aliyun_iot_network_close(INT32 fd)**
网络socket关闭。注意不同操作系统的接口差异，例如linux下使用close实现，windows下使用closesocket实现。
## **INT32 aliyun_iot_network_shutdown(INT32 fd,INT32 how)**
网络socket的shutdown。
## **INT32 aliyun_iot_get_errno(void)**
获取SDK定义的errno。不同系统的errno的值定义不同，SDK将通用的errno进行重定义，将不同系统的errno进行转换提供统一的错误码

# **3.aliyun_iot_platform_persistence文件**
当操作系统带有文件系统时，可以进行文件的读写等操作，此文件接口是抽象出操作系统中的文件基本操作，如果操作系统没有文件系统支持，则相关接口可以不进行实现。注意如果SDK在初始化时设置当前系统有文件系统支持那么一下接口必须实现。
## **IOT_RETURN_CODES_E aliyun_iot_file_open(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* filename,ALIYUN_IOT_FILE_FLAG_E flags)**
文件打开操作
## **INT32 aliyun_iot_file_close(ALIYUN_IOT_FILE_HANDLE_S*handle)**
文件关闭操作
## **INT32 aliyun_iot_file_write(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size,INT32 count)**
写文件操作
## **INT32 aliyun_iot_file_read(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size,INT32 count)**
读文件操作
## **INT32 aliyun_iot_file_fgets(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size)**
读取文件一行内容
## **INT32 aliyun_iot_file_fputs(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf)**
向文件写入一行内容
## **INT32 aliyun_iot_file_whether_exist(INT8* filename)**
判断文件是否存在

# **4.aliyun_iot_platform_pthread文件**
此文件实现对不同操作系统有关线程接口的抽象，并抽象了线程句柄，互斥锁句柄，不同操作系统线程ID和互斥锁类型都不相同，只需要使用不同的元素实现相关抽象的数据结构即可，此数据结构的各个元素不会暴露在SDK的源代码中，都是使用整个数据来进行操作。
## **INT32 aliyun_iot_mutex_init(ALIYUN_IOT_MUTEX_S*mutex)**
互斥初锁始化
## **INT32 aliyun_iot_mutex_lock(ALIYUN_IOT_MUTEX_S*mutex)**
互斥锁加锁
## **INT32 aliyun_iot_mutex_unlock(ALIYUN_IOT_MUTEX_S *mutex)**
互斥锁解锁
## **INT32 aliyun_iot_mutex_destory(ALIYUN_IOT_MUTEX_S *mutex)**
销毁互斥锁资源
## **INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle,void*(*func)(void*),void *arg,ALIYUN_IOT_PTHREAD_PARAM_S* param)**
创建线程
## **INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle)**
删除线程，注意如果操作系统不支持此接口可以不实现。
## **INT32 aliyun_iot_pthread_taskdelay( INT32 MsToDelay)**
睡眠延时

# **5.aliyun_iot_platform_random文件**
## **int aliyun_iot_gen_random(unsigned char *random, unsigned int len)**
主要实现产生随机数的接口，主要在mbedtls中有使用，注意不同系统实现随机数生成的方法不同，较为合理的方式使用设备文件生成随机数，如果不支持此方法可以使用伪随机数方式。

# **6.aliyun_iot_platform_signal文件**
## **INT32 aliyun_iot_send_SIGPIPE_signal(void)**
主要实现向系统发送管道破裂信号，主要用于mbedtls避免异常阻塞在socket中，如果系统不支持信号可以不实现。

# **7.aliyun_iot_platform_stdio文件**
此文件是为了兼容windows系统中的_snprintf 和其它系统snprintf的接口，所以将两者名称统一为aliyun_iot_stdio_snprintf

# **8.aliyun_iot_platform_timer文件**
此文件抽象了系统有关时间操作的部分接口，主要实现计时功能，并且抽象了时间数据结构代表到以某时间开始到现在的时长，在linux中此数据结构的元素由struct timeval来实现，其它操作系统也可以使用一个长整型代表从开机到当前时长。

## **void aliyun_iot_timer_assignment(INT32 millisecond,ALIYUN_IOT_TIME_TYPE_S *timer)**
时间变量赋值
## **INT32 aliyun_iot_timer_start_clock(ALIYUN_IOT_TIME_TYPE_S *timer)**
获得当前时间
## **INT32 aliyun_iot_timer_spend(ALIYUN_IOT_TIME_TYPE_S *start)**
距离起始时间已经度过的时长
## **INT32 aliyun_iot_timer_remain(ALIYUN_IOT_TIME_TYPE_S *end)**
距离终止时间还有多少时长
## **INT32 aliyun_iot_timer_expired(ALIYUN_IOT_TIME_TYPE_S *timer)**
定时时间是否已经超时
## **void aliyun_iot_timer_init(ALIYUN_IOT_TIME_TYPE_S* timer)**
初始化时间变量为初始值0
## **void aliyun_iot_timer_cutdown(ALIYUN_IOT_TIME_TYPE_S* timer,UINT32 millisecond)**
计算当前时间一定间隔后的时间值
## **UINT32 aliyun_iot_timer_now()**
获取当前时间值
## **INT32 aliyun_iot_timer_interval(ALIYUN_IOT_TIME_TYPE_S *start,ALIYUN_IOT_TIME_TYPE_S *end)**
计算两个时间的时间间隔


