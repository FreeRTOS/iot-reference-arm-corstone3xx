/*
 * coreMQTT Agent v1.2.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 */

#ifndef CORE_MQTT_AGENT_MESSAGE_INTERFACE_H
#define CORE_MQTT_AGENT_MESSAGE_INTERFACE_H

#include "fff.h"

#include <stdbool.h>
#include <stdint.h>

/* Note: do not define MQTTAgentMessageContext explicitly.
 * It is sometimes user-defined. E.g. a .h file outside of this coremqtt_agent directory
 * will define MQTTAgentMessageContext, then include this header file.
 */

typedef struct MQTTAgentMessageContext   MQTTAgentMessageContext_t;
typedef struct MQTTAgentCommand          MQTTAgentCommand_t;

typedef bool ( * MQTTAgentMessageSend_t )( MQTTAgentMessageContext_t * pMsgCtx,
                                           MQTTAgentCommand_t * const * pCommandToSend,
                                           uint32_t blockTimeMs );

typedef bool ( * MQTTAgentMessageRecv_t )( MQTTAgentMessageContext_t * pMsgCtx,
                                           MQTTAgentCommand_t ** pReceivedCommand,
                                           uint32_t blockTimeMs );

typedef MQTTAgentCommand_t * ( * MQTTAgentCommandGet_t )( uint32_t blockTimeMs );
typedef bool ( * MQTTAgentCommandRelease_t )( MQTTAgentCommand_t * pCommandToRelease );

typedef struct MQTTAgentMessageInterface
{
    MQTTAgentMessageContext_t * pMsgCtx;
    MQTTAgentMessageSend_t send;
    MQTTAgentMessageRecv_t recv;
    MQTTAgentCommandGet_t getCommand;
    MQTTAgentCommandRelease_t releaseCommand;
} MQTTAgentMessageInterface_t;

#endif /* CORE_MQTT_AGENT_MESSAGE_INTERFACE_H */
