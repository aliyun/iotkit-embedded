/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "at_wrapper.h"

#include "uart.h"
#include "atparser.h"
#include "atparser_internal.h"

#ifdef HDLC_UART
#include "hdlc.h"
#endif

#define MODULE_NAME            "atparser"

static uint8_t    inited = 0;
static uart_dev_t at_uart;

#ifdef HDLC_UART
static encode_context_t hdlc_encode_ctx;
static decode_context_t hdlc_decode_ctx;
#endif

static at_parser_t at = { ._oobs     = { { 0 } },
                          ._oobs_num = 0};

static void* at_worker(void *arg);

static void at_uart_configure(uart_dev_t *u)
{
    u->port                = AT_UART_PORT;
    u->config.baud_rate    = AT_UART_BAUDRATE;
    u->config.data_width   = AT_UART_DATA_WIDTH;
    u->config.parity       = AT_UART_PARITY;
    u->config.stop_bits    = AT_UART_STOP_BITS;
    u->config.flow_control = AT_UART_FLOW_CONTROL;
    u->config.mode         = AT_UART_MODE;
}

static int at_init_uart()
{
    at_uart_configure(&at_uart);

    if (hal_uart_init(&at_uart) != 0) {
        return -1;
    }

#ifdef HDLC_UART
    if (hdlc_encode_context_init(&hdlc_encode_ctx) != 0 ||
        hdlc_decode_context_init(&hdlc_decode_ctx, &at_uart) != 0) {
        return -1;
    }
#endif

    at._pstuart = &at_uart;

    return 0;
}

static void at_set_timeout(int timeout)
{
    at._timeout = timeout;
}

static void at_set_recv_delimiter(const char *recv_prefix,
                                  const char *recv_success_postfix,
                                  const char *recv_fail_postfix)
{
    at._default_recv_prefix          = (char *)recv_prefix;
    at._default_recv_success_postfix = (char *)recv_success_postfix;
    at._default_recv_fail_postfix    = (char *)recv_fail_postfix;
    at._recv_prefix_len              = strlen(recv_prefix);
    at._recv_success_postfix_len     = strlen(recv_success_postfix);
    at._recv_fail_postfix_len        = strlen(recv_fail_postfix);
}

static void at_set_send_delimiter(const char *delimiter)
{
    at._send_delimiter  = (char *)delimiter;
    at._send_delim_size = strlen(delimiter);
}

static int at_init_task_mutex()
{
    at.task_mutex = HAL_MutexCreate();
    if (NULL == at.task_mutex) {
        atparser_err("Creating task mutex failed\r\n");
        return -1;
    }

    return 0;
}

static void at_deinit_task_mutex()
{
    if (at.task_mutex) {
        HAL_MutexDestroy(at.task_mutex);
    }
    return;
}

static int at_init_uart_recv_mutex()
{
    at.at_uart_recv_mutex = HAL_MutexCreate();
    if (NULL == at.at_uart_recv_mutex) {
        atparser_err("Creating at_uart_recv_mutex failed\r\n");
        return -1;
    }

    return 0;
}

static void at_deinit_uart_recv_mutex()
{
    if (at.at_uart_recv_mutex) {
        HAL_MutexDestroy(at.at_uart_recv_mutex);
    }
    return;
}

static int at_worker_uart_send_mutex_init()
{
    at.at_uart_send_mutex = HAL_MutexCreate();
    if (NULL == at.at_uart_send_mutex) {
        atparser_err("Creating at worker sem failed\r\n");
        return -1;
    }
    return 0;
}

static void at_worker_uart_send_mutex_deinit()
{
    if (at.at_uart_send_mutex) {
        HAL_MutexDestroy(at.at_uart_send_mutex);
    }
}

int at_init()
{
    void *task;
    int stack_used;
    hal_os_thread_param_t task_parms = {0};

    char *recv_prefix = AT_RECV_PREFIX;
    char *recv_success_postfix = AT_RECV_SUCCESS_POSTFIX;
    char *recv_fail_postfix = AT_RECV_FAIL_POSTFIX;
    char *send_delimiter = AT_SEND_DELIMITER;
    int  timeout = AT_UART_TIMEOUT_MS;

    if (inited == 1) {
        atparser_info("have already inited ,it will init again\r\n");
        return -1;
    }

    if (at_init_uart() != 0) {
        atparser_err("at uart init fail \r\n");
        return -1;
    }

    memset(at._oobs, 0, sizeof(oob_t) * OOB_MAX);

    at_set_timeout(timeout);
    at_set_recv_delimiter(recv_prefix, recv_success_postfix, recv_fail_postfix);
    at_set_send_delimiter(send_delimiter);

    if (at_init_uart_recv_mutex() != 0) {
        atparser_err("at_uart_recv_mutex init fail \r\n");
        return -1;
    }

    if (at_init_task_mutex() != 0) {
        at_deinit_uart_recv_mutex();
        atparser_err("at mutex init fail \r\n");
        return -1;
    }

    slist_init(&at.task_l);

    if (at_worker_uart_send_mutex_init() != 0) {
        at_deinit_uart_recv_mutex();
        at_deinit_task_mutex();
        atparser_err("fail to creat at worker sem\r\n");
    }

    task_parms.priority = os_thread_priority_normal;
    task_parms.stack_size = AT_WORKER_STACK_SIZE;
    task_parms.name = "at_worker";
    if (HAL_ThreadCreate(&task, at_worker, NULL, &task_parms, &stack_used) != 0) {
        at_deinit_uart_recv_mutex();
        at_deinit_task_mutex();
        at_worker_uart_send_mutex_deinit();
        atparser_err("fail to creat at task\r\n");
        return -1;
    }

    HAL_SleepMs(500);
    return 0;
}

static int at_sendto_lower(uart_dev_t *uart, void *data, uint32_t size,
                           uint32_t timeout, bool ackreq)
{
    int ret = -1;

#ifdef HDLC_UART
    ret = hdlc_uart_send(&hdlc_encode_ctx, uart, data,
                         size, timeout, ackreq);
#else
    (void) ackreq;
    ret = hal_uart_send(uart, data, size, timeout);
#endif

    return ret;
}

static int at_recvfrom_lower(uart_dev_t *uart, void *data, uint32_t expect_size,
                             uint32_t *recv_size, uint32_t timeout)
{
    int ret = -1;

#ifdef HDLC_UART
    ret = hdlc_uart_recv(&hdlc_decode_ctx, uart, data, expect_size,
                         recv_size, timeout);
#else
    ret = hal_uart_recv_II(uart, data, expect_size, recv_size, timeout);
#endif

    return ret;
}

static int at_worker_task_add(at_task_t *tsk)
{
    if (NULL == tsk) {
        atparser_err("invalid input %s \r\n", __func__);
        return -1;
    }

    HAL_MutexLock(at.task_mutex);
    slist_add_tail(&tsk->next, &at.task_l);
    HAL_MutexUnlock(at.task_mutex);

    return 0;
}

static int at_worker_task_del(at_task_t *tsk)
{
    if (NULL == tsk) {
        atparser_err("invalid input %s \r\n", __func__);
        return -1;
    }

    HAL_MutexLock(at.task_mutex);
    slist_del(&tsk->next, &at.task_l);
    HAL_MutexUnlock(at.task_mutex);
    if (tsk->smpr) {
        HAL_SemaphoreDestroy(tsk->smpr);
    }
    if (tsk) {
        atparser_free(tsk);
    }

    return 0;
}

int at_send_wait_reply(const char *data, int datalen, bool delimiter,
                       char *replybuf, int bufsize,
                       const atcmd_config_t *atcmdconfig)
{ 
    int ret = 0;

    if (inited == 0) {
        atparser_err("at have not init yet\r\n");
        return -1;
    }

    if (NULL == data || datalen <= 0) {
        atparser_err("%s invalid input \r\n", __FUNCTION__);
        return -1;
    }

    if (NULL == replybuf || 0 == bufsize) {
        atparser_err("%s invalid input \r\n", __FUNCTION__);
        return -1;
    }

    at_task_t *tsk = (at_task_t *)atparser_malloc(sizeof(at_task_t));
    if (NULL == tsk) {
        atparser_err("tsk buffer allocating failed");
        return -1;
    }
    memset(tsk, 0, sizeof(at_task_t));

    tsk->smpr = HAL_SemaphoreCreate();
    if (NULL == tsk->smpr) {
        atparser_err("failed to allocate semaphore");
        goto end;
    }

    if (atcmdconfig) {
        if (NULL != atcmdconfig->reply_prefix) {
            tsk->rsp_prefix     = atcmdconfig->reply_prefix;
            tsk->rsp_prefix_len = strlen(atcmdconfig->reply_prefix);
        }

        if (NULL != atcmdconfig->reply_success_postfix) {
            tsk->rsp_success_postfix     = atcmdconfig->reply_success_postfix;
            tsk->rsp_success_postfix_len = strlen(atcmdconfig->reply_success_postfix);
        }

        if (NULL != atcmdconfig->reply_fail_postfix) {
            tsk->rsp_fail_postfix     = atcmdconfig->reply_fail_postfix;
            tsk->rsp_fail_postfix_len = strlen(atcmdconfig->reply_fail_postfix);
        }
    }

    tsk->command = (char *)data;
    tsk->rsp     = replybuf;
    tsk->rsp_len = bufsize;

    HAL_MutexLock(at.at_uart_send_mutex);
    at_worker_task_add(tsk);

    if ((ret = at_sendto_lower(at._pstuart, (void *)data, datalen,
                               at._timeout, true)) != 0) {
        atparser_err("uart send command failed");
        goto end;
    }

    if (delimiter) {
        if ((ret = at_sendto_lower(at._pstuart, (void *)at._send_delimiter,
                    strlen(at._send_delimiter), at._timeout, false)) != 0) {
            atparser_err("uart send delimiter failed");
            goto end;
        }
    }

    if ((ret = HAL_SemaphoreWait(tsk->smpr, TASK_DEFAULT_WAIT_TIME)) != 0) {
        atparser_err("sem_wait failed");
        goto end;
    }

end:
    at_worker_task_del(tsk);
    HAL_MutexUnlock(at.at_uart_send_mutex);
    return ret;
}

int at_send_no_reply(const char *data, int datalen, bool delimiter)
{
    int ret;

    if (inited == 0) {
        atparser_err("at have not init yet\r\n");
        return -1;
    }

    if (NULL == data || datalen <= 0) {
        atparser_err("invalid input \r\n");
        return -1;
    }

    HAL_MutexLock(at.at_uart_send_mutex);
    if ((ret = at_sendto_lower(at._pstuart, (void *)data,
                               datalen, at._timeout, true)) != 0) {
        atparser_err("uart send raw content (%s) failed", data);
        HAL_MutexUnlock(at.at_uart_send_mutex);
        return -1;
    }

    if (delimiter) {
        if ((ret = at_sendto_lower(at._pstuart, (void *)at._send_delimiter,
                    strlen(at._send_delimiter), at._timeout, false)) != 0) {
            atparser_err("uart send delimiter failed");
            HAL_MutexUnlock(at.at_uart_send_mutex);
            return -1;
        }
    }
    HAL_MutexUnlock(at.at_uart_send_mutex);

    return 0;
}

int at_connect_wifi(char *ssid, char *pwd)
{
    char conn_str[100]= {0};
    char out[20] = {0};

    sprintf(conn_str, "AT+WJAP=%s,%s", ssid, pwd);

    if (at_send_wait_reply(conn_str, strlen(conn_str), true,
                       out, sizeof(out), NULL) < 0){
        atparser_err("%s %d failed", __func__, __LINE__);
        return -1;
    }

    if (strstr(out, "ERROR") != NULL) {
        atparser_err("%s %d failed", __func__, __LINE__);
        return -1;
    }

    HAL_SleepMs(3000);

    return 0;
}


static int at_getc(char *c)
{
    int      ret = 0;
    char     data;
    uint32_t recv_size = 0;

    if (NULL == c) {
        return -1;
    }

    if (inited == 0) {
        atparser_err("at have not init yet\r\n");
        return -1;
    }

    HAL_MutexLock(at.at_uart_recv_mutex);
    ret = at_recvfrom_lower(at._pstuart, (void *)&data, 1, &recv_size, at._timeout);
    HAL_MutexUnlock(at.at_uart_recv_mutex);

    if (ret != 0) {
#ifdef WORKAROUND_DEVELOPERBOARD_DMA_UART
        if (ret == 1) {
            atparser_warning("--->AT dma fail, restart!\n");
            hal_uart_finalize(at._pstuart);
            at_init_uart();
            atparser_warning("<----AT dma fail, restart!\n");
        }
#endif
        return -1;
    }

    if (recv_size == 1) {
        *c = data;
        return 0;
    } else {
        return -1;
    }
}

int at_read(char *outbuf, int readsize)
{
    int      ret = 0;
    uint32_t recv_size, total_read = 0;

    if (inited == 0) {
        atparser_err("at have not init yet\r\n");
        return -1;
    }

    HAL_MutexLock(at.at_uart_recv_mutex);
    while (total_read < readsize) {
        ret = at_recvfrom_lower(at._pstuart, (void *)(outbuf + total_read),
                                readsize - total_read, &recv_size, at._timeout);
        if (ret != 0) {
            atparser_err("at_read failed on uart_recv.");
            break;
        }

        if (recv_size <= 0) {
            continue;
        }
        total_read += recv_size;
        if (total_read >= readsize) {
            break;
        }
    }
    HAL_MutexUnlock(at.at_uart_recv_mutex);

    if (ret != 0) {
        return -1;
    }

    return total_read;
}

// register oob
int at_register_callback(const char *prefix, const char *postfix,
                         int maxlen, at_recv_cb cb, void *arg)
{
    oob_t *oob = NULL;
    int    i   = 0;

    if (maxlen < 0 || NULL == prefix) {
        atparser_err("%s invalid input \r\n", __func__);
        return -1;
    }

    if (at._oobs_num >= OOB_MAX) {
        atparser_err("No place left in OOB.\r\n");
        return -1;
    }

    /*check oob is exit*/
    for (i = 0; i < at._oobs_num; i++) {
        if (strcmp(prefix, at._oobs[i].prefix) == 0) {
            atparser_warning("oob prefix %s is already exist.\r\n", prefix);
            return -1;
        }
    }

    oob = &(at._oobs[at._oobs_num++]);

    oob->oobinputdata = NULL;
    if (postfix != NULL) {
        oob->oobinputdata = atparser_malloc(maxlen);
        if (NULL == oob->oobinputdata) {
            atparser_err("fail to malloc len %d at %s for prefix %s \r\n",
                 maxlen, __func__, prefix);
            return -1;
        }
        memset(oob->oobinputdata, 0, maxlen);
    }


    oob->maxlen  = maxlen;
    oob->prefix  = (char *)prefix;
    oob->postfix = (char *)postfix;
    oob->cb      = cb;
    oob->arg     = arg;
    oob->reallen = 0;

    atparser_debug("New oob registered (%s)", oob->prefix);

    return 0;
}

#define RECV_BUFFER_SIZE 512
static void* at_worker(void *arg)
{
    int        offset                  = 0;
    int        ret                     = 0;
    int        at_task_empty           = 0;
    int        at_task_reponse_begin   = 0;
    int        memcpy_size             = 0;
    int        rsp_prefix_len          = 0;
    int        rsp_success_postfix_len = 0;
    int        rsp_fail_postfix_len    = 0;
    char       c;
    at_task_t *tsk;
    char      *buf                 = NULL;
    char      *rsp_prefix          = NULL;
    char      *rsp_success_postfix = NULL;
    char      *rsp_fail_postfix    = NULL;
    oob_t     *oob                 = NULL;

    atparser_debug("at_work started.");

    buf = atparser_malloc(RECV_BUFFER_SIZE);
    if (NULL == buf) {
        atparser_err("AT worker fail to malloc ,task exist \r\n");
        return NULL;
    }

    memset(buf, 0, RECV_BUFFER_SIZE);
    inited = 1;

    while (true) {
        // read from uart and store buf
        ret = at_getc(&c);
        if (ret != 0) {
            continue;
        }

        if (offset + 1 >= RECV_BUFFER_SIZE) {
            atparser_err("Fatal error, no one is handling AT uart");
            goto check_buffer;
        }
        buf[offset++] = c;
        buf[offset]   = 0;

        for (int k = 0; k < at._oobs_num; k++) {
            oob = &(at._oobs[k]);
            if (oob->reallen > 0 ||
                (offset >= strlen(oob->prefix) &&
                 memcmp(oob->prefix, buf + offset - strlen(oob->prefix),
                        strlen(oob->prefix)) == 0)) {
                //atparser_debug("AT! %s\r\n", oob->prefix);
                if (oob->postfix == NULL) {
                    oob->cb(oob->arg, NULL, 0);
                    memset(buf, 0, offset);
                    offset = 0;
                } else {
                    if (oob->reallen == 0) {
                        int len = strlen(oob->prefix) - 1;
                        len = len > 0 ? len : 0;
                        memset(oob->oobinputdata, 0, oob->maxlen);
                        memcpy(oob->oobinputdata, oob->prefix, len);
                        oob->reallen += len;
                    }

                    if (oob->reallen < oob->maxlen) {
                        oob->oobinputdata[oob->reallen] = c;
                        oob->reallen++;
                        if ((oob->reallen >=
                             strlen(oob->prefix) + strlen(oob->postfix)) &&
                            (strncmp(oob->oobinputdata + oob->reallen -
                                       strlen(oob->postfix),
                                     oob->postfix,
                                     strlen(oob->postfix)) == 0)) {
                            /*recv postfix*/
                            oob->cb(oob->arg, oob->oobinputdata, oob->reallen);
                            memset(oob->oobinputdata, 0, oob->reallen);
                            oob->reallen = 0;
                            memset(buf, 0, offset);
                            offset = 0;
                        }
                    } else {
                        atparser_err("invalid oob %s input , for oversize %s \r\n",
                             oob->prefix, oob->oobinputdata);
                        memset(oob->oobinputdata, 0, oob->reallen);
                        oob->reallen = 0;
                        memset(buf, 0, offset);
                        offset = 0;
                    }
                    /*oob data maybe more than buf size */
                    if (offset > (RECV_BUFFER_SIZE - 2)) {
                        memset(buf, 0, offset);
                        offset = 0;
                    }
                }
                continue;
            }
        }

        HAL_MutexLock(at.task_mutex);
        at_task_empty = slist_empty(&at.task_l);

        if (!at_task_empty) {
            tsk = slist_first_entry(&at.task_l, at_task_t, next);
        }
        HAL_MutexUnlock(at.task_mutex);

        // if no task, continue recv
        if (at_task_empty) {
            //atparser_debug("No task in queue");
            goto check_buffer;
        }

        if (NULL != tsk->rsp_prefix && 0 != tsk->rsp_prefix_len) {
            rsp_prefix     = tsk->rsp_prefix;
            rsp_prefix_len = tsk->rsp_prefix_len;
        } else {
            rsp_prefix     = at._default_recv_prefix;
            rsp_prefix_len = at._recv_prefix_len;
        }

        if (NULL != tsk->rsp_success_postfix &&
            0 != tsk->rsp_success_postfix_len) {
            rsp_success_postfix     = tsk->rsp_success_postfix;
            rsp_success_postfix_len = tsk->rsp_success_postfix_len;
        } else {
            rsp_success_postfix     = at._default_recv_success_postfix;
            rsp_success_postfix_len = at._recv_success_postfix_len;
        }

        if (NULL != tsk->rsp_fail_postfix && 0 != tsk->rsp_fail_postfix_len) {
            rsp_fail_postfix     = tsk->rsp_fail_postfix;
            rsp_fail_postfix_len = tsk->rsp_fail_postfix_len;
        } else {
            rsp_fail_postfix     = at._default_recv_fail_postfix;
            rsp_fail_postfix_len = at._recv_fail_postfix_len;
        }

        if (offset >= rsp_prefix_len && at_task_reponse_begin == 0 &&
            (strncmp(buf + offset - rsp_prefix_len, rsp_prefix,
                     rsp_prefix_len) == 0)) {
            at_task_reponse_begin = 1;
        }

        if (at_task_reponse_begin == 1) {
            if (tsk->rsp_offset < tsk->rsp_len) {
                tsk->rsp[tsk->rsp_offset] = c;
                tsk->rsp_offset++;

                if ((tsk->rsp_offset >= rsp_success_postfix_len &&
                     strncmp(
                       tsk->rsp + tsk->rsp_offset - rsp_success_postfix_len,
                       rsp_success_postfix, rsp_success_postfix_len) == 0) ||
                    (tsk->rsp_offset >= rsp_fail_postfix_len &&
                     strncmp(tsk->rsp + tsk->rsp_offset - rsp_fail_postfix_len,
                             rsp_fail_postfix, rsp_fail_postfix_len) == 0)) {
                    HAL_SemaphorePost(tsk->smpr);
                    at_task_reponse_begin = 0;
                    memset(buf, 0, offset);
                    offset = 0;
                }
            } else {
                memset(tsk->rsp, 0, tsk->rsp_len);
                strcpy(tsk->rsp, rsp_fail_postfix);
                HAL_SemaphorePost(tsk->smpr);
                at_task_reponse_begin = 0;
                memset(buf, 0, offset);
                offset = 0;
            }
        }
    check_buffer:
        // in case buffer is full
        if (offset > (RECV_BUFFER_SIZE - 2)) {
            atparser_debug("buffer full \r\n");
            memcpy_size = rsp_prefix_len > rsp_success_postfix_len
                            ? rsp_prefix_len
                            : rsp_success_postfix_len;
            memcpy_size = memcpy_size > rsp_fail_postfix_len
                            ? memcpy_size
                            : rsp_fail_postfix_len;
            memcpy(buf, buf + offset - memcpy_size, memcpy_size);
            memset(buf + memcpy_size, 0, offset - memcpy_size);
            offset = memcpy_size;
        }
    }

    return NULL;
}


