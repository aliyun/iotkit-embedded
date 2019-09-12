/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __WIFI_MSG_H__
#define __WIFI_MSG_H__

#define MSG_REQ_ID_LEN                       16
#define TOPIC_LEN_MAX                        128
#define DEV_INFO_LEN_MAX                     512
#define RANDOM_MAX_LEN                       16
#define WIFI_MAX_NOTIFY_CNT                  60
#define WIFI_NOTIFY_PORT                     5683
#define WIFI_CONNECTAP_NOTIFY_CNT_MAX        60
#define WIFI_CONNECTAP_NOTIFY_DURATION       500
#define DEV_SIGN_SIZE                        (SHA1_DIGEST_SIZE)

#define ILOP_VER                             "1.0"
#define AWSS_VER                             "{\"smartconfig\":\"2.0\",\"zconfig\":\"2.0\",\"router\":\"2.0\",\"ap\":\"2.0\",\"softap\":\"2.0\"}"
/*aha*/
#define TOPIC_AWSS_SWITCHAP                  "/sys/%s/%s/awss/device/switchap"
/*common */
#define TOPIC_AWSS_GET_CONNECTAP_INFO_MCAST  "/sys/awss/device/connectap/info/get"
#define TOPIC_AWSS_GET_CONNECTAP_INFO_UCAST  "/sys/%s/%s/awss/device/connectap/info/get"
/*devinfo notify,dev found */
#define TOPIC_AWSS_NOTIFY                    "/sys/awss/device/info/notify"
/*success notify,common */
#define TOPIC_AWSS_CONNECTAP_NOTIFY          "/sys/awss/event/connectap/notify"
/*register by zero register */
#define TOPIC_NOTIFY                         "/sys/device/info/notify"
/*dev ap*/
#define TOPIC_AWSS_DEV_AP_SWITCHAP           "/sys/awss/device/softap/switchap"
/*zero config mqtt*/
#define TOPIC_ZC_ENROLLEE                    "/sys/%s/%s/thing/awss/enrollee/found"
#define TOPIC_ZC_ENROLLEE_REPLY              "/sys/%s/%s/thing/awss/enrollee/found_reply"
#define TOPIC_ZC_CHECKIN                     "/sys/%s/%s/thing/awss/enrollee/checkin"
#define TOPIC_ZC_CHECKIN_REPLY               "/sys/%s/%s/thing/awss/enrollee/checkin_reply"
#define TOPIC_ZC_CIPHER                      "/sys/%s/%s/thing/cipher/get"
#define TOPIC_ZC_CIPHER_REPLY                "/sys/%s/%s/thing/cipher/get_reply"
#define TOPIC_AWSS_GETDEVICEINFO_MCAST       "/sys/awss/device/info/get"
#define TOPIC_AWSS_GETDEVICEINFO_UCAST       "/sys/%s/%s/awss/device/info/get"

#define METHOD_AWSS_DEV_INFO_NOTIFY          "awss.device.info.notify"
#define METHOD_AWSS_CONNECTAP_NOTIFY         "awss.event.connectap.notify"
#define METHOD_AWSS_DEV_AP_SWITCHAP          "awss.device.softap.switchap"
#define METHOD_EVENT_ZC_SWITCHAP             "thing.awss.device.switchap"
#define METHOD_EVENT_ZC_ENROLLEE             "thing.awss.enrollee.found"
#define METHOD_EVENT_ZC_CHECKIN              "thing.awss.enrollee.checkin"
#define METHOD_EVENT_ZC_CIPHER               "thing.cipher.get"
#define METHOD_MATCH_REPORT                  "thing.awss.enrollee.match"
#define METHOD_LOG_POST                      "things.log.post"

#define WIFI_NOTIFY_HOST                     "255.255.255.255"
#define WIFI_ACK_FMT                         "{\"id\":%s,\"code\":%d,\"data\":%s}"
#define WIFI_REQ_FMT                         "{\"id\":%s,\"version\":\"%s\",\"method\":\"%s\",\"params\":%s}"
#define WIFI_JSON_PARAM                      "params"
#define WIFI_JSON_CODE                       "code"
#define WIFI_JSON_ID                         "id"
#define WIFI_STATIS_FMT                      "{\"template\":\"timestamp logLevel module traceContext logContent\",\"contents\":[\"%u %s %s %u %s\"]}"
#define WIFI_DEV_NOTIFY_FMT                  "{\"id\":\"%u\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":{%s}}"
#define WIFI_SUCCESS_FMT                     ",\"type\":%d"
#define WIFI_DEV_RAND_SIGN_FMT               ",\"random\":\"%s\",\"signMethod\":%d,\"sign\":\"%s\""
#define WIFI_DEV_INFO_FMT                    "\"awssVer\":%s,\"productKey\":\"%s\",\"deviceName\":\"%s\",\"mac\":\"%s\",\"ip\":\"%s\",\"cipherType\":%d"
#define WIFI_DEV_BIND_TOKEN_FMT              ",\"token\":\"%s\",\"remainTime\":%d,\"type\":%d"
#endif
