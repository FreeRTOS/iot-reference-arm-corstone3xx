/*
 * AWS IoT Over-the-air Update v3.4.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 *
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

#ifndef OTA_PRIVATE_H
#define OTA_PRIVATE_H

#include <stdbool.h>
#include <stdint.h>

#define OTA_MAX_BLOCK_BITMAP_SIZE    128U

typedef struct OtaAgentStatistics
{
    uint32_t otaPacketsReceived;
    uint32_t otaPacketsQueued;
    uint32_t otaPacketsProcessed;
    uint32_t otaPacketsDropped;
} OtaAgentStatistics_t;

typedef enum OtaImageState
{
    OtaImageStateUnknown = 0,
    OtaImageStateTesting = 1,
    OtaImageStateAccepted = 2,
    OtaImageStateRejected = 3,
    OtaImageStateAborted = 4,
    OtaLastImageState = OtaImageStateAborted
} OtaImageState_t;

typedef enum OtaEvent
{
    OtaAgentEventStart = 0,
    OtaAgentEventReceivedJobDocument,
    OtaAgentEventReceivedFileBlock
} OtaEvent_t;

typedef struct OtaEventData
{
    uint8_t data[ 10 ];
    uint32_t dataLength;
    bool bufferUsed;
} OtaEventData_t;

typedef struct OtaEventMsg
{
    OtaEventData_t * pEventData;
    OtaEvent_t eventId;
} OtaEventMsg_t;

#endif /* OTA_PRIVATE_H */
