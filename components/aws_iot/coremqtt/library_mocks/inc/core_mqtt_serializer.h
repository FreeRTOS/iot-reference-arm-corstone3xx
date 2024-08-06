/*
 * coreMQTT v2.1.1
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CORE_MQTT_SERIALIZER_H
#define CORE_MQTT_SERIALIZER_H

#include <stdbool.h>
#include <stddef.h>

typedef enum MQTTStatus
{
    MQTTSuccess = 0,
    MQTTBadParameter,
    MQTTSendFailed,
    MQTTRecvFailed
} MQTTStatus_t;

typedef struct MQTTConnectInfo
{
    bool cleanSession;
    uint16_t keepAliveSeconds;
    const char * pClientIdentifier;
    uint16_t clientIdentifierLength;
    const char * pUserName;
    uint16_t userNameLength;
    const char * pPassword;
    uint16_t passwordLength;
} MQTTConnectInfo_t;

typedef enum MQTTQoS
{
    MQTTQoS0 = 0,
    MQTTQoS1 = 1,
    MQTTQoS2 = 2
} MQTTQoS_t;

typedef struct MQTTPublishInfo
{
    MQTTQoS_t qos;
    bool retain;
    bool dup;
    const char * pTopicName;
    uint16_t topicNameLength;
    const void * pPayload;
    size_t payloadLength;
} MQTTPublishInfo_t;

typedef struct MQTTSubscribeInfo
{
    MQTTQoS_t qos;
    const char * pTopicFilter;
    uint16_t topicFilterLength;
} MQTTSubscribeInfo_t;

typedef struct MQTTFixedBuffer
{
    uint8_t * pBuffer;
    size_t size;
} MQTTFixedBuffer_t;

#endif /* ifndef CORE_MQTT_SERIALIZER_H */
