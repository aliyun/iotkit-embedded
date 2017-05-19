#ifndef ALIYUN_IOT_COMMON_UTIL_H
#define ALIYUN_IOT_COMMON_UTIL_H

typedef enum ALIYUN_IOT_TOPIC_TYPE
{
    TOPIC_NAME_TYPE = 0,
    TOPIC_FILTER_TYPE
}ALIYUN_IOT_TOPIC_TYPE_E;

/***********************************************************
* 函数名称: aliyun_iot_common_check_topic
* 描       述: topic校验
* 输入参数: const char * topicName
*          ALIYUN_IOT_TOPIC_TYPE_E type 校验类型
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: topicname校验时不允许有+，#符号
*           topicfilter校验时+，#允许存在但必须是单独的一个路径单元，
*           且#只能存在于最后一个路径单元
************************************************************/
int aliyun_iot_common_check_topic(const char * topicName,ALIYUN_IOT_TOPIC_TYPE_E type);

#endif
