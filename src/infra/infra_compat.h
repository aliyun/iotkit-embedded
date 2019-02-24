#ifndef _INFRA_COMPAT_H_
#define _INFRA_COMPAT_H_

#include "infra_defs.h"
#include "infra_list.h"

#undef  being_deprecated
#define being_deprecated

typedef enum _IOT_LogLevel {
    IOT_LOG_NONE = 0,
    IOT_LOG_CRIT,
    IOT_LOG_ERROR,
    IOT_LOG_WARNING,
    IOT_LOG_INFO,
    IOT_LOG_DEBUG,
} IOT_LogLevel;

DLL_IOT_API void IOT_SetLogLevel(IOT_LogLevel level);
DLL_IOT_API void IOT_DumpMemoryStats(IOT_LogLevel level);

#ifndef offset_of
    #define offset_of aos_offsetof
#endif
#ifndef container_of
    #define container_of aos_container_of
#endif

#define LIST_HEAD                       AOS_DLIST_HEAD
#define LIST_HEAD_INIT                  AOS_DLIST_INIT
#define INIT_LIST_HEAD                  INIT_AOS_DLIST_HEAD
#define LIST_INIT                       AOS_DLIST_INIT

#define list_head                       dlist_s
#define list_head_t                     dlist_t

#define list_add                        dlist_add
#define list_add_tail                   dlist_add_tail
#define list_del                        dlist_del
#define list_empty                      dlist_empty
#define list_entry_number               dlist_entry_number
#define list_first_entry                dlist_first_entry
#define list_for_each                   dlist_for_each
#define list_for_each_entry_reverse     dlist_for_each_entry_reverse
#define list_for_each_safe              dlist_for_each_safe
#define list_init                       dlist_init

#define list_for_each_entry(pos, head, member, type)             \
    dlist_for_each_entry(head, pos, type, member)

#define list_for_each_entry_safe(pos, n, head, member, type)         \
    for (pos = list_entry((head)->next, type, member), \
         n = list_entry(pos->member.next, type, member);    \
         &pos->member != (head);                    \
         pos = n, n = list_entry(n->member.next, type, member))

#define list_next_entry(pos, member, type) \
    list_entry((pos)->member.next, type, member)

static inline void list_del_init(struct list_head *entry)
{
    list_del(entry);
    INIT_LIST_HEAD(entry);
}

static inline int list_is_last(const struct list_head *list,
                               const struct list_head *head)
{
    return list->next == head;
}

typedef struct {
    uint16_t        port;
    uint8_t         init;
    char            *host_name;
    char            *client_id;
    char            *username;
    char            *password;
    const char      *pub_key;
} iotx_conn_info_t, *iotx_conn_info_pt;

int IOT_SetupConnInfo(const char *product_key,
                      const char *device_name,
                      const char *device_secret,
                      void **info_ptr);

typedef struct {
    int domain_type;
    int dynamic_register;
    char *cloud_custom_domain;
    char *http_custom_domain;
} sdk_impl_ctx_t;

typedef enum {
    IOTX_IOCTL_SET_REGION,              /* value(int*): iotx_cloud_region_types_t */
    IOTX_IOCTL_GET_REGION,              /* value(int*) */
    IOTX_IOCTL_SET_MQTT_DOMAIN,         /* value(const char*): point to mqtt domain string */
    IOTX_IOCTL_SET_HTTP_DOMAIN,         /* value(const char*): point to http domain string */
    IOTX_IOCTL_SET_DYNAMIC_REGISTER,    /* value(int*): 0 - Disable Dynamic Register, 1 - Enable Dynamic Register */
    IOTX_IOCTL_GET_DYNAMIC_REGISTER,    /* value(int*) */
    IOTX_IOCTL_RECV_PROP_REPLY,         /* value(int*): 0 - Disable property post reply by cloud; 1 - Enable property post reply by cloud */
    IOTX_IOCTL_RECV_EVENT_REPLY,        /* value(int*): 0 - Disable event post reply by cloud; 1 - Enable event post reply by cloud */
    IOTX_IOCTL_SEND_PROP_SET_REPLY,     /* value(int*): 0 - Disable send post set reply by devid; 1 - Enable property set reply by devid */
    IOTX_IOCTL_SET_SUBDEV_SIGN,         /* value(const char*): only for slave device, set signature of subdevice */
    IOTX_IOCTL_GET_SUBDEV_LOGIN,        /* value(int*): 0 - SubDev is logout; 1 - SubDev is login */
    IOTX_IOCTL_SET_OTA_DEV_ID           /* value(int*):     select the device to do OTA according to devid */
} iotx_ioctl_option_t;

typedef enum {
    IMPL_LINKKIT_IOCTL_SWITCH_PROPERTY_POST_REPLY,           /* only for master device, choose whether you need receive property post reply message */
    IMPL_LINKKIT_IOCTL_SWITCH_EVENT_POST_REPLY,              /* only for master device, choose whether you need receive event post reply message */
    IMPL_LINKKIT_IOCTL_SWITCH_PROPERTY_SET_REPLY,            /* only for master device, choose whether you need send property set reply message */
    IMPL_LINKKIT_IOCTL_MAX
} impl_linkkit_ioctl_cmd_t;

/**
 * @brief Setup Demain type, should be called before MQTT connection.
 *
 * @param [in] option: see iotx_ioctl_option_t.
 *
 * @return None.
 * @see None.
 */
int IOT_Ioctl(int option, void *data);

#ifdef INFRA_MEM_STATS
#include "infra_mem_stats.h"
#endif

/* compatible for V2.3.0 */
#define IOTX_CLOUD_DOMAIN_SH        IOTX_CLOUD_REGION_SHANGHAI
#define IOTX_CLOUD_DOMAIN_SG        IOTX_CLOUD_REGION_SINGAPORE
#define IOTX_CLOUD_DOMAIN_JP        IOTX_CLOUD_REGION_JAPAN
#define IOTX_CLOUD_DOMAIN_US        IOTX_CLOUD_REGION_USA_WEST
#define IOTX_CLOUD_DOMAIN_GER       IOTX_CLOUD_REGION_GERMANY
#define IOTX_IOCTL_SET_DOMAIN       IOTX_IOCTL_SET_REGION
#define IOTX_IOCTL_GET_DOMAIN       IOTX_IOCTL_GET_REGION

#define IOT_OpenLog(arg)
#define IOT_CloseLog()              IOT_SetLogLevel(IOT_LOG_NONE)
#define IOT_LOG_EMERG               IOT_LOG_NONE

#define IOT_Linkkit_Post            IOT_Linkkit_Report
/* compatible for V2.3.0 end */

typedef enum {
    HAL_AES_ENCRYPTION = 0,
    HAL_AES_DECRYPTION = 1,
} AES_DIR_t;

typedef void *p_HAL_Aes128_t;

#define NETWORK_ADDR_LEN        (16)

typedef struct _network_addr_t {
    unsigned char
    addr[NETWORK_ADDR_LEN];
    unsigned short  port;
} NetworkAddr;

#endif  /* _INFRA_COMPAT_H_ */
