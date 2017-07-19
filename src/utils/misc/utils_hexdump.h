
#ifndef _IOTX_DUMP_H_
#define _IOTX_DUMP_H_

typedef enum {
    IOTX_HEXDUMP_PREFIX_NONE,
    IOTX_HEXDUMP_PREFIX_ADDRESS,
    IOTX_HEXDUMP_PREFIX_OFFSET
} utils_hexdump_prefix_type_t;


void utils_hexdump(utils_hexdump_prefix_type_t prefix_type,
                   int rowsize,
                   int groupsize,
                   const void *buf,
                   size_t len,
                   int ascii);

#endif /* IOTX_DUMP_H_ */
