#ifndef CMP_MESSAGE_INFO_H
#define CMP_MESSAGE_INFO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "interface/message_info_abstract.h"
#include "interface/log_abstract.h"
#include "dm_import.h"

#define CMP_MESSAGE_INFO_CLASS get_cmp_message_info_class()
#define CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX 1200

#define CMP_MESSAGE_INFO_MESSAGE_TYPE_REQUEST  0
#define CMP_MESSAGE_INFO_MESSAGE_TYPE_RESPONSE 1
#define CMP_MESSAGE_INFO_MESSAGE_TYPE_RAW      2

typedef struct _req_rsp_param {
    char* key;
    char* value;
} req_rsp_param_t;

typedef struct {
    const void*     _;
    char*           uri;
    char*           payload_buf;
    char*           params_data_buf;
#ifdef MEMORY_NO_COPY
    int             params_data_buf_prefix_len;
#endif
    char*           raw_data_buf;
    int             raw_data_length;
    char*           product_key;
    char*           device_name;
    int             id;
    int             code;
    char*           version;
    void*           param_list;
    char*           method;
    int             message_type; /* 0: request; 1: response; 2: raw. */
    int             ret;
} cmp_message_info_t;

extern const void* get_cmp_message_info_class();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MESSAGE_INFO_H */
