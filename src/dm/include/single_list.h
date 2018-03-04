#ifndef SINGLE_LIST_H
#define SINGLE_LIST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "interface/log_abstract.h"

#define SINGLE_LIST_CLASS get_single_list_class()

void list_iterator(const void* _list, handle_fp_t handle_fn, ...);

typedef struct _node {
    void* data;
    struct _node* next;
} node_t;

typedef struct {
    const void* _;
    node_t*     _head;
    int         _size;
    char*       _name;
} single_list_t;

extern const void* get_single_list_class();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SINGLE_LIST_H */
