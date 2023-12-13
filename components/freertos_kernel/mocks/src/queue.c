/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "queue.h"
#include "fff.h"

DEFINE_FAKE_VALUE_FUNC( BaseType_t, xQueueSendToBack, QueueHandle_t, const void *, TickType_t );
DEFINE_FAKE_VALUE_FUNC( BaseType_t, xQueueReceive, QueueHandle_t, void *, TickType_t );
