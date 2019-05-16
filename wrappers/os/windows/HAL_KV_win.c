/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */



#if defined(HAL_KV)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include "infra_defs.h"
#include "infra_config.h"

#define  HAL_DEBUG_OUT 1
/*The content of the file can't get lost after reboot, please ensure the file is not stored on a RAM file system*/
static char *kvFileName = "c:\\iotKv.data";

typedef struct {
    int keyLen;
    int valLen;
    char *pKey;
    char *pValue;
} kvHeader_t;

typedef struct kvNode_s {
    struct kvNode_s *pNext;
    kvHeader_t item;
} kvNode_t;

static int kv_read_header(HANDLE hFind, kvHeader_t *pHeader)
{
    DWORD readLen;

    if (ReadFile(hFind, pHeader, 2 * sizeof(int), &readLen, NULL) == 0) {
        return (0);
    }
    return (readLen);
}

static int kv_read_item(HANDLE hFind, kvHeader_t *pHeader)
{
    DWORD r1, total = 0;

    ReadFile(hFind, pHeader, 2 * sizeof(int), &r1, NULL);
    total += r1;
    pHeader->pKey = malloc(pHeader->keyLen);
    pHeader->pValue = malloc(pHeader->valLen);
    ReadFile(hFind, pHeader->pKey, pHeader->keyLen, &r1, NULL);
    total += r1;
    ReadFile(hFind, pHeader->pValue, pHeader->valLen, &r1, NULL);
    total += r1;

    return (total);
}

static void kv_write_item(HANDLE hFile, kvHeader_t *pHeader)
{
    DWORD w1;

    if (WriteFile(hFile, pHeader, 2 * sizeof(int), &w1, NULL) == 0) {
        printf("KV write error %d\n\r", (int)GetLastError());
    }
    WriteFile(hFile, pHeader->pKey, pHeader->keyLen, &w1, NULL);
    WriteFile(hFile, pHeader->pValue, pHeader->valLen, &w1, NULL);
}

int HAL_Kv_Get(const char *key, void *val, int *buffer_len)
{
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    DWORD readLen = 0, r1;
    kvHeader_t header;
    char keyInFile[32];
    int ret = -1;

    hFind = FindFirstFile(kvFileName, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return (-1);
    }
    FindClose(hFind);
    hFind = CreateFileA(kvFileName, GENERIC_READ,
                        0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    while (readLen < FindFileData.nFileSizeLow) {
        readLen += kv_read_header(hFind, &header);
        ReadFile(hFind, keyInFile, header.keyLen, &r1, NULL);
        readLen += r1;
        keyInFile[header.keyLen] = 0;
        if (strncmp(keyInFile, key, header.keyLen) == 0) {
            if ((*buffer_len) < header.valLen) {
                printf("HAL_Kv_Get(key): length of buffer is not long enough\n\r");
                break;
            }
            ReadFile(hFind, val, header.valLen, &r1, NULL);
            readLen += r1;
            *buffer_len = header.valLen;
            ret = 0;
            break;
        } else {
            readLen += header.valLen;
            SetFilePointer(hFind, readLen, NULL, FILE_BEGIN);
        }
    }

    CloseHandle(hFind);
    return (ret);
}


int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    DWORD readLen = 0;
    kvNode_t *pRoot = NULL, *pNode, *pPrevNode = NULL;
    int found = 0;

    if (HAL_DEBUG_OUT) {
        printf("HAL_Kv_Set %s\n\r", key);
    }
    hFind = FindFirstFile(kvFileName, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        hFind = CreateFileA(kvFileName, GENERIC_READ | GENERIC_WRITE,
                            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFind == INVALID_HANDLE_VALUE) {
            return (-1);
        }
        FindFileData.nFileSizeLow = 0;
    } else {
        FindClose(hFind);
        hFind = CreateFileA(kvFileName, GENERIC_READ | GENERIC_WRITE,
                            0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    while (readLen < FindFileData.nFileSizeLow) {
        pNode = (kvNode_t *)malloc(sizeof(kvNode_t));
        if (pRoot == NULL) {
            pRoot = pNode;
        }
        if (pPrevNode) {
            pPrevNode->pNext = pNode;
        }
        pNode->pNext = NULL;
        readLen += kv_read_item(hFind, &pNode->item);
        if (strcmp(key, pNode->item.pKey) == 0) {
            found = 1;
            free(pNode->item.pValue);
            pNode->item.pValue = malloc(len);
            memcpy(pNode->item.pValue, val, len);
            pNode->item.valLen = len;
        }
        pPrevNode = pNode;
    }

    if (found == 0) {
        pNode = (kvNode_t *)malloc(sizeof(kvNode_t));
        if (pRoot == NULL) {
            pRoot = pNode;
        }
        if (pPrevNode) {
            pPrevNode->pNext = pNode;
        }
        pNode->pNext = NULL;
        pNode->item.keyLen = strlen(key) + 1;
        pNode->item.pKey = malloc(pNode->item.keyLen);
        pNode->item.pValue = malloc(len);
        pNode->item.valLen = len;
        strcpy(pNode->item.pKey, key);
        memcpy(pNode->item.pValue, val, len);
    }

    pNode = pRoot;
    SetFilePointer(hFind, 0, NULL, FILE_BEGIN);
    SetEndOfFile(hFind);
    while (pNode) {
        kv_write_item(hFind, &pNode->item);
        pNode = pNode->pNext;
    }

    CloseHandle(hFind);
    return (int)0;
}

int HAL_Kv_Del(const char *key)
{
    return 0;
}

#endif  /* #if defined(HAL_KV) */



