#include <stdio.h>
#include <unistd.h>
#include "CoAPPlatform.h"
#include "CoAPList.h"

int CoAPList_Init(CoAPSendList *list, unsigned char maxcount)
{
    INIT_LIST_HEAD(&list->sendlist);
    list->count = 0;
    list->maxcount = maxcount;

    return 0;
}

int CoAPList_Destroy(CoAPSendList *list)
{
    CoAPSendNode *cur, *next;
    list_for_each_entry_safe(cur, next, &list->sendlist, sendlist){
        if(NULL != cur){
            if(NULL != cur->message){
                coap_free(cur->message);
                cur->message = NULL;
            }
            coap_free(cur);
            cur = NULL;
        }
    }
}


int CoAPListNode_Add(CoAPSendList *list, CoAPSendNode *node)
{
    if(NULL == list || NULL == node){
        return -1;
    }

    if(list->count >= list->maxcount){
        return -1;
    }
    else{
        list_add_tail(&node->sendlist, &list->sendlist);
        list->count ++;
        return 0;
    }
}

int CoAPListNode_Remove(CoAPSendList *list, CoAPSendNode *node)
{
    if(NULL == list || NULL == node){
        return -1;
    }
    list_del_init(&node->sendlist);
    list->count--;
    return 0;
}

CoAPSendNode *CoAPListNodeToken_Find(CoAPSendList *list, unsigned char *token, unsigned char len)
{
    CoAPSendNode *node = NULL;
    list_for_each_entry(node, &list->sendlist, sendlist){
        if(0 != node->tokenlen && node->tokenlen == len
                && 0 == memcmp(node->token, token, len)){
            return node;
        }
    }
    return NULL;
}

CoAPSendNode *CoAPListNodeMsgId_Find(CoAPSendList *list, unsigned short msgid)
{
    CoAPSendNode *node = NULL;
    list_for_each_entry(node, &list->sendlist, sendlist){
        if(node->msgid == msgid){
            return node;
        }
    }
    return NULL;
}

