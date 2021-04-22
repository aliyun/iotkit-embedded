/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __IMPORT_PRODUCT_H__
#define __IMPORT_PRODUCT_H__

#define MAC_ADDR_LEN_MAX            (10)
#define PID_STRLEN_MAX              (64)
#define MID_STRLEN_MAX              (64)
#define IOTX_URI_MAX_LEN            (135)  /* IoTx CoAP/HTTP uri & MQTT topic maximal length */
#define PID_STR_MAXLEN              (64)
#define MID_STR_MAXLEN              (64)
#define PRODUCT_KEY_MAXLEN          (20 + 1)
#define DEVICE_NAME_MAXLEN          (32 + 1)
#define DEVICE_ID_MAXLEN            (64 + 1)
#define DEVICE_SECRET_MAXLEN        (64 + 1)
#define PRODUCT_SECRET_MAXLEN       (64 + 1)
#define FIRMWARE_VERSION_MAXLEN     (64 + 1)
#define HAL_CID_LEN                 (64 + 1)

/**
 * @brief   获取设备的固件版本字符串
 *
 * @param   version : 用来存放版本字符串的数组
 * @return  写到version[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_GetFirmwareVersion(_OU_ char version[FIRMWARE_VERSION_MAXLEN]);

/**
 * @brief   获取设备的`Partner ID`, 仅用于紧密合作伙伴
 *
 * @param   pid_str : 用来存放Partner ID字符串的数组
 * @return  写到pid_str[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_GetPartnerID(_OU_ char pid_str[PID_STR_MAXLEN]);

/**
 * @brief   获取设备的`Module ID`, 仅用于紧密合作伙伴
 *
 * @param   mid_str : 用来存放Module ID字符串的数组
 * @return  写到mid_str[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_GetModuleID(_OU_ char mid_str[MID_STR_MAXLEN]);


/**
 * @brief   获取设备的`DeviceID`, 用于标识设备单品的ID
 *
 * @param   device_id : 用来存放DeviceID字符串的数组
 * @return  写到device_id[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_GetDeviceID(_OU_ char device_id[DEVICE_ID_LEN]);

/**
 * @brief   获取唯一的芯片ID字符串
 *
 * @param   cid_str : 存放芯片ID字符串的缓冲区数组
 * @return  指向缓冲区数组的起始地址
 */
char *HAL_GetChipID(_OU_ char cid_str[HAL_CID_LEN]);

/**
 * @brief   设置设备的`ProductKey`, 用于标识设备的品类, 三元组之一
 *
 * @param   product_key : 用来存放ProductKey字符串的数组
 * @return  写到product_key[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_SetProductKey(_IN_ char *product_key);

/**
 * @brief   设置设备的`DeviceName`, 用于标识设备单品的名字, 三元组之一
 *
 * @param   device_name : 用来存放DeviceName字符串的数组
 * @return  写到device_name[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_SetDeviceName(_IN_ char *device_name);

/**
 * @brief   设置设备的`DeviceSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   device_secret : 用来存放DeviceSecret字符串的数组
 * @return  写到device_secret[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_SetDeviceSecret(_IN_ char *device_secret);

/**
 * @brief   设置设备的`ProductSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   product_secret : 用来存放ProductSecret字符串的数组
 * @return  写到product_secret[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_SetProductSecret(_IN_ char *product_secret);

/**
 * @brief   获取设备的`ProductKey`, 用于标识设备的品类, 三元组之一
 *
 * @param   product_key : 用来存放ProductKey字符串的数组
 * @return  写到product_key[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_GetProductKey(_OU_ char product_key[PRODUCT_KEY_LEN]);

/**
 * @brief   获取设备的`DeviceName`, 用于标识设备单品的名字, 三元组之一
 *
 * @param   device_name : 用来存放DeviceName字符串的数组
 * @return  写到device_name[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_GetDeviceName(_OU_ char device_name[DEVICE_NAME_LEN]);

/**
 * @brief   获取设备的`DeviceSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   device_secret : 用来存放DeviceSecret字符串的数组
 * @return  写到device_secret[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_GetDeviceSecret(_OU_ char device_secret[DEVICE_SECRET_LEN]);

/**
 * @brief   获取设备的`ProductSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   product_secret : 用来存放ProductSecret字符串的数组
 * @return  写到product_secret[]数组中的字符长度, 单位是字节(Byte)
 */
DLL_HAL_API int HAL_GetProductSecret(_OU_ char product_secret[DEVICE_SECRET_LEN]);

#define NIF_STRLEN_MAX (160)

/**
 ** @brief Retrieves all the info of the current network interfaces,
 ** including eth Mac, WiFi Mac, and IMEI/ICCID/IMSI/MSISDN for cellular connections.
 ** Note that the network interface length MUST NOT exceed NIF_STRLEN_MAX
 **
 ** @param [nif_str] give buffer to save network interface
 ** @return the lenth of the nif_str info
 ** @see None.
 ** @note None.
 **/
DLL_HAL_API int HAL_GetNetifInfo(char *nif_str);

/**
 * 这个Hal主要用于解决三元组烧重的问题. 如果不涉及这个问题, 则请忽略这个Hal.
 *
 * @breif 获取设备的唯一标识符(uuid)
 * 这里的uuid, 主要用于在多个设备都烧了相同三元组情况下能够区分不同设备,不要求全球唯一,但要求对同一个设备始终保持不变
 * 用户可以从IMEI/mac地址/cpu序列号等信息中择一作为设备的uuid
 *
 * 考虑到部分设备会用mac地址作为uuid, 而少量设备的mac地址会偶现无法读取成功的情况, 或者每次读出来都不一样的情况,
 * 首选的方案为步骤3, 即将uuid+time的信息持久化到一片恢复出厂设置/固件升级也不会被erase掉的存储器件上
 *
 * 如果步骤3无法实施, 但是设备的uuid的确每次都能读到, 而且每次读出来都一样,
 * 则可优先用步骤4.a, 即将uuid的信息持久化到flash中, 每次开机优先读这片flash
 * 如果4.a无法实现, 则可用4.b, 即每次直接从器件中读取出设备的uuid(器件可能不稳定,不推荐)
 *
 * 如果步骤3/4都无法实施, 则返回-1作为错误码
 *
 * @param[in] buf 缓存的buf, 用以存储设备的唯一标识符
 * @param[in] len 缓存的最大长度. 默认是256 Byte
 *
 * @return int
 * @retval <= 0 没有获取到uuid
 * @retval > 0 返回的字节数
 **/
DLL_HAL_API  int HAL_GetUUID(uint8_t *buf, int len);

#endif  /* __IMPORT_PRODUCT_H__ */
