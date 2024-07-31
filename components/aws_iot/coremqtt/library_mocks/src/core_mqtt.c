/*
 * coreMQTT v2.1.1
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
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

#include "core_mqtt.h"

DEFINE_FAKE_VALUE_FUNC( const char *,
                        MQTT_Status_strerror,
                        MQTTStatus_t );

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTT_Connect,
                        void *,
                        const MQTTConnectInfo_t *,
                        const MQTTPublishInfo_t *,
                        uint32_t,
                        bool * );

DEFINE_FAKE_VALUE_FUNC( MQTTStatus_t,
                        MQTT_MatchTopic,
                        const char *,
                        const uint16_t,
                        const char *,
                        const uint16_t,
                        bool * );
