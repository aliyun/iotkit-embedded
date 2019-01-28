#ifdef INFRA_CLASSIC

#if defined(BUILD_AOS)
extern void aos_get_version_hex(unsigned char version[VERSION_NUM_SIZE]);
#else
void aos_get_version_hex(unsigned char version[VERSION_NUM_SIZE])
{
    const char *p_version = LINKKIT_VERSION;
    int i = 0, j = 0;
    unsigned char res = 0;

    for (j = 0; j < 3; j++) {
        for (res = 0; p_version[i] <= '9' && p_version[i] >= '0'; i++) {
            res = res * 10 + p_version[i] - '0';
        }
        version[j] = res;
        i++;
    }
    version[3] = 0x00;
}
#endif

#ifndef BUILD_AOS
unsigned int aos_get_version_info(unsigned char version_num[VERSION_NUM_SIZE],
                                  unsigned char random_num[RANDOM_NUM_SIZE], unsigned char mac_address[MAC_ADDRESS_SIZE],
                                  unsigned char chip_code[CHIP_CODE_SIZE], unsigned char *output_buffer, unsigned int output_buffer_size)
{
    char *p = (char *)output_buffer;

    if (output_buffer_size < AOS_ACTIVE_INFO_LEN) {
        return 1;
    }

    memset(p, 0, output_buffer_size);

    LITE_hexbuf_convert(version_num, p, VERSION_NUM_SIZE, 1);
    p += VERSION_NUM_SIZE * 2;
    LITE_hexbuf_convert(random_num, p, RANDOM_NUM_SIZE, 1);
    p += RANDOM_NUM_SIZE * 2;
    LITE_hexbuf_convert(mac_address, p, MAC_ADDRESS_SIZE, 1);
    p += MAC_ADDRESS_SIZE * 2;
    LITE_hexbuf_convert(chip_code, p, CHIP_CODE_SIZE, 1);
    p += CHIP_CODE_SIZE * 2;
    strcat(p, "1111111111222222222233333333334444444444");

    return 0;
}
#endif

static void *g_event_monitor = NULL;

int iotx_event_regist_cb(void (*monitor_cb)(int event))
{
    g_event_monitor = monitor_cb;
    return 0;
}

int iotx_event_post(int event)
{
    if (g_event_monitor == NULL)
        return -1;
    ((void (*)(int))g_event_monitor)(event);
    return 0;
}

#endif