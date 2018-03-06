#include <stdlib.h>
#include "interface/list_abstract.h"
#include "single_list.h"
#include "logger.h"

#include "dm_import.h"

static const char string_single_list_class_name[] __DM_READ_ONLY__ = "slist_cls";
static const char string_single_list_log_pattern[] __DM_READ_ONLY__ = "slist(%s) %s node,size:%d\n";
static const char string_single_list_clear_pattern[] __DM_READ_ONLY__ = "slist(%s) cleared\n";
static const char string_single_list_insert[] __DM_READ_ONLY__ = "insert";
static const char string_single_list_remove[] __DM_READ_ONLY__ = "remove";

static void single_list_insert(void* _self, void* data);
static void single_list_clear(void* _self);

static void* single_list_ctor(void* _self, va_list* params)
{
    single_list_t* self = _self;

    self->_head = (node_t*)dm_lite_calloc(1, sizeof(node_t));

    if (self->_head == NULL) return NULL;

    self->_head->next = NULL;
    self->_head->data = NULL;
    self->_size = 0;

    self->_name = va_arg(*params, char*);

    if (self->_name == NULL) goto err_handler;

    return self;

err_handler:
    if (self->_head) dm_lite_free(self->_head);
    if (self->_name) dm_lite_free(self->_name);

    return NULL;
}

static void* single_list_dtor(void* _self)
{
    single_list_t* self = _self;
    node_t** p = &self->_head;

    self->_size = 0;

    while ((*p) != NULL) {
        node_t* node = *p;
        *p = node->next;
        dm_lite_free(node);
    }

    return self;
}

static void single_list_insert(void* _self, void* data)
{
    single_list_t* self = _self;
    node_t* node = (node_t*)dm_lite_calloc(1, sizeof(node_t));

    node_t** p = (node_t**)&self->_head;
    for (; (*p) != NULL; p = &(*p)->next)
        ;

    node->data = data;
    node->next = *p;
    *p = node;
    self->_size++;

    printf(string_single_list_log_pattern, self->_name, string_single_list_insert, self->_size);
}

static void single_list_remove(void* _self, void* data)
{
    single_list_t* self = _self;
    node_t** p = (node_t**)&self->_head;

    while ((*p) != NULL) {
        node_t* node = *p;
        if (node->data == data) {
            *p = node->next;
            self->_size--;
            printf(string_single_list_log_pattern, self->_name, string_single_list_remove, self->_size);
            dm_lite_free(node);
        } else {
            p = &(*p)->next;
        }
    }
}

static void single_list_clear(void* _self)
{
    single_list_t* self = _self;
    node_t** p = (node_t**)&self->_head->next;

    while ((*p) != NULL) {
        node_t* node = *p;
        *p = node->next;
        dm_lite_free(node);
    }
    self->_head->next = NULL;
    self->_size = 0;
    printf(string_single_list_clear_pattern, self->_name);
}

static int single_list_empty(const void* _self)
{
    const single_list_t* self = _self;

    return self->_head == NULL;
}

static int single_list_get_size(const void* _self)
{
    const single_list_t* self = _self;

    return self->_size;
}

static void single_list_iterator(const void* _self, handle_fp_t handle_fn, va_list* params)
{
    const single_list_t* self = _self;
    node_t** p = &self->_head->next;

    for (; (*p) != NULL; p = &(*p)->next) {
        va_list args;
        va_copy(args, *params);
        handle_fn((*p)->data, &args);
        va_end(args);
    }
}

void list_iterator(const void* _list, handle_fp_t handle_fn, ...)
{
    const list_t** list = (const list_t**)_list;
    va_list params;
    va_start(params, handle_fn);

    (*list)->iterator(list, handle_fn, &params);
    va_end(params);
}

static void single_list_print(const void* _self, print_fp_t print_fn)
{
    const single_list_t* self = _self;
    node_t** p = &self->_head->next;

    while ((*p) != NULL) {
        print_fn((*p)->data);
        p = &(*p)->next;
    }
}

static const list_t _single_list_class = {
    sizeof(single_list_t),
    string_single_list_class_name,
    single_list_ctor,
    single_list_dtor,
    single_list_insert,
    single_list_remove,
    single_list_clear,
    single_list_empty,
    single_list_get_size,
    single_list_iterator,
    single_list_print,
};

const void* get_single_list_class()
{
    return &_single_list_class;
}
