# README.md: mqtt_impl

## Contents

```shell
.
├── iotx_mqtt_client.c
├── iotx_mqtt_client.h
├── iotx_mqtt_config.h
├── MQTTConnectClient.c
├── MQTTConnect.h
├── MQTTDeserializePublish.c
├── mqtt_internal.h
├── MQTTPacket.c
├── MQTTPacket.h
├── MQTTPublish.h
├── MQTTSerializePublish.c
├── MQTTSubscribeClient.c
├── MQTTSubscribe.h
├── MQTTUnsubscribeClient.c
├── MQTTUnsubscribe.h
└── README.md
```

## Introduction

 Implementation of a deeply customized MQTT client 3.1.1.
 Do not compile or include this folder if other MQTT implementations are used！

### Features

- **Common**.  Fully support for MQTT Client 3.1.1.
- **Low memory footprint**. Minimum memory requirements < 1KB
- **Quick subsciribe**. subscribe effective immediately. 
- **Compressed topic**. Use md5/sha256 to Compress topic string to reduce ram usage.

### Dependencies

- **wrapper**. osal and hal to shield different os and hardware
- **infra**. Authentication, net and so on tool set.



