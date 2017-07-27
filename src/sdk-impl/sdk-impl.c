#include "sdk-impl_internal.h"

int IOT_SetupConnInfo(void)
{
    return iotx_guider_authenticate();
}
