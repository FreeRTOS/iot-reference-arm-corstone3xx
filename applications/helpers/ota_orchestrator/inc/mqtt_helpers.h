/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef MQTT_HELPERS_H
#define MQTT_HELPERS_H

/* Standard includes. */
#include <stdlib.h>
#include <stdint.h>

/**
 * @ingroup ota_enum_types
 * @brief The OTA MQTT interface return status.
 */
typedef enum OtaMqttStatus
{
    OtaMqttSuccess = 0,          /*!< @brief OTA MQTT interface success. */
    OtaMqttPublishFailed = 0xa0, /*!< @brief Attempt to publish a MQTT message failed. */
    OtaMqttSubscribeFailed,      /*!< @brief Failed to subscribe to a topic. */
    OtaMqttUnsubscribeFailed     /*!< @brief Failed to unsubscribe from a topic. */
} OtaMqttStatus_t;

/**
 * @brief Subscribe to a specified topic.
 *
 * Precondition: pTopicFilter is not null.
 *
 * @param[in] pTopicFilter The topic filter to subscribe to.
 * @param[in] topicFilterLength Length of the topic filter.
 * @param[in] ucQoS Quality of Service (QoS), the level of reliability for
 * message delivery. Can be 0, 1 or 2.
 *
 * @return OtaMqttSuccess if the subscribe completed successfully, otherwise
 * OtaMqttSubscribeFailed.
 */
OtaMqttStatus_t prvMQTTSubscribe( const char * pTopicFilter,
                                  uint16_t topicFilterLength,
                                  uint8_t ucQoS );

/**
 * @brief Publish a message to a specified topic.
 * *
 * @param[in] pacTopic The topic that the message should be published to.
 * @param[in] topicLen Length of the topic.
 * @param[in] pMsg The message to be published.
 * @param[in] msgSize Size of the message.
 * @param[in] ucQoS Quality of Service (QoS), the level of reliability for
 * message delivery. Can be 0, 1 or 2.
 *
 * @return OtaMqttSuccess if the subscribe completed successfully, otherwise
 * OtaMqttSubscribeFailed.
 */
OtaMqttStatus_t prvMQTTPublish( const char * const pacTopic,
                                uint16_t topicLen,
                                const char * pMsg,
                                uint32_t msgSize,
                                uint8_t ucQoS );

/**
 * @brief Unsubscribe from a specified topic.
 *
 * Precondition: pTopicFilter is not null.
 *
 * @param[in] pTopicFilter The topic filter to unsubscribe from.
 * @param[in] topicFilterLength Length of the topic filter.
 * @param[in] ucQoS Quality of Service (QoS), the level of reliability for
 * message delivery. Can be 0, 1 or 2.
 *
 * @return OtaMqttSuccess if the unsubscribe completed successfully, otherwise
 * OtaMqttSubscribeFailed.
 */
OtaMqttStatus_t prvMQTTUnsubscribe( const char * pTopicFilter,
                                    uint16_t topicFilterLength,
                                    uint8_t ucQoS );

#endif /* MQTT_HELPERS_H */
