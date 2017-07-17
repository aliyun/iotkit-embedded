#ifndef __LITE_UTILS_INTERNAL_H__
#define __LITE_UTILS_INTERNAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lite/lite-log.h"

#include "lite-utils_config.h"
#include "lite-utils.h"

void       *LITE_malloc_internal(const char *f, const int l, int size);
void       *LITE_realloc_internal(const char *f, const int l, void *ptr, int size);
void        LITE_free_internal(void *ptr);

#endif  /* __LITE_UTILS_INTERNAL_H__ */
