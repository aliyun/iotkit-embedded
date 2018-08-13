#include "iot_import.h"
#include "iot_export.h"
#include "iotx_utils_internal.h"
#include "utils_box.h"
int ubox_random_string(_IN_ char *random,_IN_ int length)
{
    int index = 0, seed = 0;

    if (random == NULL || length <= 0) {
        utils_err("Invalid Parameter");
        return FAIL_RETURN;
    }

    for (index = 0;index < length;index++) {
        seed = rand() % 3;
        switch (seed)
        {
            case 0:
            {
                random[index] = 'A' + HAL_Random(26);
            }
            break;
            case 1:
            {
                random[index]  = 'a' + HAL_Random(26);
            }
            break;
            case 2:
            {
                random[index] = '0' + HAL_Random(10);
            }
            break;
            default:
            {
                utils_err("Should Not Be Execute Here");
                return FAIL_RETURN;
            }
            break;
        }
    }

    return SUCCESS_RETURN;
}