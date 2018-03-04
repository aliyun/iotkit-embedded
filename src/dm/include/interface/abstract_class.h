#ifndef ABSTRACT_CLASS_H
#define ABSTRACT_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdarg.h>

typedef struct {
    size_t _size;
    const char*  _class_name;
    void*  (*ctor)(void* _self, va_list* params);
    void*  (*dtor)(void* _self);
} abstract_class_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ABSTRACT_CLASS_H */
