/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "sdk-testsuites_internal.h"

int main(int argc, char *argv[])
{
    LITE_openlog("test");
    LITE_set_loglevel(LOG_DEBUG_LEVEL);
    LITE_dump_malloc_free_stats(LOG_DEBUG_LEVEL);

    unittest_HAL_OS();
    unittest_string_utils();
    unittest_json_token();

#ifdef MQTT_ID2_AUTH
    uint64_t    fake_timestamp = 1493274903;

    fake_timestamp = fake_timestamp * 1000;
    unittest_tfs(fake_timestamp);
#endif

    LITE_malloc(1);
    LITE_calloc(2, 4);
    LITE_dump_malloc_free_stats(LOG_DEBUG_LEVEL);
    LITE_closelog();
    return 0;
}

void unittest_HAL_OS(void)
{
    log_info("HAL_UptimeMs() = %" PRIu64, HAL_UptimeMs());
    HAL_SleepMs(100);
    log_info("HAL_UptimeMs() = %" PRIu64 " (+100)", HAL_UptimeMs());
    HAL_SleepMs(200);
    log_info("HAL_UptimeMs() = %" PRIu64 " (+200)", HAL_UptimeMs());
    HAL_SleepMs(400);
    log_info("HAL_UptimeMs() = %" PRIu64 " (+400)", HAL_UptimeMs());

    HAL_Srandom((uint32_t)HAL_UptimeMs());
    log_info("HAL_Random(100) = %u", HAL_Random(100));
    log_info("HAL_Random(50) = %u", HAL_Random(50));
    log_info("HAL_Random(20) = %u", HAL_Random(20));

    return;
}
