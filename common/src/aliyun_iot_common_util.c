#include <stdlib.h>
#include <string.h>
#include "aliyun_iot_common_util.h"
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_error.h"

#define TOPIC_NAME_MAX_LEN 64

int aliyun_iot_common_check_rule(char *iterm,ALIYUN_IOT_TOPIC_TYPE_E type)
{
    if(NULL == iterm)
    {
        WRITE_IOT_ERROR_LOG("iterm is NULL");
        return FAIL_RETURN;
    }

    int i = 0;
    int len = strlen(iterm);
    for(i = 0;i<len;i++)
    {
        if(TOPIC_FILTER_TYPE == type)
        {
            if('+' == iterm[i] || '#' == iterm[i])
            {
                if(1 != len)
                {
                    WRITE_IOT_ERROR_LOG("the character # and + is error");
                    return FAIL_RETURN;
                }
            }
        }
        else
        {
            if('+' == iterm[i] || '#' == iterm[i])
            {
                WRITE_IOT_ERROR_LOG("has character # and + is error");
                return FAIL_RETURN;
            }
        }

        if(iterm[i] < 32 || iterm[i] >= 127 )
        {
            return FAIL_RETURN;
        }
    }
    return SUCCESS_RETURN;
}

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
int aliyun_iot_common_check_topic(const char * topicName, ALIYUN_IOT_TOPIC_TYPE_E type)
{
    if(NULL == topicName || '/' != topicName[0])
    {
        return FAIL_RETURN;
    }

	if(strlen(topicName) > TOPIC_NAME_MAX_LEN)
	{
		WRITE_IOT_ERROR_LOG("len of topicName exceeds 64");
		return FAIL_RETURN;
	}
	
    int mask = 0;
    char topicString[TOPIC_NAME_MAX_LEN];
    memset(topicString,0x0,TOPIC_NAME_MAX_LEN);
    strncpy(topicString,topicName,TOPIC_NAME_MAX_LEN);

    char* delim = "/";
    char* iterm = NULL;
    iterm = strtok(topicString,delim);

    if(SUCCESS_RETURN != aliyun_iot_common_check_rule(iterm,type))
    {
        WRITE_IOT_ERROR_LOG("run aliyun_iot_common_check_rule error");
        return FAIL_RETURN;
    }

    for(;;)
    {
        iterm = strtok(NULL,delim);

        if(iterm == NULL)
        {
            break;
        }

        //当路径中包含#字符，且不是最后一个路径名时报错
        if(1 == mask)
        {
            WRITE_IOT_ERROR_LOG("the character # is error");
            return FAIL_RETURN;
        }

        if(SUCCESS_RETURN != aliyun_iot_common_check_rule(iterm,type))
        {
            WRITE_IOT_ERROR_LOG("run aliyun_iot_common_check_rule error");
            return FAIL_RETURN;
        }

        if(iterm[0] == '#')
        {
            mask = 1;
        }
    }

    return SUCCESS_RETURN;
}
