/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "alink_core.h"
#include "infra_defs.h"

/* index value may overflow! */ 
#define ALINK_SUBDEV_INDEX_VALUE_MAX        1000000     

/* upper limit of cloud is 1500 */ 
#define ALINK_SUBDEV_NUM_MAX                1500        

#define ALINK_SUBDEV_HTABLE_SIZE_MAX        2000

#define ALINK_SUBDEV_HTABLE_SIZE            53          /* TODO, 151 */


typedef struct _subdev_hash_node *subdev_hash_table_t;

typedef struct _subdev_hash_node {
    char *product_key;
    char *device_name;
    char *device_secret;
    uint32_t devid;
    alink_subdev_status_t status;
    struct _subdev_hash_node *next;
} subdev_hash_node_t;

typedef struct {
    subdev_hash_table_t *hash_table;
    uint32_t table_size;
    uint32_t subdev_num;
    uint32_t devid_alloc;
    void *mutex;
} alink_subdev_mgr_htable_t;

/***************************************************************
 * local function prototypes
 ***************************************************************/
void _subdev_hash_destroy(subdev_hash_table_t *hash_table, uint32_t size);


/***************************************************************
 * local variables define
 ***************************************************************/
alink_subdev_mgr_htable_t subdev_mgr_htable = { 0 };


/***************************************************************
 * local functions
 ***************************************************************/
static void _alink_subdev_mgr_lock(void)
{
    if (subdev_mgr_htable.mutex) {
        HAL_MutexLock(subdev_mgr_htable.mutex);
    }
}

static void _alink_subdev_mgr_unlock(void)
{
    if (subdev_mgr_htable.mutex) {
        HAL_MutexUnlock(subdev_mgr_htable.mutex);
    }
}

/***************************************************************
 * global functions
 ***************************************************************/
int alink_subdev_mgr_init(void)
{
    subdev_mgr_htable.mutex = HAL_MutexCreate();
    if (subdev_mgr_htable.mutex == NULL) {
        return FAIL_RETURN;
    }

    subdev_mgr_htable.table_size = ALINK_SUBDEV_HTABLE_SIZE;     /* TODO!!! */
    subdev_mgr_htable.subdev_num = 0;
    subdev_mgr_htable.devid_alloc = 0;

    subdev_mgr_htable.hash_table = alink_malloc(sizeof(subdev_hash_table_t *) * subdev_mgr_htable.table_size);
    if (subdev_mgr_htable.hash_table == NULL) {
        HAL_MutexDestroy(subdev_mgr_htable.mutex);
        subdev_mgr_htable.mutex = NULL;
        subdev_mgr_htable.table_size = 0;
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    memset(subdev_mgr_htable.hash_table, 0, sizeof(*subdev_mgr_htable.hash_table) * subdev_mgr_htable.table_size);

    return SUCCESS_RETURN;
}

int alink_subdev_mgr_deinit(void)
{
    _alink_subdev_mgr_lock();

    _subdev_hash_destroy(subdev_mgr_htable.hash_table, subdev_mgr_htable.table_size);
    alink_free(subdev_mgr_htable.hash_table);
    subdev_mgr_htable.table_size = 0;
    subdev_mgr_htable.subdev_num = 0;
    subdev_mgr_htable.devid_alloc = 0;

    _alink_subdev_mgr_unlock();

    if (subdev_mgr_htable.mutex != NULL) {
        HAL_MutexDestroy(subdev_mgr_htable.mutex);
        subdev_mgr_htable.mutex = NULL;
    }

    return SUCCESS_RETURN;    
}

/** TODO **/
static uint32_t _pkdn_to_hash(const char *pk, const char *dn)
{
    uint8_t i;
    uint32_t sum = 0;
    uint8_t pk_len, dn_len;

    pk_len = strlen(pk);
    dn_len = strlen(dn);

    for (i = 0; i < pk_len; i++)
    {
        sum += pk[i];
    }

    for (i = 0; i < dn_len; i++)
    {
        sum += dn[i];
        if (dn[i] >= '0' && dn[i] <= '9') {
            sum >>= (dn[i] - '0');
        }
    }

    for (; i < 32; i++)
    {
        sum++;
    }

    sum += pk_len;
    sum += dn_len;
    sum = sum % subdev_mgr_htable.table_size;

    return sum;
}

/** TODO: add mutex **/
int _subdev_hash_insert(const char *pk, const char *dn, const char *ds) 
{
    int res = FAIL_RETURN;
    uint32_t hash = _pkdn_to_hash(pk, dn);
    struct _subdev_hash_node **table = subdev_mgr_htable.hash_table;
    subdev_hash_node_t *node, *temp;

    _alink_subdev_mgr_lock();

    do {
        if (subdev_mgr_htable.devid_alloc++ >= ALINK_SUBDEV_INDEX_VALUE_MAX) {
            res = IOTX_CODE_SUBDEV_IDX_OVERFLOW;
            break;
        }
        if (subdev_mgr_htable.subdev_num >= ALINK_SUBDEV_NUM_MAX) {
            res = IOTX_CODE_TOO_MANY_SUBDEV;
            break;
        }

        node = (subdev_hash_node_t *)alink_malloc(sizeof(subdev_hash_node_t));
        if (node == NULL) {
            res = IOTX_CODE_MEMORY_NOT_ENOUGH;
            break;
        }

        node->product_key = alink_utils_strdup(pk, strlen(pk));
        if (node->product_key == NULL) {
            alink_free(node);
            res = IOTX_CODE_MEMORY_NOT_ENOUGH;
            break;
        }
        node->device_name = alink_utils_strdup(dn, strlen(dn));
        if (node->device_name == NULL) {
            alink_free(node->product_key);
            alink_free(node);
            res = IOTX_CODE_MEMORY_NOT_ENOUGH;
            break;
        }

        if (strlen(ds)) {
            node->device_secret = alink_utils_strdup(ds, strlen(ds));
            if (node->device_secret == NULL) {
                alink_free(node->product_key);
                alink_free(node->device_name);
                alink_free(node);
                res = IOTX_CODE_MEMORY_NOT_ENOUGH;
                break;
            }
        }
        else {
            node->device_secret = NULL;
        }

        /* add list node */
        node->next = NULL;
        temp = table[hash];
        table[hash] = node;
        node->next = temp;

        node->devid = hash * ALINK_SUBDEV_INDEX_VALUE_MAX + subdev_mgr_htable.devid_alloc;
        subdev_mgr_htable.subdev_num++;
        _alink_subdev_mgr_unlock();

        return node->devid;
    } while (0);

    _alink_subdev_mgr_unlock();
    return res;
}

/** TODO **/
int _subdev_hash_remove(uint32_t devid)
{
    uint32_t hash;
    subdev_hash_node_t *node, *temp;

    ALINK_ASSERT_DEBUG(devid != 0);

    hash = devid / ALINK_SUBDEV_INDEX_VALUE_MAX;
    if (hash >= subdev_mgr_htable.table_size) {
        return FAIL_RETURN;
    }
    node = subdev_mgr_htable.hash_table[hash];

    alink_info("devid = %d", node->devid);

    if (node->devid == devid) {
        temp = node->next;
        alink_free(node->product_key);
        alink_free(node->device_name);
        if (node->device_secret) {
            alink_free(node->device_secret);
        }
        alink_free(node);
        subdev_mgr_htable.hash_table[hash] = temp;
        return SUCCESS_RETURN;
    }
    else {
        temp = node;
        node = node->next;
        while (node) {
            if (node->devid == devid) {
                temp->next = node->next;
                alink_free(node->product_key);
                alink_free(node->device_name);
                if (node->device_secret) {
                    alink_free(node->device_secret);
                }
                alink_free(node);
                return SUCCESS_RETURN;
            }
            else {
                temp = node;
                node = node->next;
            }
        }
    }

    return FAIL_RETURN;
}

#if UTILS_HASH_TABLE_ITERATOR_ENABLE
void subdev_hash_iterator(void)
{
    struct _subdev_hash_node **table = subdev_mgr_htable.hash_table;

    subdev_hash_node_t *node;
    uint32_t idx;

    for (idx = 0; idx < subdev_mgr_htable.table_size; idx++) {
        if (table[idx] == NULL ) {
            alink_debug("hTable[%d] = NULL", idx);
            continue;
        }
        else {
            node = table[idx];
            alink_debug("hTable[%d] = %s", idx, node->device_name);

            while (node->next) {
                node = node->next;
                alink_debug("hTable[%d] = %s *", idx, node->device_name);
            }
        }
    }
}
#endif


/** TODO: add mutex **/
subdev_hash_node_t *_subdev_hash_search_by_pkdn(const char *pk, const char *dn)
{
    uint32_t hash;
    subdev_hash_node_t *node;
    uint8_t pk_len;
    uint8_t dn_len;

    hash = _pkdn_to_hash(pk, dn);
    node = subdev_mgr_htable.hash_table[hash];

    pk_len = strlen(pk);
    dn_len = strlen(dn);

    while (node) {
        if (pk_len == strlen(node->product_key) && !memcmp(pk, node->product_key, pk_len) &&
            dn_len == strlen(node->device_name) && !memcmp(dn, node->device_name, dn_len)) {
            return node;
        }
        else {
            node = node->next;
        }
    }

    return NULL;
}

subdev_hash_node_t *_subdev_hash_search_by_devid(uint32_t devid)
{
    uint32_t hash;
    subdev_hash_node_t *node;

    hash = devid / ALINK_SUBDEV_INDEX_VALUE_MAX;     /* TODO */
    node = subdev_mgr_htable.hash_table[hash];

    while (node) {
        if (devid == node->devid) {
            return node;
        }
        else {
            node = node->next;
        }
    }

    return NULL;
}

void _subdev_hash_destroy(subdev_hash_table_t *hash_table, uint32_t size)
{
    subdev_hash_table_t *table = hash_table;
    subdev_hash_node_t *node, *temp;
    uint8_t idx;

    for (idx = 0; idx < size; idx++) {
        if (table[idx] == NULL) {
            continue;
        }

        node = table[idx];
        table[idx] = NULL;
        temp = node->next;
        alink_free(node->product_key);
        alink_free(node->device_name);
        if (node->device_secret) {
            alink_free(node->device_secret);
        }
        alink_free(node);

        while (temp) {
            node = temp;
            temp = temp->next;
            alink_free(node->product_key);
            alink_free(node->device_name);
            if (node->device_secret) {
                alink_free(node->device_secret);
            }
            alink_free(node);
        }
    }
}


int alink_subdev_open(iotx_dev_meta_info_t *dev_info)
{
    subdev_hash_node_t *node = NULL;
    int res = 0;
    
    node = _subdev_hash_search_by_pkdn(dev_info->product_key, dev_info->device_name);
    if (node != NULL) {
        /* subdev already add, just return the devid */
        return node->devid;
    }

    /* return devid if insert successfully */
    res = _subdev_hash_insert(dev_info->product_key, dev_info->device_name, dev_info->device_secret);
    if (res <= SUCCESS_RETURN) {
        alink_info("subdev add failed");
        return res;
    }
    alink_info("subdev add succeed");

    return res;
}

int alink_subdev_close(uint32_t devid)
{
    int res = FAIL_RETURN;

    _alink_subdev_mgr_lock();
    res = _subdev_hash_remove(devid);
    _alink_subdev_mgr_unlock();

    return res;
}

int alink_subdev_connect_cloud(uint32_t devid)
{
    subdev_hash_node_t *node;

    /* check core status first */
    if (alink_core_get_status() != ALINK_CORE_STATUS_CONNECTED) {
        return IOTX_CODE_STATUS_ERROR;
    }

    node = _subdev_hash_search_by_devid(devid);
    if (node == NULL) {
        return IOTX_CODE_SUBDEV_NOT_EXIST;
    }

    /* dynamic register first if device_secret is NULL */
    if (node->device_secret == NULL) {
        return alink_subdev_register(devid);
    }

    /* TODO: reply */
    node->status = ALINK_SUBDEV_STATUS_REGISTERED;

    /* TODO: login?? */

    return SUCCESS_RETURN;
}

int alink_subdev_register(uint32_t devid)
{
    int res = FAIL_RETURN;
    uint32_t subdev_id = devid;

    alink_subdev_id_list_t subdev_id_list;
    subdev_id_list.subdev_array = &subdev_id;
    subdev_id_list.subdev_num = 1; 

    res = alink_upstream_subdev_register_post_req(&subdev_id_list);
    if (res < SUCCESS_RETURN) {
        alink_info("subdev register post failed");
        return res;
    }
    alink_info("subdev register post succeed");

    return SUCCESS_RETURN;
}

int alink_subdev_unregister(uint32_t devid)
{
    int res = FAIL_RETURN;
    uint32_t subdev_id = devid;

    alink_subdev_id_list_t subdev_id_list;
    subdev_id_list.subdev_array = &subdev_id;
    subdev_id_list.subdev_num = 1; 

    res = alink_upstream_subdev_register_delete_req(&subdev_id_list);
    if (res < SUCCESS_RETURN) {
        alink_info("subdev unregister post failed");
        return res;
    }
    alink_info("subdev unregister post succeed");

    return SUCCESS_RETURN;
}

int alink_subdev_login(uint32_t *devid, uint8_t devid_num)
{
    int res = FAIL_RETURN;
    alink_subdev_id_list_t subdev_id_list;
    subdev_id_list.subdev_array = devid;
    subdev_id_list.subdev_num = devid_num;

    /* check core status first */
    if (alink_core_get_status() != ALINK_CORE_STATUS_CONNECTED) {
        return IOTX_CODE_STATUS_ERROR;
    }

    res = alink_upstream_subdev_login_post_req(&subdev_id_list);
    if (res < SUCCESS_RETURN) {
        alink_info("subdev login post failed");
        return res;
    }
    alink_info("subdev login post succeed"); 
    return res;
}

int alink_subdev_logout(uint32_t *devid, uint8_t devid_num)
{
    int res = FAIL_RETURN;
    alink_subdev_id_list_t subdev_id_list;
    subdev_id_list.subdev_array = devid;
    subdev_id_list.subdev_num = devid_num;

    /* check core status first */
    if (alink_core_get_status() != ALINK_CORE_STATUS_CONNECTED) {
        return IOTX_CODE_STATUS_ERROR;
    }

    res = alink_upstream_subdev_login_delete_req(&subdev_id_list);
    if (res < SUCCESS_RETURN) {
        alink_info("subdev logout post failed");
        return res;
    }
    alink_info("subdev logout post succeed"); 
    return res;
}


int alink_subdev_get_pkdn_by_devid(uint32_t devid, char *product_key, char *device_name)
{
    subdev_hash_node_t *node;

    _alink_subdev_mgr_lock();

    node = _subdev_hash_search_by_devid(devid);
    if (node == NULL) {
        _alink_subdev_mgr_unlock();
        return FAIL_RETURN;
    }
    memcpy(product_key, node->product_key, strlen(node->product_key)+1);
    memcpy(device_name, node->device_name, strlen(node->device_name)+1);

    _alink_subdev_mgr_unlock();

    return SUCCESS_RETURN;
}

int alink_subdev_get_triple_by_devid(uint32_t devid, char *product_key, char *device_name, char *device_secret)
{
    subdev_hash_node_t *node;

    _alink_subdev_mgr_lock();

    node = _subdev_hash_search_by_devid(devid);
    if (node == NULL) {
        _alink_subdev_mgr_unlock();
        return FAIL_RETURN;
    }
    memcpy(product_key, node->product_key, strlen(node->product_key)+1);
    memcpy(device_name, node->device_name, strlen(node->device_name)+1);
    if (node->device_secret) {
        memcpy(device_secret, node->device_secret, strlen(node->device_secret)+1);
    }

    _alink_subdev_mgr_unlock();

    return SUCCESS_RETURN; 
}

int alink_subdev_get_devid_by_pkdn(const char *product_key, const char *device_name, uint32_t *devid)
{
    subdev_hash_node_t *node;

    _alink_subdev_mgr_lock();

    node = _subdev_hash_search_by_pkdn(product_key, device_name);
    if (node == NULL) {
        _alink_subdev_mgr_unlock();
        return FAIL_RETURN;
    }

    *devid = node->devid;

    _alink_subdev_mgr_unlock();

    return SUCCESS_RETURN;
}

int alink_subdev_update_status(uint32_t devid, alink_subdev_status_t status)
{
    subdev_hash_node_t *node;

    _alink_subdev_mgr_lock();

    node = _subdev_hash_search_by_devid(devid);
    if (node == NULL) {
        _alink_subdev_mgr_unlock();
        return FAIL_RETURN;
    }

    node->status = status;

    _alink_subdev_mgr_unlock();

    return SUCCESS_RETURN;    
}

int alink_subdev_update_device_secret(uint32_t devid, const char *device_secret)
{
    subdev_hash_node_t *node;
    char *temp;

    temp = alink_utils_strdup(device_secret, strlen(device_secret));
    if (temp == NULL) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    _alink_subdev_mgr_lock();

    node = _subdev_hash_search_by_devid(devid);
    if (node == NULL) {
        _alink_subdev_mgr_unlock();
        return FAIL_RETURN;
    }

    if (node->device_secret) {
        alink_free(node->device_secret);
    }
    node->device_secret = temp;

    _alink_subdev_mgr_unlock();

    return SUCCESS_RETURN;    
}

int alink_subdev_update_mass_status(uint32_t *subdev_array, uint32_t subdev_num, alink_subdev_status_t status)
{
    int res;
    uint8_t i;

    for (i=0; i<subdev_num; i++) {
        if (subdev_array[i] == 0) {
            break;
        }
        
        alink_debug("devid %d update status", subdev_array[i]);
        res = alink_subdev_update_status(subdev_array[i], status);
        if (res < SUCCESS_RETURN) {
            alink_warning("update mass subdev status error");
            return res;
        }
    }

    return SUCCESS_RETURN;
}

