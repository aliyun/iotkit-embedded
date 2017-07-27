
#include "lite-utils.h"
#ifndef __COAP_LIST_H__
#define __COAP_LIST_H__

typedef void (*CoAPRespMsgHandler)(void *data, void *message);

typedef struct
{
    void                    *user;
    unsigned short           msgid;
    char                     acked;
    unsigned char            tokenlen;
    unsigned char            token[8];
    unsigned char            retrans_count;
    unsigned short           timeout;
    unsigned short           timeout_val;
    unsigned char           *message;
    unsigned int             msglen;
    CoAPRespMsgHandler       handler;
    struct list_head         sendlist;
} CoAPSendNode;

typedef struct
{
    unsigned char            count;
    unsigned char            maxcount;
    struct list_head         sendlist;
}CoAPSendList;
#endif
