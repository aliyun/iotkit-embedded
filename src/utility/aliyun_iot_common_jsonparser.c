

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "aliyun_iot_platform_memory.h"
#include "aliyun_iot_common_jsonparser.h"


#define JSON_PARSE 0
#if (JSON_PARSE==1)
    #define json_debug log_debug
#else
    #define json_debug ;
#endif

typedef struct JSON_NV{
    int nLen;
    int vLen;
    int vType;
    const char* pN;
    const char* pV;
} JSON_NV;

const char *json_get_object(const char *pjson, size_t json_len, int type)
{
    const char *pos = 0;
    const char *str = pjson;
    char ch = (type == JOBJECT) ? '{' : '[';
    while ((NULL != str) && (*str != '\0') && ((str - pjson) < json_len)) {
        if (*str == ' ') {
            str++;
            continue;
        }
        pos = (*str == ch) ? str : 0;
        break;
    }
    return pos;
}

const char *json_get_next_object(const char *pjson, size_t json_len, int type, const char *str, const char **key,
        int *key_len, const char **val, int *val_len, int *val_type)
{
    char JsonMark[JTYPEMAX][2] = { { '\"', '\"' }, { '{', '}' }, { '[', ']' }, {
            '0', ' ' } };
    int iMarkDepth = 0, iValueType = JNONE, iNameLen = 0, iValueLen = 0;
    const char *p_cName = 0, *p_cValue = 0, *p_cPos = str;

    if (type == JOBJECT) {
        /****Catch Name****/
        p_cPos = strchr(p_cPos, '"');
        if (!p_cPos)
            return 0;
        p_cName = ++p_cPos;
        p_cPos = strchr(p_cPos, '"');
        if (!p_cPos)
            return 0;
        iNameLen = p_cPos - p_cName;

        /****Catch Value****/
        p_cPos = strchr(p_cPos, ':');
    }
    while ((p_cPos) && (*p_cPos) && ((p_cPos - pjson) < json_len)) {
        if (*p_cPos == '"') {
            iValueType = JSTRING;
            p_cValue = ++p_cPos;
            break;
        } else if (*p_cPos == '{') {
            iValueType = JOBJECT;
            p_cValue = p_cPos++;
            break;
        } else if (*p_cPos == '[') {
            iValueType = JARRAY;
            p_cValue = p_cPos++;
            break;
        } else if (*p_cPos >= '0' && *p_cPos <= '9') {
            iValueType = JNUMBER;
            p_cValue = p_cPos++;
            break;
        } else if (*p_cPos == 't' || *p_cPos == 'T' || *p_cPos == 'f' || *p_cPos == 'F') {
            iValueType = JBOOLEAN;
            p_cValue = p_cPos;
            break;
        }
        p_cPos++;
    }
    while ((p_cPos) && (*p_cPos) && ((p_cPos - pjson) < json_len) && (iValueType > JNONE)) {
        if (iValueType == JBOOLEAN) {
            int len = strlen(p_cValue);
            if ((*p_cValue == 't' || *p_cValue == 'T') && len >= 4
                    && (!strncmp(p_cValue, "true", 4)
                            || !strncmp(p_cValue, "TRUE", 4))) {
                iValueLen = 4;
                p_cPos = p_cValue + iValueLen;
                break;
            } else if ((*p_cValue == 'f' || *p_cValue == 'F') && len >= 5
                    && (!strncmp(p_cValue, "false", 5)
                            || !strncmp(p_cValue, "FALSE", 5))) {
                iValueLen = 5;
                p_cPos = p_cValue + iValueLen;
                break;
            }
        } else if (iValueType == JNUMBER) {
            if (*p_cPos < '0' || *p_cPos > '9') {
                iValueLen = p_cPos - p_cValue;
                break;
            }
        } else if (*p_cPos == JsonMark[iValueType][1]) {
            if (iMarkDepth == 0) {
                iValueLen = p_cPos - p_cValue + (iValueType == JSTRING ? 0 : 1);
                p_cPos++;
                break;
            } else {
                iMarkDepth--;
            }
        } else if (*p_cPos == JsonMark[iValueType][0]) {
            iMarkDepth++;
        }
        p_cPos++;
    }

    if (type == JOBJECT) {
        *key = p_cName;
        *key_len = iNameLen;
    }

    *val = p_cValue;
    *val_len = iValueLen;
    *val_type = iValueType;
    if (iValueType == JSTRING)
        return p_cValue + iValueLen + 1;
    else
        return p_cValue + iValueLen;
}


int json_parse_name_value(const char *p_cJsonStr, int iStrLen, json_parse_cb pfnCB, void *p_CBData)
{
    const char *pos=0, *key=0, *val=0;
    int klen=0, vlen=0, vtype=0;
    //char last_char=0;
    int ret = JSON_RESULT_ERR;

    if (p_cJsonStr==NULL || iStrLen==0 || pfnCB==NULL)
        return ret;

    //last_char = *(p_cJsonStr+iStrLen);
    //*(p_cJsonStr + iStrLen) = 0;

    json_object_for_each_kv(p_cJsonStr, iStrLen, pos, key, klen, val, vlen, vtype) {
        if (key && klen && val && vlen) {
            ret = JSON_RESULT_OK;
            if (JSON_PARSE_FINISH == pfnCB(key, klen, val, vlen, vtype, p_CBData)) {	//catch the ball
                break;
            }
        }
    }

    //*(p_cJsonStr + iStrLen) = last_char;
    return ret;
}

int json_get_value_by_name_cb(const char *p_cName, int iNameLen, const char *p_cValue, int iValueLen, int iValueType, void *p_CBData)
{
#if(JSON_DEBUG == 1)
    int i;
    if (p_cName) {
        json_debug("\nName:\n  ");
        for (i = 0; i < iNameLen; i++)
            json_debug("%c", *(p_cName + i));
    }

    if (p_cValue) {
        json_debug("\nValue:\n  ");
        for (i = 0; i < iValueLen; i++)
            json_debug("%c", *(p_cValue + i));
        json_debug("\n");
    }
#endif

    JSON_NV *p_stNameValue = (JSON_NV *) p_CBData;
    if (!strncmp(p_cName, p_stNameValue->pN, p_stNameValue->nLen)) {
        p_stNameValue->pV = p_cValue;
        p_stNameValue->vLen = iValueLen;
        p_stNameValue->vType = iValueType;
        return JSON_PARSE_FINISH;
    } else {
        return JSON_PARSE_OK;
    }

}

/*TODO:name不存在和name存在且value值为空串，都返回NULL*/
const char *json_get_value_by_name(const char *p_cJsonStr, int iStrLen, const char *p_cName, int *p_iValueLen, int *p_iValueType)
{
    JSON_NV stNV = { 0, 0, 0, 0 };
    stNV.pN = p_cName;
    stNV.nLen = strlen(p_cName);
    if (JSON_RESULT_OK == json_parse_name_value(p_cJsonStr, iStrLen, json_get_value_by_name_cb, (void *)&stNV)) {
        if (p_iValueLen) {
            *p_iValueLen = stNV.vLen;
        }

        if (p_iValueType) {
            *p_iValueType = stNV.vType;
        }
    }
    return stNV.pV;
}


int json_get_array_size(const char *json_str, int str_len)
{
    const char *pos, *entry;
    int len, type, size = 0;
    json_array_for_each_entry(json_str, str_len, pos, entry, len, type){
        size++;
    }

    return size;
}


const char *json_get_value_by_fullname(const char *p_cJsonStr, int iStrLen, const char *p_cName, int *p_iValueLen, int *p_iValueType)
{
    const char *value = NULL, *delim = NULL;
    int value_len = -1, value_type = -1, key_len = -1;

    const char *key_iter, *src_iter;
    char *key_next;

    size_t key_total_len = strlen(p_cName) + 1;

    iStrLen = iStrLen;

    src_iter = p_cJsonStr;
    key_iter = p_cName;
    do {
        if ((delim = strchr(key_iter, '.')) != NULL) {
            key_len = delim - key_iter;
            key_next = aliyun_iot_memory_malloc(key_total_len);
            strncpy(key_next, key_iter, key_len);
            key_next[key_len] = '\0';
            value = json_get_value_by_name(src_iter, strlen(src_iter), key_next, &value_len, 0);
            if (value == NULL) {
                aliyun_iot_memory_free(key_next);
                return NULL;
            }
            src_iter = value;
            key_iter = delim + 1;
        }
    } while (delim);

    value = json_get_value_by_name(src_iter, strlen(src_iter), key_iter, &value_len, &value_type);
    if (NULL == value) {
        return NULL;
    }

    if (NULL != p_iValueLen) {
        *p_iValueLen = value_len;
    }

    if (NULL != p_iValueType) {
        *p_iValueType = value_type;
    }

    return value;
}
