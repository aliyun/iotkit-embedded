#include <stdio.h>
#include <string.h>
#include "CCPIoTSDK.h"

#define HOST_NAME "iot-auth.aliyun.com"

//用户需要根据设备信息完善以下宏定义中的四元组内容
#define PRODUCT_KEY    ""
#define PRODUCT_SECRET ""
#define DEVICE_NAME    ""
#define DEVICE_SECRET  ""

//以下三个TOPIC的宏定义不需要用户修改，可以直接使用
//IOT HUB为设备建立三个TOPIC：update用于设备发布消息，error用于设备发布错误，get用于订阅消息
#define TOPIC_UPDATE         "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR          "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET            "/"PRODUCT_KEY"/"DEVICE_NAME"/get"

#define COMMAND_TIMEOUT_MS 3000

#define KEEP_ALIVE_INTERVAL 65

CLIENT_S g_client;

#define CCP_WRITE_BUF_LEN 1024

#define CCP_READ_BUF_LEN 1024

int onMessage(CCP_MESSAGE_S *msg)
{
    switch (msg->msgType)
    {
        case PUBLISH:
            {
                printf("recv publish message!\n");
                CCP_PUBLISH_S *req = (CCP_PUBLISH_S *) msg->payload;
                if (req->payloadLen > 0)
                {
                    printf("publish:SequenceId is %u, aliveSecond is %d, topic is %s, payload is %s, payloadLen is %d!\n",
                           req->SequenceId, req->aliveSecond, req->topic, req->payload, req->payloadLen);
                }
                else
                {
                    printf("publish:SequenceId is %u, aliveSecond is %d, topic is %s, payloadLen is %d!\n",
                           req->SequenceId, req->aliveSecond, req->topic, req->payloadLen);
                }
                break;
            }
        case PUBLISH_ACK:
            {
                printf("recv publich ack message!\n");
                CCP_PUBLISH_ACK_S *resp = (CCP_PUBLISH_ACK_S *) msg->payload;
                printf("publish ack:SequenceId is %u, code is %d!\n", resp->SequenceId, resp->code);
                break;
            }
        case SUBSCRIBE_ACK:
            {
                printf("recv subscribe ack message!\n");
                CCP_SUBSCRIBE_ACK_S *resp = (CCP_SUBSCRIBE_ACK_S *) msg->payload;
                printf("subscribe ack:SequenceId is %u, codesLen is %d, messagesLen is %d!\n",
                       resp->SequenceId, resp->codesLen, resp->messagesLen);
                int i;
                for (i = 0; i < resp->codesLen; i++)
                {
                    printf("subscribe ack:codes[%d] is %d!\n", i, resp->codes[i]);
                }
                for (i = 0; i < resp->messagesLen; i++)
                {
                    printf("subscribe ack:messages[%d] is %s!\n", i, resp->messages[i]);
                }
                break;
            }
        case UNSUBSCRIBE_ACK:
            {
                printf("recv unsubscribe ack message!\n");
                CCP_UNSUBSCRIBE_ACK_S *resp = (CCP_UNSUBSCRIBE_ACK_S *) msg->payload;
                printf("unsubscribe ack:SequenceId is %u, codesLen is %d!\n", resp->SequenceId, resp->codesLen);
                int i;
                for (i = 0; i < resp->codesLen; i++)
                {
                    printf("unsubscribe ack:codes[%d] is %d!\n", i, resp->codes[i]);
                }
                break;
            }
        case RPCRESPONSE:
            {
                printf("recv rpc response message!\n");
                CCP_RPC_RESP_S *resp = (CCP_RPC_RESP_S *) msg->payload;
                if (RPC_RESP_STATUS_SUCCESS == resp->StatusCode)
                {
                    if (resp->payloadLen > 0)
                    {
                        printf("rpc response success:SequenceId is %u, StatusCode is %d, payload is %s, payloadLen is %d!\n",
                               resp->SequenceId, resp->StatusCode, resp->payload, resp->payloadLen);
                    }
                    else
                    {
                        printf("rpc response success:SequenceId is %u, StatusCode is %d, payloadLen is %d!\n",
                               resp->SequenceId, resp->StatusCode, resp->payloadLen);
                    }

                }
                else
                {
                    printf("rpc response failed:SequenceId is %u, StatusCode is %d!\n", resp->SequenceId, resp->StatusCode);
                }
                break;
            }
        case REVERSE_RPCREQUEST:
            {
                printf("recv rrpc request message!\n");
                CCP_RRPC_REQ_S *req = (CCP_RRPC_REQ_S *) msg->payload;
                CCP_RRPC_RESP_S resp;
                resp.SequenceId = req->SequenceId;
                resp.statusCode = 0;
                resp.payload = req->payload;
                resp.payloadLen = req->payloadLen;
                if (0 != CCPSendRRPCResponseMsg(&g_client, &resp))
                {
                    printf("send rrpc response message failed!\n");
                }

                if (req->payloadLen > 0)
                {
                    printf("rrpc request:SequenceId is %u, payload is %s, payloadLen is %d!\n",
                           req->SequenceId, req->payload, req->payloadLen);
                }
                else
                {
                    printf("rrpc request:SequenceId is %u, payloadLen is %d!\n", req->SequenceId, req->payloadLen);
                }

                if (resp.payloadLen > 0)
                {
                    printf("rrpc response:SequenceId is %u, statusCode is %d, payload is %s, payloadLen is %d!\n",
                           resp.SequenceId, resp.statusCode, resp.payload, resp.payloadLen);
                }
                else
                {
                    printf("rrpc response:SequenceId is %u, statusCode is %d, payloadLen is %d!\n",
                           resp.SequenceId, resp.statusCode, resp.payloadLen);
                }
                break;
            }
        default:
            printf("message type is invalid!\n");
            break;
    }

    return 0;
}

void onConnect()
{
    printf("onConnect!\n");
}

void onDisconnect(CLIENT_STATUS_E status)
{
    printf("onDisconnect, status is %d!\n", status);
}

void onTimeout(CCP_MESSAGE_S *msg)
{
    switch (msg->msgType)
    {
        case RPCREQUEST:
            {
                printf("rpc request wait for response timeout!\n");
                CCP_RPC_REQ_S *req = (CCP_RPC_REQ_S *) msg->payload;
                if (req->payloadLen > 0)
                {
                    printf("rpc request timeout:SequenceId is %u, payload is %s, payloadLen is %d!\n",
                           req->SequenceId, req->payload, req->payloadLen);
                }
                else
                {
                    printf("rpc request timeout:SequenceId is %u, payloadLen is %d!\n", req->SequenceId, req->payloadLen);
                }

                break;
            }
        case SUBSCRIBE:
            {

                printf("subscribe wait for ack timeout!\n");
                CCP_SUBSCRIBE_S *req = (CCP_SUBSCRIBE_S *) msg->payload;
                printf("subscribe timeout:SequenceId is %u, topicsSize is %d!\n", req->SequenceId, req->topicsSize);
                int i;
                for (i = 0; i < req->topicsSize; i++)
                {
                    printf("subscribe timeout:topics[%d] is %s!\n", i, req->topics[i]);
                }
                break;
            }
        case UNSUBSCRIBE:
            {
                printf("unsubscribe wait for ack timeout!\n");
                CCP_UNSUBSCRIBE_S *req = (CCP_UNSUBSCRIBE_S *) msg->payload;
                printf("unsubscribe timeout:SequenceId is %u, topicsSize is %d!\n", req->SequenceId, req->topicsSize);
                int i;
                for (i = 0; i < req->topicsSize; i++)
                {
                    printf("unsubscribe timeout:topics[%d] is %s!\n", i, req->topics[i]);
                }
                break;
            }
        default:
            printf("message type is invalid!\n");
            break;
    }
}

int ccp_client_demo()
{
	unsigned char *writeBuf = (unsigned char*)malloc(CCP_WRITE_BUF_LEN);
	if(!writeBuf)
	{
		printf("malloc ccp write buf failed\n");
		return -1;
	}
	
	unsigned char *readBuf = (unsigned char*)malloc(CCP_READ_BUF_LEN);
	if(!readBuf)
	{
		printf("malloc ccp read buf failed\n");
		free(writeBuf);
		return -1;
	}

    unsigned int sequenceId;
    AUTH_CONFIG_S authConfig;
	CCP_CONNECT_S connect;
    CCP_RPC_REQ_S rpcReq;
    CCP_SUBSCRIBE_S subscribe;
    CCP_PUBLISH_S publish;
    CCP_UNSUBSCRIBE_S unsubscribe;

    memset(&g_client, 0, sizeof(CLIENT_S));
    memset(&authConfig, 0, sizeof(AUTH_CONFIG_S));

    strncpy(authConfig.productKey, PRODUCT_KEY, strlen(PRODUCT_KEY));
    strncpy(authConfig.productSecret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
    strncpy(authConfig.deviceName, DEVICE_NAME, strlen(DEVICE_NAME));
    strncpy(authConfig.deviceSecret, DEVICE_SECRET, strlen(DEVICE_SECRET));
    strncpy(authConfig.hostName, HOST_NAME, strlen(HOST_NAME));
    authConfig.signMethod = SIGN_HMAC_MD5;

    if (0 != CCPInit(&g_client, &authConfig, COMMAND_TIMEOUT_MS, writeBuf, CCP_WRITE_BUF_LEN, readBuf,
                     CCP_READ_BUF_LEN, onMessage, onConnect, onDisconnect, onTimeout))
    {
        printf("ccp init failed!\n");
		free(writeBuf);
		free(readBuf);
        return -1;
    }

	if (0 != CCPAuth(&g_client))
	{
		printf("ccp auth failed!\n");
		free(writeBuf);
		free(readBuf);
		return -1;
	}

    memset(&connect, 0, sizeof(connect));
	connect.limit = 50;
	connect.keepalive = KEEP_ALIVE_INTERVAL;
	connect.network = 0;

	if (0 != CCPSendConnectMsg(&g_client, &connect))
	{
		printf("ccp connect failed!\n");
		free(writeBuf);
		free(readBuf);
		return -1;
	}

    memset(&rpcReq, 0, sizeof(rpcReq));
    rpcReq.payload = (unsigned char *) "hello world";
    rpcReq.payloadLen = strlen("hello world");

    if (0 != CCPSendRPCRequestMsg(&g_client, &rpcReq))
    {
        printf("ccp send rpc request message failed!\n");
    }

    sequenceId = rpcReq.SequenceId;
    printf("rpc request sequenceId is %u!\n", sequenceId);

    memset(&subscribe, 0, sizeof(subscribe));
    subscribe.topicsSize = 1;
    strncpy(subscribe.topics[0], TOPIC_GET, sizeof(subscribe.topics[0]) - 1);

    if (0 != CCPSendSubscribeMsg(&g_client, &subscribe))
    {
        printf("ccp send subscribe message failed!\n");
    }

    sequenceId = subscribe.SequenceId;
    printf("subscribe sequenceId is %u!\n", sequenceId);

    memset(&publish, 0, sizeof(publish));
    publish.aliveSecond = 5;
    strncpy(publish.topic, TOPIC_UPDATE, sizeof(publish.topic));
    publish.payload = (unsigned char *)"{\"message\":\"Hello World\"}";
    publish.payloadLen = strlen("{\"message\":\"Hello World\"}");

    if (0 != CCPSendPublishMsg(&g_client, &publish))
    {
        printf("ccp send publish message failed!\n");
    }

    sequenceId = publish.SequenceId;
    printf("publish sequenceId is %u!\n", sequenceId);

    while (getchar() != 'q')

    memset(&unsubscribe, 0, sizeof(unsubscribe));
    unsubscribe.topicsSize = 1;
    strncpy(unsubscribe.topics[0], TOPIC_GET, sizeof(unsubscribe.topics[0]) - 1);

    if (0 != CCPSendUnsubscribeMsg(&g_client, &unsubscribe))
    {
        printf("ccp send unsubscribe message failed!\n");
    }

    sequenceId = unsubscribe.SequenceId;
    printf("unsubscribe sequenceId is %u!\n", sequenceId);

    if (0 != CCPRelease(&g_client))
    {
        printf("ccp release failed!\n");
    }

	free(writeBuf);
	free(readBuf);

    return 0;
}

