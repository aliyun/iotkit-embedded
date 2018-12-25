/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"

static uint8_t _uri_to_hash(const char *uri, uint8_t uri_len)
{
    uint8_t i;
    uint32_t sum = 0;

    for (i = 0; i < uri_len; i++)
    {
        sum += uri[i];
    }

    for (; i < ALINK_URI_MAX_LEN; i++)
    {
        sum++;
    }

    sum += uri_len;
    sum = sum % HASH_TABLE_SIZE_MAX;

    return sum;
}

static int _uri_hash_insert(const alink_uri_handle_pair_t *pair, uri_hash_table_t *table) 
{
    uint8_t hash = _uri_to_hash(pair->uri_string, strlen(pair->uri_string));
    uri_hash_node_t *node, *search_node;

    node = HAL_Malloc(sizeof(uri_hash_node_t));
    if (node == NULL) {
        return FAIL_RETURN;
    }
    node->pair = pair;
    node->next = NULL;

    if (table[hash] == NULL) {
        table[hash] = node;
    }
    else {
        search_node = table[hash];
        while (search_node->next) {
            search_node = search_node->next;
        }
        search_node->next = node;
    }

    return SUCCESS_RETURN;
}

#if UTILS_HASH_TABLE_ITERATOR_ENABLE
void _uri_hash_iterator(uri_hash_table_t *table)
{
    ALINK_ASSERT_DEBUG(table != NULL);

    uri_hash_node_t *node;
    uint8_t idx;

    for (idx = 0; idx < HASH_TABLE_SIZE_MAX; idx++) {
        if (table[idx] == NULL ) {
            alink_debug("hTable[%d] = NULL", idx);
            continue;
        }
        else {
            node = table[idx];
            alink_debug("hTable[%d] = %s", idx, node->pair->uri_string);

            while (node->next) {
                node = node->next;
                alink_debug("hTable[%d] = %s *", idx, node->pair->uri_string);
            }
        }
    }
}
#endif

int utils_uri_hash_init(const alink_uri_handle_pair_t *uri_handle_pair, uint8_t pair_num, uri_hash_table_t *table)
{
    uint8_t i;
    int res = FAIL_RETURN;

    for (i = 0; i < pair_num; i++) {
        res = _uri_hash_insert(&uri_handle_pair[i], table);
        if (res == FAIL_RETURN) {
            return res;
        }
    }

#if UTILS_HASH_TABLE_ITERATOR_ENABLE
    alink_debug("print hash table");
    _uri_hash_iterator(table);
#endif

    return SUCCESS_RETURN;
}

uri_hash_node_t *utils_uri_hash_search(const char *uri_string, uint8_t uri_len, uri_hash_table_t *table)
{
    uint8_t hash;
    uri_hash_node_t *node;

    hash = _uri_to_hash(uri_string, uri_len);
    node = table[hash];

    while (node) {
        if (uri_len == strlen(node->pair->uri_string) && !memcmp(uri_string, node->pair->uri_string, uri_len)) {
            return node;
        }
        else {
            node = node->next;
        }
    }

    return NULL;
}

void utils_uri_hash_destroy(uri_hash_table_t *table)
{
    ALINK_ASSERT_DEBUG(table != NULL);

    uri_hash_node_t *node, *temp;
    uint8_t idx;

    for (idx = 0; idx < HASH_TABLE_SIZE_MAX; idx++) {
        if (table[idx] == NULL) {
            continue;
        }

        node = table[idx];
        table[idx] = NULL;
        temp = node->next;
        HAL_Free(node);

        while (temp) {
            node = temp;
            temp = temp->next;
            HAL_Free(node);
        }
    }
}
