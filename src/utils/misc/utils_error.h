

#ifndef _IOTX_COMMON_ERROR_H_
#define _IOTX_COMMON_ERROR_H_




// #define IOTX_TRACE
/**
 * @brief Debug level trace logging macro.
 *
 * Macro to print message function entry and exit
 */
#ifdef IOTX_TRACE
#define IOTX_FUNC_ENTRY    \
    {\
        printf("FUNC_ENTRY:   %s L#%d \n", __PRETTY_FUNCTION__, __LINE__);  \
    }
#define IOTX_FUNC_EXIT    \
    {\
        printf("FUNC_EXIT:   %s L#%d \n", __PRETTY_FUNCTION__, __LINE__);  \
    }
#define IOTX_FUNC_EXIT_RC(x)    \
    {\
        printf("FUNC_EXIT:   %s L#%d Return Code : %d \n", __PRETTY_FUNCTION__, __LINE__, x);  \
        return x; \
    }
#else

#define IOTX_FUNC_ENTRY
#define IOTX_FUNC_EXIT
#define IOTX_FUNC_EXIT_RC(x) { return x; }
#endif

#endif
