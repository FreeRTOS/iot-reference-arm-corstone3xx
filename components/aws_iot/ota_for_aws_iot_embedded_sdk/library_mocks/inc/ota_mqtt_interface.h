/*
 * AWS IoT Over-the-air Update v3.4.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 *
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

#ifndef OTA_MQTT_INTERFACE_H
#define OTA_MQTT_INTERFACE_H

typedef enum OtaMqttStatus
{
    OtaMqttSuccess = 0,
    OtaMqttPublishFailed = 1,
    OtaMqttSubscribeFailed = 2,
    OtaMqttUnsubscribeFailed = 3
} OtaMqttStatus_t;

typedef OtaMqttStatus_t ( * OtaMqttSubscribe_t ) ( const char * pTopicFilter,
                                                   uint16_t topicFilterLength,
                                                   uint8_t ucQoS );
typedef OtaMqttStatus_t ( * OtaMqttUnsubscribe_t )  ( const char * pTopicFilter,
                                                      uint16_t topicFilterLength,
                                                      uint8_t ucQoS );
typedef OtaMqttStatus_t ( * OtaMqttPublish_t )( const char * const pacTopic,
                                                uint16_t usTopicLen,
                                                const char * pcMsg,
                                                uint32_t ulMsgSize,
                                                uint8_t ucQoS );

typedef struct OtaMqttInterface
{
    OtaMqttSubscribe_t subscribe;     /*!< @brief Interface for subscribing to Mqtt topics. */
    OtaMqttUnsubscribe_t unsubscribe; /*!< @brief interface for unsubscribing to MQTT topics. */
    OtaMqttPublish_t publish;         /*!< @brief Interface for publishing MQTT messages. */
} OtaMqttInterface_t;

#endif /* OTA_MQTT_INTERFACE_H */
