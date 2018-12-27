/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "alink_core.h"
#include "infra_defs.h"



typedef struct {
    uint32_t devid;
    char product_key[IOTX_PRODUCT_KEY_LEN];
    char device_name[IOTX_DEVICE_NAME_LEN];
    char device_secret[IOTX_DEVICE_SECRET_LEN];
    alink_device_status_t status;
    list_head_t list;
} alink_subdev_node_t;


typedef struct {
    void           *mutex;
    uint16_t        subdev_num;
    uint32_t        devid_alloc;
    list_head_t     dev_list;
} alink_subdev_ctx_t;


alink_subdev_ctx_t alink_subdev_ctx;


static uint32_t alink_subdev_allocate_devid(void)
{
    uint32_t devid;


    devid = ++alink_subdev_ctx.devid_alloc;


    return devid;
}

/**
 * move from dm
 */
static int alink_subdev_search_node_by_pkdn(const char *product_key, const char *device_name, alink_subdev_node_t **node)
{
    alink_subdev_ctx_t *ctx = &alink_subdev_ctx;        // TODO
    alink_subdev_node_t *search_node = NULL;

    list_for_each_entry(search_node, &ctx->dev_list, list, alink_subdev_node_t) {
        if ((strlen(search_node->product_key) == strlen(product_key)) &&
            (memcmp(search_node->product_key, product_key, strlen(product_key)) == 0) &&
            (strlen(search_node->device_name) == strlen(device_name)) &&
            (memcmp(search_node->device_name, device_name, strlen(device_name)) == 0)) {
            alink_info("Device Found, Product Key: %s, Device Name: %s", product_key, device_name);
            if (node) {
                *node = search_node;
            }
            return SUCCESS_RETURN;
        }
    }

    alink_info("Device Not Found, Product Key: %s, Device Name: %s", product_key, device_name);
    return FAIL_RETURN;
}

/**
 * move from dm
 */
static int alink_subdev_search_node_by_devid(uint32_t devid, alink_subdev_node_t **node)
{
    alink_subdev_ctx_t *ctx = &alink_subdev_ctx;        // TODO
    alink_subdev_node_t *search_node = NULL;

    list_for_each_entry(search_node, &ctx->dev_list, list, alink_subdev_node_t) {
        if (search_node->devid == devid) {
            alink_info("Device Found, devid: %d", devid);
            if (node) {
                *node = search_node;
            }
            return SUCCESS_RETURN;
        }
    }

    alink_info("Device Not Found, devid: %d", devid);
    return FAIL_RETURN;
}

/**
 * 
 */
int alink_subdev_create(const char *product_key, const char *device_name, const char *device_secret, int *devid)
{
    int res = FAIL_RETURN;
    alink_subdev_ctx_t *ctx = &alink_subdev_ctx;
    alink_subdev_node_t *node = NULL;

    ALINK_ASSERT_DEBUG(product_key != NULL);
    ALINK_ASSERT_DEBUG(device_name != NULL);
    ALINK_ASSERT_DEBUG(devid != NULL);

    res = alink_subdev_search_node_by_pkdn(product_key, device_name, &node);
    if (res == SUCCESS_RETURN) {
        // TOOD: already exist
        return FAIL_RETURN;
    }

    node = HAL_Malloc(sizeof(alink_subdev_node_t));
    if (node == NULL) {
        return ALINK_CODE_MEMORY_NOT_ENOUGH;
    }
    memset(node, 0, sizeof(alink_subdev_node_t));

    node->devid = alink_subdev_allocate_devid();
    *devid = node->devid;

    memcpy(node->product_key, product_key, strlen(product_key));
    memcpy(node->device_name, device_name, strlen(device_name));
    if (device_secret != NULL) {
        memcpy(node->device_secret, device_secret, strlen(device_secret));
    }

    //node->status = ALINK_DEV_STATUS_AUTHORIZED;
    INIT_LIST_HEAD(&node->list);
    list_add_tail(&node->list, &ctx->dev_list);

    return SUCCESS_RETURN;
}

int alink_subdev_destroy(uint32_t devid)
{
    ALINK_ASSERT_DEBUG(devid > 0);

    int res = 0;

    alink_subdev_node_t *node = NULL;

    res = alink_subdev_search_node_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    list_del(&node->list);
    HAL_Free(node);

    return SUCCESS_RETURN;
}






int alink_subdev_search_dev_by_devid(int devid, char *product_key, char *device_name, char *device_secret)
{
    ALINK_ASSERT_DEBUG(devid > 0);

    int res = 0;
    alink_subdev_node_t *node = NULL;

    res = alink_subdev_search_node_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    if (product_key != NULL) {
        memcpy(product_key, node->product_key, strlen(node->product_key));
    }
    
    if (device_name != NULL) {
        memcpy(device_name, node->device_name, strlen(node->device_name));
    }
    
    if (device_secret != NULL) {
        memcpy(device_secret, node->device_secret, strlen(node->device_secret));
    }

    return SUCCESS_RETURN;
}

int alink_subdev_search_devid_by_pkdn(const char *product_key, const char *device_name, uint32_t *devid)
{
    ALINK_ASSERT_DEBUG(product_key != NULL);
    ALINK_ASSERT_DEBUG(device_name != NULL);
    ALINK_ASSERT_DEBUG(devid != NULL);

    int res = 0;
    alink_subdev_node_t *node = NULL;

    res = alink_subdev_search_node_by_pkdn(product_key, device_name, &node);
    if (res != SUCCESS_RETURN) {
        *devid = 0;
        return FAIL_RETURN;
    }

    *devid = node->devid;

    return SUCCESS_RETURN;
}

