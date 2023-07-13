/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef EVENT_HELPER_H
#define EVENT_HELPER_H

#include "event_groups.h"

#define EVENT_MASK_MQTT_INIT         0x01
#define EVENT_MASK_MQTT_CONNECTED    0x02

extern EventGroupHandle_t xSystemEvents;

#endif /* EVENT_HELPER_H */
