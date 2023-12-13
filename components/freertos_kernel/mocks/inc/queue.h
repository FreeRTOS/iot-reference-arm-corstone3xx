/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef QUEUE_H
    #define QUEUE_H

    #ifdef __cplusplus
    extern "C" {
    #endif

    #include "fff.h"
    #include "portmacro.h"
    #include "projdefs.h"

    struct QueueDefinition
    {
        int dummy;
    };
    typedef struct QueueDefinition * QueueHandle_t;

    DECLARE_FAKE_VALUE_FUNC( BaseType_t, xQueueSendToBack, QueueHandle_t, const void *, TickType_t );
    DECLARE_FAKE_VALUE_FUNC( BaseType_t, xQueueReceive, QueueHandle_t, void *, TickType_t );

    #ifdef __cplusplus
    }
    #endif

#endif // QUEUE_H
