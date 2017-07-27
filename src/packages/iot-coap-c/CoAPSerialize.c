#include <stdio.h>
#include "CoAPExport.h"

int CoAPSerialize_Header(CoAPMessage *msg, unsigned char *buf, int buflen)
{
    if(4 > buflen){
        return 0;
    }
    buf[0] = (((msg->header.version & 0x3) << 6) | ((msg->header.type & 0x3) << 4))
                | (msg->header.tokenlen & 0x0F);

    buf[1] = msg->header.code;
    buf[2] = (msg->header.msgid & 0xFF00) >> 8;
    buf[3] = (msg->header.msgid & 0x00FF);

    return 4;
}

int CoAPSerialize_Token(CoAPMessage *msg, unsigned char * buf, int buflen)
{
    int i = 0;

    if(buflen < msg->header.tokenlen){
        return 0;
    }
    for (i = 0; i < msg->header.tokenlen; i++){
        buf[i] = msg->token[i];
    }
    return msg->header.tokenlen;
}

static int CoAPSerialize_Option(CoAPMsgOption *option, unsigned char *buf)
{
    int  len  = 0;
    unsigned char *ptr   = buf;
    unsigned short count = 0;

    if(269 <= option->num){
        *ptr = ((14 & 0x0F) << 4);
    }
    else if(13 <= option->num){
        *ptr = ((13 & 0x0F) << 4);
    }
    else{
        *ptr = option->num << 4;
    }

    if (269 <= option->len){
        *ptr |=  (14 & 0x0F);
    }
    else if(13 <= option->len){
        *ptr |= (13 & 0x0F);
    }
    else{
        *ptr |= (option->len & 0x0F);
    }
    ptr ++;

    if (269 <= option->num){
        *ptr     = (unsigned char)(((option->num - 269) & 0xFF00) >> 8);
        *(ptr+1) = (unsigned char)(((option->num - 269) & 0x00FF));
        ptr += 2;
    }
    else if(13 <= option->num){
        *ptr    = (unsigned char)(option->num - 13);
        ptr ++;
    }


    if (269 <= option->len){
        *ptr     = (unsigned char)(((option->len - 269) & 0xFF00) >> 8);
        *(ptr+1) = (unsigned char)(((option->len - 269) & 0x00FF));
        ptr += 2;
    }
    else if(13 <= option->len){
        *ptr = (unsigned char)(option->len - 13);
        ptr ++;
    }


    memcpy(ptr, option->val, option->len);
    ptr += option->len;

    return (int)(ptr - buf);
}

int CoAPSerialize_Options(CoAPMessage *msg,  unsigned char * buf, int buflen)
{
    int i      = 0;
    int count  = 0;

    for (i = 0; i < msg->optnum; i++)
    {
        unsigned short len = 0;
        len = CoAPSerialize_Option(&msg->options[i], &buf[count]);
        if (0 < len){
            count += len;
        }
        else{
            return 0;
        }
    }

    return count;
}

int CoAPSerialize_Payload(CoAPMessage *msg, unsigned char *buf, int buflen)
{
    if(msg->payloadlen + 1 > buflen){
        return -1;
    }
    if(msg->payloadlen > 0 && NULL != msg->payload)
    {
        *buf = 0xFF;
        buf ++;
        memcpy(buf, msg->payload, msg->payloadlen);
        return msg->payloadlen + 1;
    }
    else{
        return 0;
    }
}


int CoAPSerialize_MessageLength(CoAPMessage *msg, unsigned char *buf, unsigned short buflen)
{

}

int CoAPSerialize_Message(CoAPMessage *msg, unsigned char *buf, unsigned short buflen)
{
    unsigned char *ptr   = buf;
    unsigned short count = 0;
    unsigned short remlen  = buflen;

    if(NULL == buf || NULL == msg){
        return COAP_ERROR_INVALID_PARAM;
    }

    count = CoAPSerialize_Header(msg, ptr, remlen);
    ptr += count;
    remlen -= count;

    count = CoAPSerialize_Token(msg, ptr, remlen);
    ptr += count;
    remlen -= count;


    count = CoAPSerialize_Options(msg, ptr, remlen);
    ptr += count;
    remlen -= count;

    count = CoAPSerialize_Payload(msg, ptr, remlen);
    ptr += count;
    remlen -= count;

    return (buflen-remlen);
}
