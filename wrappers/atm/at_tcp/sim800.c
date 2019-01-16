/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */


#include <stdio.h>
#include <string.h>

#include "infra_config.h"

#include "at_wrapper.h"
#include "at_parser.h"
#include "at_api.h"

#define TAG "sim800_gprs_module"

#define SIM800_AT_CMD_SUCCESS_RSP "OK"
#define SIM800_AT_CMD_FAIL_RSP "ERROR"
#define AT_CMD_TEST  "AT"
#define AT_CMD_TEST_RESULT "OK\r\n"

#define AT_CMD_ECHO_OFF   "ATE0"
#define AT_CMD_BAUDRATE_SET "AT+IPR"
#define AT_CMD_FLOW_CONTROL "AT+IFC"

#define AT_CMD_SAVE_CONFIG  "AT&W"

#define AT_CMD_SIM_PIN_CHECK  "AT+CPIN?"
#define AT_CMD_SIGNAL_QUALITY_CHECK "AT+CSQ"
#define AT_CMD_NETWORK_REG_CHECK "AT+CREG?"
#define AT_CMD_GPRS_ATTACH_CHECK "AT+CGATT?"

#define AT_CMD_GPRS_PDP_DEACTIVE "AT+CIPSHUT"
#define AT_CMD_MULTI_IP_CONNECTION "AT+CIPMUX"
#define AT_CMD_TCPIP_MODE "AT+CIPMODE"
#define AT_CMD_SEND_DATA_PROMPT_SET "AT+CIPSPRT"
#define AT_CMD_RECV_DATA_FORMAT_SET "AT+CIPSRIP"

#define AT_CMD_DOMAIN_TO_IP "AT+CDNSGIP"
#define AT_CMD_DOMAIN_RSP   "\r\n+CDNSGIP: "

#define AT_CMD_START_TASK   "AT+CSTT"
#define AT_CMD_BRING_UP_GPRS_CONNECT "AT+CIICR"
#define AT_CMD_GOT_LOCAL_IP "AT+CIFSR"

#define AT_CMD_START_CLIENT_CONN "AT+CIPSTART"
#define AT_CMD_START_TCP_SERVER  "AT+CIPSERVER"

#define AT_CMD_CLIENT_CONNECT_OK "CONNECT OK\r\n"
#define AT_CMD_CLIENT_CONNECT_FAIL "CONNECT FAIL\r\n"

#define AT_CMD_STOP_CONN "AT+CIPCLOSE"

#define AT_CMD_SEND_DATA "AT+CIPSEND"

#define AT_CMD_DATA_RECV "\r\n+RECEIVE,"

#define SIM800_DEFAULT_CMD_LEN    64
#define SIM800_DEFAULT_RSP_LEN    64

#define SIM800_MAX_LINK_NUM       6

#define SIM800_DOMAIN_MAX_LEN     256
#define SIM800_DOMAIN_RSP_MAX_LEN 512
#define SIM800_DOMAIN_CMD_LEN (sizeof(AT_CMD_DOMAIN_TO_IP) + SIM800_DOMAIN_MAX_LEN + 1)

#define SIM800_CONN_CMD_LEN   (SIM800_DOMAIN_MAX_LEN + SIM800_DEFAULT_CMD_LEN)

#define SIM800_RETRY_MAX          50

#ifdef AT_DEBUG_MODE
#define at_conn_hal_err(...)               do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define at_conn_hal_info(...)              do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define at_conn_hal_debug(...)             do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#else
#define at_conn_hal_err(...)               do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define at_conn_hal_info(...)              do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define at_conn_hal_debug(...)
#endif

void *HAL_SemaphoreCreate(void);
void HAL_SemaphoreDestroy(void *sem);
void HAL_SemaphorePost(void *sem);
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms);

/* Change to include data slink for each link id respectively. <TODO> */
typedef struct link_s {
    int fd;
    void* sem_start;
    void* sem_close;
} link_t;

static uint8_t inited = 0;
static link_t g_link[SIM800_MAX_LINK_NUM];
static void *g_link_mutex;
static void *g_domain_mutex;
#ifdef PLATFORM_HAS_OS
static void *g_domain_sem;
#else
static int  g_domain_mark = 0;
#endif
static char  g_pcdomain_rsp[SIM800_DOMAIN_RSP_MAX_LEN];
static char  g_pccmd[SIM800_CONN_CMD_LEN];

static int fd_to_linkid(int fd)
{
    int link_id;

    HAL_MutexLock(g_link_mutex);
    for (link_id = 0; link_id < SIM800_MAX_LINK_NUM; link_id++) {
        if (g_link[link_id].fd == fd) {
            break;
        }
    }

    HAL_MutexUnlock(g_link_mutex);

    return link_id;
}

static void sim800_gprs_domain_rsp_callback(void *arg, char *rspinfo, int rsplen)
{
    if (NULL == rspinfo || rsplen == 0) {
        at_conn_hal_err( "invalid input at %s \r\n", __func__);
        return;
    }

    memcpy(g_pcdomain_rsp, rspinfo, rsplen);
#ifdef PLATFORM_HAS_OS
    HAL_SemaphorePost(g_domain_sem);
#else
    g_domain_mark = 1;
#endif
    return;
}

static void sim800_gprs_module_socket_data_handle(void *arg, char *rspinfo, int rsplen)
{
    unsigned char uclinkid = 0;
    unsigned char unusesymbol = 0;
    unsigned char datalen[6] = {0};
    unsigned char ipaddr[16] = {0};
    unsigned char port[6] = {0};
    int           i = 0;
    int           j = 0;
    int           len = 0;
    int           remoteport = 0;
    int           linkid = 0;
    char          *recvdata = NULL;
    struct at_conn_input param;

    at_read((char *)&uclinkid, 1);
    linkid = uclinkid - '0';
    if (linkid < 0 || linkid >=  SIM800_MAX_LINK_NUM) {
        at_conn_hal_err( "Invalid link id 0x%02x !!!\r\n", linkid);
        return;
    }

    /*eat , char*/
    at_read((char *)&unusesymbol, 1);

    /* get data len */
    i = 0;
    do {
        at_read((char *)&datalen[i], 1);
        if (datalen[i] == ',') {
            break;
        }
        if (i >= sizeof(datalen)) {
            at_conn_hal_err( "Too long length of data.datalen is %s \r\n", datalen);
            return;
        }
        if (datalen[i] > '9' || datalen[i] < '0') {
            at_conn_hal_err( "Invalid len string!!!, datalen is %s \r\n", datalen);
            return;
        }
        i++;
    } while (1);

    /* len: string to number */
    for (j = 0; j < i; j++) {
        len = len * 10 + datalen[j] - '0';
    }

    /*get ip addr and port*/
    i = 0;
    do {
        at_read((char *)&ipaddr[i], 1);
        if (ipaddr[i] == ':') {
            break;
        }
        if (i >= sizeof(ipaddr)) {
            at_conn_hal_err( "Too long length of ipaddr.ipaddr is %s \r\n", ipaddr);
            return;
        }

        if (!((ipaddr[i] <= '9' && ipaddr[i] >= '0') || ipaddr[i] == '.')) {
            at_conn_hal_err( "Invalid ipaddr string!!!, ipaddr is %s \r\n", ipaddr);
            return;
        }
        i++;
    } while (1);

    ipaddr[i] = 0;

    i = 0;
    do {
        at_read((char *)&port[i], 1);
        if (port[i] == '\r') {
            break;
        }
        if (i >= sizeof(port)) {
            at_conn_hal_err( "Too long length of remote port.port is %s \r\n", port);
            return;
        }

        if (port[i] > '9' || port[i] < '0') {
            at_conn_hal_err( "Invalid ipaddr string!!!, port is %s \r\n", port);
            return;
        }
        i++;
    } while (1);

    port[i] = 0;

    /*eat \n char*/
    at_read((char *)&unusesymbol, 1);

    for (j = 0; j < i; j++) {
        remoteport = remoteport * 10 + port[j] - '0';
    }

    /* Prepare socket data */
    recvdata = (char *)HAL_Malloc(len + 1);
    if (!recvdata) {
        at_conn_hal_err( "Error: %s %d out of memory.", __func__, __LINE__);
        return;
    }

    memset(recvdata, 0, len + 1);

    at_read(recvdata, len);

    if (g_link[linkid].fd >= 0) {
        param.fd = g_link[linkid].fd;
        param.data = recvdata;
        param.datalen = len;
        param.remote_ip = (char *)ipaddr;
        param.remote_port = remoteport;

        /* TODO get recv data src ip and port*/
        if (IOT_ATM_Input(&param) != 0) {
            at_conn_hal_err( " %s socket %d get data len %d fail to post to at_conn, drop it\n",
                 __func__, g_link[linkid].fd, len);
        }
    }
    at_conn_hal_debug( "%s socket data on link %d with length %d posted to at_conn\n",
         __func__, linkid, len);
    HAL_Free(recvdata);
    return;
}

int sim800_uart_selfadaption(const char *command, const char *rsp, uint32_t rsplen)
{
    char buffer[SIM800_DEFAULT_RSP_LEN] = {0};
    int retry = 0;

    if (NULL == command || NULL == rsp || 0 == rsplen) {
        at_conn_hal_err( "invalid input %s %d\r\n", __FILE__, __LINE__);
        return -1;
    }

    while (true) {
        retry++;
        at_send_wait_reply(command, strlen(command), true,
                           NULL, 0, buffer, SIM800_DEFAULT_RSP_LEN, NULL);
        if ((strstr(buffer, rsp) == NULL)) {
             HAL_SleepMs(50);

            if (retry > SIM800_RETRY_MAX) {
                return -1;
            }
            at_conn_hal_err( "%s %d failed rsp %s retry count %d\r\n", __func__, __LINE__, rsp, retry);
        } else {
            break;
        }
    }

    return 0;
}

static int sim800_uart_init(void)
{
    int ret = 0;
    char cmd[SIM800_DEFAULT_CMD_LEN] = {0};
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};

    /* uart baudrate self adaption*/
    ret = sim800_uart_selfadaption(AT_CMD_TEST, AT_CMD_TEST_RESULT, strlen(AT_CMD_TEST_RESULT));
    if (ret) {
        at_conn_hal_err( "sim800_uart_selfadaption fail \r\n");
        return ret;
    }

    /*turn off echo*/
    at_send_wait_reply(AT_CMD_ECHO_OFF, strlen(AT_CMD_ECHO_OFF), true, NULL, 0,
                       rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }

    /*set baudrate 115200*/
    HAL_Snprintf(cmd, SIM800_DEFAULT_CMD_LEN - 1, "%s=%d", AT_CMD_BAUDRATE_SET, AT_UART_BAUDRATE);
    at_send_wait_reply(cmd, strlen(cmd), true, NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }

    memset(cmd, 0, SIM800_DEFAULT_CMD_LEN);
    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    /*turn off flow control*/
    HAL_Snprintf(cmd, SIM800_DEFAULT_CMD_LEN - 1, "%s=%d,%d", AT_CMD_FLOW_CONTROL, 0, 0);
    at_send_wait_reply(cmd, strlen(cmd), true, NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }

    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    /*save configuration */
    at_send_wait_reply(AT_CMD_SAVE_CONFIG, strlen(AT_CMD_SAVE_CONFIG), true, NULL, 0,
                       rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }

    return 0;
}

static int sim800_gprs_status_check(void)
{
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};
    int retry = 0;

    while (true) {
        retry++;
        /*sim card status check*/
        at_send_wait_reply(AT_CMD_SIM_PIN_CHECK, strlen(AT_CMD_SIM_PIN_CHECK), true,
                           NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
        if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
            HAL_SleepMs(50);

            if (retry > SIM800_RETRY_MAX) {
                return -1;
            }

            at_conn_hal_err( "%s %d failed rsp %s retry count %d\r\n", __func__, __LINE__, rsp, retry);
        } else {
            break;
        }
    }

    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    /*Signal quaility check*/
    at_send_wait_reply(AT_CMD_SIGNAL_QUALITY_CHECK, strlen(AT_CMD_SIGNAL_QUALITY_CHECK), true,
                       NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }
    at_conn_hal_info( "signal quality is %s \r\n", rsp);

    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    /*network registration check*/
    at_send_wait_reply(AT_CMD_NETWORK_REG_CHECK, strlen(AT_CMD_NETWORK_REG_CHECK), true,
                       NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }
    at_conn_hal_info( "network registration is %s \r\n", rsp);


    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    /*GPRS attach check*/
    at_send_wait_reply(AT_CMD_GPRS_ATTACH_CHECK, strlen(AT_CMD_GPRS_ATTACH_CHECK),true,
                       NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }
    at_conn_hal_info( "gprs attach check %s \r\n", rsp);

    return 0;
}

static int sim800_gprs_ip_init(void)
{
    char cmd[SIM800_DEFAULT_CMD_LEN] = {0};
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};
    int retry = 0;

    while (true) {
        retry++;
        /*Deactivate GPRS PDP Context*/
        at_send_wait_reply(AT_CMD_GPRS_PDP_DEACTIVE, strlen(AT_CMD_GPRS_PDP_DEACTIVE), true,
                           NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
        if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
             HAL_SleepMs(50);

            if (retry > SIM800_RETRY_MAX) {
                return -1;
            }

            at_conn_hal_err( "%s %d failed rsp %s retry count %d\r\n", __func__, __LINE__, rsp, retry);
        } else {
            break;
        }
    }

    /*set multi ip connection mode*/
    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    HAL_Snprintf(cmd, SIM800_DEFAULT_CMD_LEN - 1, "%s=%d", AT_CMD_MULTI_IP_CONNECTION, 1);
    at_send_wait_reply(cmd, strlen(cmd), true, NULL, 0,
                       rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }

    /*not prompt echo > when sending data*/
    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    memset(cmd, 0, SIM800_DEFAULT_CMD_LEN);
    HAL_Snprintf(cmd, SIM800_DEFAULT_CMD_LEN - 1, "%s=%d", AT_CMD_SEND_DATA_PROMPT_SET, 0);
    at_send_wait_reply(cmd, strlen(cmd), true, NULL, 0, 
                       rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }
   
    /*Show Remote ip and port when receive data*/
    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    memset(cmd, 0, SIM800_DEFAULT_CMD_LEN);
    HAL_Snprintf(cmd, SIM800_DEFAULT_CMD_LEN - 1, "%s=%d", AT_CMD_RECV_DATA_FORMAT_SET, 1);
    at_send_wait_reply(cmd, strlen(cmd), true, NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }

    return 0;
}

static int sim800_gprs_got_ip(void)
{
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};
    atcmd_config_t atcmd_config = {NULL, AT_RECV_PREFIX, NULL};
    int retry = 0;

    while (true) {
        retry++;
         /*start gprs stask*/
        at_send_wait_reply(AT_CMD_START_TASK, strlen(AT_CMD_START_TASK), true,
                       NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
        if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
            HAL_SleepMs(50);

            if (retry > SIM800_RETRY_MAX) {
                return -1;
            }
            at_conn_hal_err( "cmd %s rsp %s retry %d at %s %d fail \r\n", AT_CMD_START_TASK,
                        rsp, retry, __func__, __LINE__);
        } else {
            break;
        }
    }

    /*bring up wireless connectiong with gprs*/
    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);
    at_send_wait_reply(AT_CMD_BRING_UP_GPRS_CONNECT, strlen(AT_CMD_BRING_UP_GPRS_CONNECT), true,
                       NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
    }

    /*try to got ip*/
    memset(rsp, 0, SIM800_DEFAULT_RSP_LEN);

    at_send_wait_reply(AT_CMD_GOT_LOCAL_IP, strlen(AT_CMD_GOT_LOCAL_IP), true, NULL, 0,
                       rsp, SIM800_DEFAULT_RSP_LEN, &atcmd_config);
    if (strstr(rsp, SIM800_AT_CMD_FAIL_RSP) != NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
    }

    return 0;
}

static int sim800_gprs_get_ip_only()
{
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};
    atcmd_config_t atcmd_config = {NULL, AT_RECV_PREFIX, NULL};
    at_send_wait_reply(AT_CMD_GOT_LOCAL_IP, strlen(AT_CMD_GOT_LOCAL_IP), true,
                        NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, &atcmd_config);
    if (strstr(rsp, SIM800_AT_CMD_FAIL_RSP) != NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        return -1;
    }
    return 0;
}

int HAL_AT_CONN_Init(void)
{
    int ret = 0;
    uint32_t linknum = 0;

    if (inited) {
        at_conn_hal_info( "sim800 gprs module have already inited \r\n");
        return 0;
    }

    memset(g_pcdomain_rsp , 0, SIM800_DOMAIN_RSP_MAX_LEN);

    if (NULL == (g_link_mutex = HAL_MutexCreate())) {
        at_conn_hal_err( "Creating link mutex failed (%s %d).", __func__, __LINE__);
        goto err;
    }

    if (NULL == (g_domain_mutex = HAL_MutexCreate())) {
        at_conn_hal_err( "Creating link mutex failed (%s %d).", __func__, __LINE__);
        goto err;
    }

#ifdef PLATFORM_HAS_OS
    if (NULL == (g_domain_sem = HAL_SemaphoreCreate())) {
        at_conn_hal_err( "Creating domain mutex failed (%s %d).", __func__, __LINE__);
        goto err;
    }
#endif

    memset(g_link, 0, sizeof(g_link));

    for (linknum = 0; linknum < SIM800_MAX_LINK_NUM; linknum++) {
        g_link[linknum].fd = -1;
    }

    ret = sim800_uart_init();
    if (ret) {
        at_conn_hal_err( "%s %d failed \r\n", __func__, __LINE__);
        goto err;
    }

    ret = sim800_gprs_status_check();
    if (ret) {
        at_conn_hal_err( "%s %d failed \r\n", __func__, __LINE__);
        goto err;
    }
 
    ret = sim800_gprs_ip_init();
    if (ret) {
        at_conn_hal_err( "%s %d failed \r\n", __func__, __LINE__);
        goto err;
    }

    /* reg oob for domain and packet input*/
    at_register_callback(AT_CMD_DOMAIN_RSP, AT_RECV_PREFIX, SIM800_DOMAIN_RSP_MAX_LEN,
                         sim800_gprs_domain_rsp_callback, NULL);
    at_register_callback(AT_CMD_DATA_RECV, NULL, 0, sim800_gprs_module_socket_data_handle, NULL);
    ret = sim800_gprs_got_ip();
    if (ret) {
        at_conn_hal_err( "%s %d failed \r\n", __func__, __LINE__);
        goto err;
    }

    inited = 1;

    return 0;
err:
    if (g_link_mutex != NULL) {
        HAL_MutexDestroy(g_link_mutex);
    }

    if (g_domain_mutex != NULL) {
        HAL_MutexDestroy(g_domain_mutex);
    }

#ifdef PLATFORM_HAS_OS
    if (g_domain_sem != NULL) {
        HAL_SemaphoreDestroy(g_domain_sem);
    }
#endif

    return -1;
}

int HAL_AT_CONN_Deinit()
{
    if (!inited) {
        return 0;
    }

    HAL_MutexDestroy(g_link_mutex);
    inited = 0;
    return 0;
}

int HAL_AT_CONN_DomainToIp(char *domain, char ip[16])
{
    char *pccmd = NULL;
    char *head = NULL;
    char *end = NULL;
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};

    if (!inited) {
        at_conn_hal_err( "%s sim800 gprs module haven't init yet \r\n", __func__);
        return -1;
    }

    if (NULL == domain || NULL == ip) {
        at_conn_hal_err( "invalid input at %s \r\n", __func__);
        return -1;
    }

    if (strlen(domain) > SIM800_DOMAIN_MAX_LEN) {
        at_conn_hal_err( "domain length oversize at %s \r\n", __func__);
        return -1;
    }

    pccmd = g_pccmd;
    if (NULL == pccmd) {
        at_conn_hal_err( "fail to malloc memory %d at %s \r\n", SIM800_DOMAIN_CMD_LEN, __func__);
        return -1;
    }

    memset(pccmd, 0, SIM800_DOMAIN_CMD_LEN);
    HAL_Snprintf(pccmd, SIM800_DOMAIN_CMD_LEN - 1, "%s=%s", AT_CMD_DOMAIN_TO_IP, domain);

    HAL_MutexLock(g_domain_mutex);
restart:
    at_send_wait_reply(pccmd, strlen(pccmd), true, NULL, 0, rsp,
                       SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
        goto err;
    }

#ifdef PLATFORM_HAS_OS
    /*TODO wait for reponse for ever for now*/
    HAL_SemaphoreWait(g_domain_sem, PLATFORM_WAIT_INFINITE);
#else
    while(!g_domain_mark) {
        at_yield(NULL, 0, NULL, 100);
        HAL_SleepMs(50);
    }
    g_domain_mark = 0;
#endif

    /*
     * formate is :
       +CDNSGIP: 1,"www.baidu.com","183.232.231.173","183.232.231.172"
       or :
       +CDNSGIP: 0,8
    */
    if ((head = strstr(g_pcdomain_rsp, domain)) == NULL) {
        at_conn_hal_err( "invalid domain rsp %s at %d\r\n", g_pcdomain_rsp, __LINE__);
        goto err;
    }

    head += (strlen(domain) + 3);
    if ((end = strstr(head, "\"")) == NULL) {
        at_conn_hal_err( "invalid domain rsp head is %s at %d\r\n", head, __LINE__);
        goto err;
    }

    if ((end - head) > 15 || (end - head) < 7) {
        at_conn_hal_err( "invalid domain rsp head is %s at %d\r\n", head, __LINE__);
        goto err;
    }

    /* We find a good IP, save it. */
    memcpy(ip, head, end - head);
    ip[end - head] = '\0';
    memset(g_pcdomain_rsp, 0, SIM800_DOMAIN_RSP_MAX_LEN);
    HAL_MutexUnlock(g_domain_mutex);

    return 0;
err:
    if (sim800_gprs_get_ip_only() != 0) {
        sim800_gprs_ip_init();
        sim800_gprs_got_ip();
        goto restart;
    }

    memset(g_pcdomain_rsp, 0, SIM800_DOMAIN_RSP_MAX_LEN);
    HAL_MutexUnlock(g_domain_mutex);

    return -1;
}

int HAL_AT_CONN_Start(at_conn_t *conn)
{
    int  linkid = 0;
    char *pccmd = NULL;
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};
    atcmd_config_t atcmd_config_client = { NULL, AT_CMD_CLIENT_CONNECT_OK, AT_CMD_CLIENT_CONNECT_FAIL};

    if (!inited) {
        at_conn_hal_err( "%s sim800 gprs module haven't init yet \r\n", __func__);
        return -1;
    }

    if (!conn || !conn->addr) {
        at_conn_hal_err( "%s %d - invalid input \r\n", __func__, __LINE__);
        return -1;
    }

    HAL_MutexLock(g_link_mutex);
    for (linkid = 0; linkid < SIM800_MAX_LINK_NUM; linkid++) {
        if (g_link[linkid].fd >= 0) {
            continue;
        }
        g_link[linkid].fd = conn->fd;
        break;
    }
    HAL_MutexUnlock(g_link_mutex);

    if (linkid >= SIM800_MAX_LINK_NUM) {
        at_conn_hal_err( "No link available for now, %s failed. \r\n", __func__);
        return -1;
    }

    pccmd = g_pccmd;
    if (NULL == pccmd) {
        at_conn_hal_err( "fail to malloc %d at %s \r\n", SIM800_CONN_CMD_LEN, __func__);
        goto err;
    }
    memset(pccmd, 0, SIM800_CONN_CMD_LEN);

    switch (conn->type) {
        case TCP_SERVER:
            HAL_Snprintf(pccmd, SIM800_CONN_CMD_LEN - 1, "%s=%d,%d", AT_CMD_START_TCP_SERVER, 1, conn->l_port);
            at_send_wait_reply(pccmd, strlen(pccmd), true, NULL, 0,
                               rsp, SIM800_DEFAULT_RSP_LEN, NULL);
            if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
                at_conn_hal_err( "%s %d failed rsp %s\r\n", __func__, __LINE__, rsp);
                goto err;
            }
            break;
        case TCP_CLIENT:
            HAL_Snprintf(pccmd, SIM800_CONN_CMD_LEN - 1, "%s=%d,\"TCP\",\"%s\",%d", AT_CMD_START_CLIENT_CONN, linkid, conn->addr,
                     conn->r_port);
             
            at_send_wait_reply(pccmd, strlen(pccmd), true, NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN, 
                                    &atcmd_config_client);
            if (strstr(rsp, AT_CMD_CLIENT_CONNECT_FAIL) != NULL) {
                at_conn_hal_err( "pccmd %s fail, rsp %s \r\n", pccmd, rsp);
                goto err;
            }
            break;
        case UDP_UNICAST:
            HAL_Snprintf(pccmd, SIM800_CONN_CMD_LEN - 1, "%s=%d,\"UDP\",\"%s\",%d", AT_CMD_START_CLIENT_CONN, linkid, conn->addr,
                     conn->r_port);
           
            at_send_wait_reply(pccmd, strlen(pccmd), true, NULL, 0, rsp, SIM800_DEFAULT_RSP_LEN,
                                    &atcmd_config_client);
            if (strstr(rsp, AT_CMD_CLIENT_CONNECT_FAIL) != NULL) {
                at_conn_hal_err( "pccmd %s fail, rsp %s \r\n", pccmd, rsp);
                goto err;
            }
            break;
        case SSL_CLIENT:
        case UDP_BROADCAST:
        default:
            at_conn_hal_err( "sim800 gprs module connect type %d not support \r\n", conn->type);
            goto err;
    }

    return 0;
err:
    HAL_MutexLock(g_link_mutex);
    g_link[linkid].fd = -1;
    HAL_MutexUnlock(g_link_mutex);
    return -1;
}

int HAL_AT_CONN_Close(int fd, int32_t remote_port)
{
    int  linkid = 0;
    int  ret = 0;
    char cmd[SIM800_DEFAULT_CMD_LEN] = {0};
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};

    if (!inited) {
        at_conn_hal_err( "%s sim800 gprs module haven't init yet \r\n", __func__);
        return -1;
    }

    linkid = fd_to_linkid(fd);
    if (linkid < 0 || linkid >= SIM800_MAX_LINK_NUM) {
        at_conn_hal_err( "No connection found for fd (%d) in %s \r\n", fd, __func__);
        return -1;
    }

    HAL_Snprintf(cmd, SIM800_DEFAULT_CMD_LEN - 1, "%s=%d", AT_CMD_STOP_CONN, linkid);
    at_send_wait_reply(cmd, strlen(cmd), true, NULL, 0, 
                       rsp, SIM800_DEFAULT_RSP_LEN, NULL);
    if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
        at_conn_hal_err( "cmd %s rsp is %s \r\n", cmd, rsp);
        ret = -1;
    }

    HAL_MutexLock(g_link_mutex);
    g_link[linkid].fd = -1;
    HAL_MutexUnlock(g_link_mutex);

    return ret;
}

int HAL_AT_CONN_Send(int fd,
                 uint8_t *data,
                 uint32_t len,
                 char remote_ip[16],
                 int32_t remote_port,
                 int32_t timeout)
{
    int  linkid;
    char cmd[SIM800_DEFAULT_CMD_LEN] = {0};
    char rsp[SIM800_DEFAULT_RSP_LEN] = {0};
    int retry = 0;

    if (!inited) {
        at_conn_hal_err( "%s sim800 gprs module haven't init yet \r\n", __func__);
        return -1;
    }

    linkid = fd_to_linkid(fd);
    if (linkid < 0 || linkid >= SIM800_MAX_LINK_NUM) {
        at_conn_hal_err( "No connection found for fd (%d) in %s \r\n", fd, __func__);
        return -1;
    }

    HAL_Snprintf(cmd, SIM800_DEFAULT_CMD_LEN - 1, "%s=%d,%d", AT_CMD_SEND_DATA, linkid, len);
    
    while (true) {
        retry++;
        /*TODO data send fail rsp is SEND FAIL*/
        at_send_wait_reply((const char *)cmd, strlen(cmd), true, (const char *)data, len,
                            rsp, sizeof(rsp), NULL);
        if (strstr(rsp, SIM800_AT_CMD_SUCCESS_RSP) == NULL) {
            HAL_SleepMs(50);

            if (retry > SIM800_RETRY_MAX) {
                return -1;
            }
            at_conn_hal_err( "cmd %s rsp %s retry %d at %s %d fail \r\n", cmd, rsp, retry, __func__, __LINE__);
        } else {
            break;
        }
    }

    return 0;
}
