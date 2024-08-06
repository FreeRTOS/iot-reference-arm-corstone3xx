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

#include "core_mqtt_agent.h"

#include "fff.h"

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTTAgent_CommandLoop,
                        MQTTAgentContext_t * );

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTTAgent_ResumeSession,
                        MQTTAgentContext_t *,
                        bool );
DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTTAgent_Disconnect,
                        const MQTTAgentContext_t *,
                        const MQTTAgentCommandInfo_t * );

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTTAgent_Init,
                        MQTTAgentContext_t *,
                        const MQTTAgentMessageInterface_t *,
                        const MQTTFixedBuffer_t *,
                        const TransportInterface_t *,
                        MQTTGetCurrentTimeFunc_t,
                        MQTTAgentIncomingPublishCallback_t,
                        int * )

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTTAgent_Subscribe,
                        const MQTTAgentContext_t *,
                        MQTTAgentSubscribeArgs_t *,
                        const MQTTAgentCommandInfo_t * );

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTTAgent_CancelAll,
                        MQTTAgentContext_t * );

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTTAgent_Publish,
                        const MQTTAgentContext_t *,
                        MQTTPublishInfo_t *,
                        const MQTTAgentCommandInfo_t * );

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTTAgent_Unsubscribe,
                        const MQTTAgentContext_t *,
                        MQTTAgentSubscribeArgs_t *,
                        const MQTTAgentCommandInfo_t * );
