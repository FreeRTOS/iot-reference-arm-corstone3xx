/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "events.h"

EventGroupHandle_t xSystemEvents;

DEFINE_FAKE_VOID_FUNC( vWaitUntilNetworkIsUp );
