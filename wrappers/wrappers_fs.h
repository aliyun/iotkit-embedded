#ifndef _WRAPPERS_FS_H_
#define _WRAPPERS_FS_H_

#include "infra_types.h"
#include "infra_defs.h"

typedef enum {
    HAL_SEEK_SET,
    HAL_SEEK_CUR,
    HAL_SEEK_END
} hal_fs_seek_type_t;

void *HAL_Fopen(const char *path, const char *mode);
uint32_t HAL_Fread(void *buff, uint32_t size, uint32_t count, void *stream);
uint32_t HAL_Fwrite(const void *ptr, uint32_t size, uint32_t count, void *stream);
int HAL_Fseek(void *stream, long offset, int framewhere);
int HAL_Fclose(void *stream);
long HAL_Ftell(void *stream);

#endif

