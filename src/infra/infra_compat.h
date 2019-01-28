#ifndef _INFRA_COMPAT_H_
#define _INFRA_COMPAT_H_

#include "infra_list.h"

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
iotx_conn_info_t g_iotx_conn_info;

int IOT_SetupConnInfo(const char *product_key,
                      const char *device_name,
                      const char *device_secret,
                      void **info_ptr);

#endif  /* _INFRA_COMPAT_H_ */
