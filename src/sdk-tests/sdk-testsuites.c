#include "sdk-testsuites_internal.h"

int main(int argc, char *argv[])
{
    LITE_openlog("test");
    LITE_set_loglevel(LOG_DEBUG_LEVEL);
    LITE_dump_malloc_free_stats(LOG_DEBUG_LEVEL);

    unittest_string_utils();
    unittest_json_token();
    unittest_crypto();

    LITE_malloc(1);
    LITE_calloc(2, 4);
    LITE_dump_malloc_free_stats(LOG_DEBUG_LEVEL);
    LITE_closelog();
    return 0;
}
