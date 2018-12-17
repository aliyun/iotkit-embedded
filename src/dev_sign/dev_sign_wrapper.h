

#include "infra_types.h"
#include "infra_defs.h"

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
int8_t HAL_Snprintf(char *str, const int len, const char *fmt, ...);