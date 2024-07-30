/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef EVENT_H
#define EVENT_H

#include "fff.h"

#define EVENT_MASK_MQTT_INIT         0x02
#define EVENT_MASK_MQTT_CONNECTED    0x04

typedef void * EventGroupHandle_t;

extern EventGroupHandle_t xSystemEvents;

DECLARE_FAKE_VOID_FUNC( vWaitUntilNetworkIsUp );

#endif /* EVENT_H */