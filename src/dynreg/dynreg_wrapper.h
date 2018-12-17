#ifndef _DYNREG_WRAPPER_H_
#define _DYNREG_WRAPPER_H_

#include "infra_types.h"

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);

#endif