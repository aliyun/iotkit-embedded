/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifdef INFRA_NET
#include <stdio.h>
#include <string.h>

#include "infra_net.h"
uintptr_t HAL_TCP_Establish(const char *host, uint16_t port);
int HAL_TCP_Destroy(uintptr_t fd);
int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms);
int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms);
void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
uintptr_t HAL_SSL_Establish(const char *host,
                            uint16_t port,
                            const char *ca_crt,
                            size_t ca_crt_len);
int32_t HAL_SSL_Destroy(uintptr_t handle);
int HAL_SSL_Read(uintptr_t handle, char *buf, int len, int timeout_ms);
int HAL_SSL_Write(uintptr_t handle, const char *buf, int len, int timeout_ms);
int HAL_SSLHooks_set(ssl_hooks_t *hooks);

/*** TCP connection ***/
int read_tcp(utils_network_pt pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    return HAL_TCP_Read(pNetwork->handle, buffer, len, timeout_ms);
}

static int write_tcp(utils_network_pt pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
    return HAL_TCP_Write(pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_tcp(utils_network_pt pNetwork)
{
    if (pNetwork->handle == (uintptr_t)(-1)) {
        return -1;
    }

    HAL_TCP_Destroy(pNetwork->handle);
    pNetwork->handle = -1;
    return 0;
}

static int connect_tcp(utils_network_pt pNetwork)
{
    if (NULL == pNetwork) {
        return 1;
    }

    pNetwork->handle = HAL_TCP_Establish(pNetwork->pHostAddress, pNetwork->port);
    if (pNetwork->handle == (uintptr_t)(-1)) {
        return -1;
    }

    return 0;
}

/*** SSL connection ***/
#ifdef SUPPORT_TLS
static void *ssl_malloc(uint32_t size)
{
#ifdef INFRA_MEM_STATS
    return LITE_malloc(size, MEM_MAGIC, "tls");
#else
    return HAL_Malloc(size);
#endif
}
static void ssl_free(void *ptr)
{
#ifdef INFRA_MEM_STATS
    LITE_free(ptr);
#else
    HAL_Free(ptr);
#endif
}

static int read_ssl(utils_network_pt pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork) {
        return -1;
    }

    return HAL_SSL_Read((uintptr_t)pNetwork->handle, buffer, len, timeout_ms);
}

static int write_ssl(utils_network_pt pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork) {
        return -1;
    }

    return HAL_SSL_Write((uintptr_t)pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_ssl(utils_network_pt pNetwork)
{
    if (NULL == pNetwork) {
        return -1;
    }

    HAL_SSL_Destroy((uintptr_t)pNetwork->handle);
    pNetwork->handle = 0;

    return 0;
}

static int connect_ssl(utils_network_pt pNetwork)
{
    ssl_hooks_t ssl_hooks;

    if (NULL == pNetwork) {
        return 1;
    }

    memset(&ssl_hooks, 0, sizeof(ssl_hooks_t));
    ssl_hooks.malloc = ssl_malloc;
    ssl_hooks.free = ssl_free;

    HAL_SSLHooks_set(&ssl_hooks);

    if (0 != (pNetwork->handle = (intptr_t)HAL_SSL_Establish(
            pNetwork->pHostAddress,
            pNetwork->port,
            pNetwork->ca_crt,
            pNetwork->ca_crt_len + 1))) {
        return 0;
    } else {
        /* TODO SHOLUD not remove this handle space */
        /* The space will be freed by calling disconnect_ssl() */
        /* utils_memory_free((void *)pNetwork->handle); */
#if 0
        iotx_event_post(IOTX_CONN_CLOUD_FAIL);
#endif
        return -1;
    }
}
#endif  /* #ifndef SUPPORT_TLS */

/*** iTLS connection ***/
#if defined(SUPPORT_ITLS)
static int read_itls(utils_network_pt pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork) {
        return -1;
    }

    return HAL_SSL_Read((uintptr_t)pNetwork->handle, buffer, len, timeout_ms);
}

static int write_itls(utils_network_pt pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork) {
        return -1;
    }

    return HAL_SSL_Write((uintptr_t)pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_itls(utils_network_pt pNetwork)
{
    if (NULL == pNetwork) {
        return -1;
    }

    HAL_SSL_Destroy((uintptr_t)pNetwork->handle);
    pNetwork->handle = 0;

    return 0;
}

static int connect_itls(utils_network_pt pNetwork)
{
    if (NULL == pNetwork) {
        return 1;
    }

    char pkps[PRODUCT_KEY_LEN + PRODUCT_SECRET_LEN] = {0};
    int len = strlen(pNetwork->product_key);
    strncpy(pkps, pNetwork->product_key, len);
    HAL_GetProductSecret(pkps + len + 1);
    len += strlen(pkps + len + 1) + 2;

    if (0 != (pNetwork->handle = (intptr_t)HAL_SSL_Establish(
            pNetwork->pHostAddress,
            pNetwork->port,
            pkps, len))) {
        return 0;
    } else {
        /* TODO SHOLUD not remove this handle space */
        /* The space will be freed by calling disconnect_ssl() */
        /* utils_memory_free((void *)pNetwork->handle); */
        return -1;
    }
}
#endif  /* #ifndef IOTX_WITHOUT_iTLS */

/****** network interface ******/
int utils_net_read(utils_network_pt pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    int     ret = 0;

    if (NULL == pNetwork->ca_crt && NULL == pNetwork->product_key) {
        ret = read_tcp(pNetwork, buffer, len, timeout_ms);
    }
#if defined(SUPPORT_ITLS)
    else if (NULL == pNetwork->ca_crt && NULL != pNetwork->product_key) {
        ret = read_itls(pNetwork, buffer, len, timeout_ms);
    }
#endif
#ifdef SUPPORT_TLS
    else if (NULL != pNetwork->ca_crt && NULL == pNetwork->product_key) {
        ret = read_ssl(pNetwork, buffer, len, timeout_ms);
    }
#endif
    else {
        ret = -1;
    }

    return ret;
}

int utils_net_write(utils_network_pt pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
    int     ret = 0;

    if (NULL == pNetwork->ca_crt && NULL == pNetwork->product_key) {
        ret = write_tcp(pNetwork, buffer, len, timeout_ms);
    }
#if defined(SUPPORT_ITLS)
    else if (NULL == pNetwork->ca_crt && NULL != pNetwork->product_key) {
        ret = write_itls(pNetwork, buffer, len, timeout_ms);
    }
#endif
#ifdef SUPPORT_TLS
    else if (NULL != pNetwork->ca_crt && NULL == pNetwork->product_key) {
        ret = write_ssl(pNetwork, buffer, len, timeout_ms);
    }
#endif
    else {
        ret = -1;
    }

    return ret;
}

int iotx_net_disconnect(utils_network_pt pNetwork)
{
    int     ret = 0;

    if (NULL == pNetwork->ca_crt && NULL == pNetwork->product_key) {
        ret = disconnect_tcp(pNetwork);
    }
#if defined(SUPPORT_ITLS)
    else if (NULL == pNetwork->ca_crt && NULL != pNetwork->product_key) {
        ret = disconnect_itls(pNetwork);
    }
#endif
#ifdef SUPPORT_TLS
    else if (NULL != pNetwork->ca_crt && NULL == pNetwork->product_key) {
        ret = disconnect_ssl(pNetwork);
    }
#endif
    else {
        ret = -1;
    }

    return  ret;
}

int iotx_net_connect(utils_network_pt pNetwork)
{
    int     ret = 0;

    if (NULL == pNetwork->ca_crt && NULL == pNetwork->product_key) {
        ret = connect_tcp(pNetwork);
    }
#if defined(SUPPORT_ITLS)
    else if (NULL == pNetwork->ca_crt && NULL != pNetwork->product_key) {
        ret = connect_itls(pNetwork);
    }
#endif
#ifdef SUPPORT_TLS
    else if (NULL != pNetwork->ca_crt && NULL == pNetwork->product_key) {
        ret = connect_ssl(pNetwork);
    }
#endif
    else {
        ret = -1;
    }

    return ret;
}

int iotx_net_init(utils_network_pt pNetwork, const char *host, uint16_t port, const char *ca_crt, char *product_key)
{
    if (!pNetwork || !host) {
        return -1;
    }
    pNetwork->pHostAddress = host;
    pNetwork->port = port;
    pNetwork->ca_crt = ca_crt;
#if !defined(SUPPORT_ITLS)
    pNetwork->product_key = NULL;
#else
    pNetwork->product_key = product_key;
#endif

    if (NULL == ca_crt) {
        pNetwork->ca_crt_len = 0;
    } else {
        pNetwork->ca_crt_len = strlen(ca_crt);
    }

    pNetwork->handle = 0;
    pNetwork->read = utils_net_read;
    pNetwork->write = utils_net_write;
    pNetwork->disconnect = iotx_net_disconnect;
    pNetwork->connect = iotx_net_connect;

    return 0;
}
#endif