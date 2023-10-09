/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef EVENT_H
#define EVENT_H

#include <stdbool.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "event_groups.h"

#define EVENT_MASK_NETWORK_UP        0x01
#define EVENT_MASK_MQTT_INIT         0x02
#define EVENT_MASK_MQTT_CONNECTED    0x04

extern EventGroupHandle_t xSystemEvents;

/**
 * @brief Create a system events group.
 */
void vEventHelperInit( void );

/**
 * @brief Wait until network is up and running.
 */
void vWaitUntilNetworkIsUp( void );

/**
 * @brief Wait until MQTT agent is initialised.
 */
void vWaitUntilMQTTAgentReady( void );

/**
 * @brief Wait until MQTT agent is connected to an MQTT broker.
 */
void vWaitUntilMQTTAgentConnected( void );

/**
 * @brief Is MQTT agent connected to an MQTT broker.
 * @return  true: MQTT agent connected to an MQTT broker
 *          false: MQTT agent is not connected to an MQTT broker
 */
bool xIsMqttAgentConnected( void );

#endif /* EVENT_H */
