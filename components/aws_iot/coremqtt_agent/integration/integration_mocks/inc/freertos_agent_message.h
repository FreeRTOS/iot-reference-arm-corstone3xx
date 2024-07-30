/*
 * FreeRTOS V202104.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
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
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_AGENT_MESSAGE_H
#define FREERTOS_AGENT_MESSAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "core_mqtt_agent_message_interface.h"
#include "fff.h"
#include "queue.h"

struct MQTTAgentMessageContext
{
    QueueHandle_t queue;
};

DECLARE_FAKE_VALUE_FUNC( bool,
                         Agent_MessageSend,
                         MQTTAgentMessageContext_t *,
                         MQTTAgentCommand_t * const *,
                         uint32_t );
DECLARE_FAKE_VALUE_FUNC( bool,
                         Agent_MessageReceive,
                         MQTTAgentMessageContext_t *,
                         MQTTAgentCommand_t **,
                         uint32_t );

#endif /* FREERTOS_AGENT_MESSAGE_H */