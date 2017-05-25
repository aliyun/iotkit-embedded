
#ifndef ALIOT_DUMP_H_
#define ALIOT_DUMP_H_

typedef enum {
	ALIOT_HEXDUMP_PREFIX_NONE,
	ALIOT_HEXDUMP_PREFIX_ADDRESS,
	ALIOT_HEXDUMP_PREFIX_OFFSET
} aliot_hexdump_prefix_type_t;


void aliyun_iot_common_hexdump(aliot_hexdump_prefix_type_t prefix_type,
            int rowsize,
            int groupsize,
            const void *buf,
            size_t len,
            int ascii);

#endif /* ALIOT_DUMP_H_ */
