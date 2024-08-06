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

#ifndef OTA_H
#define OTA_H

#include <stdint.h>
#include "fff.h"
#include "ota_private.h"
#include "ota_os_interface.h"
#include "ota_mqtt_interface.h"
#include "ota_platform_interface.h"

typedef struct OtaInterface
{
    OtaOSInterface_t os;
    OtaMqttInterface_t mqtt;
    OtaPalInterface_t pal;
} OtaInterfaces_t;

typedef struct OtaAppBuffer
{
    uint8_t * pUpdateFilePath;
    uint16_t updateFilePathsize;
    uint8_t * pCertFilePath;
    uint16_t certFilePathSize;
    uint8_t * pStreamName;
    uint16_t streamNameSize;
    uint8_t * pDecodeMemory;
    uint32_t decodeMemorySize;
    uint8_t * pFileBitmap;
    uint16_t fileBitmapSize;
    uint8_t * pUrl;
    uint16_t urlSize;
    uint8_t * pAuthScheme;
    uint16_t authSchemeSize;
} OtaAppBuffer_t;

typedef enum OtaJobEvent
{
    OtaJobEventActivate = 0,
    OtaJobEventFail = 1,
    OtaJobEventStartTest = 2,
    OtaJobEventProcessed = 3,
    OtaJobEventSelfTestFailed = 4,
    OtaJobEventParseCustomJob = 5,
    OtaJobEventReceivedJob = 6,
    OtaJobEventUpdateComplete = 7,
    OtaJobEventNoActiveJob = 8,
    OtaLastJobEvent = OtaJobEventStartTest
} OtaJobEvent_t;

typedef enum OtaErr
{
    OtaErrNone = 0,
    OtaErrUninitialized = 1
} OtaErr_t;

typedef enum OtaState
{
    OtaAgentStateNoTransition = 0,
    OtaAgentStateInit = 1,
    OtaAgentStateReady = 2,
    OtaAgentStateRequestingJob = 3,
    OtaAgentStateWaitingForJob = 4,
    OtaAgentStateCreatingFile = 5,
    OtaAgentStateRequestingFileBlock = 6,
    OtaAgentStateWaitingForFileBlock = 7,
    OtaAgentStateClosingFile = 8,
    OtaAgentStateSuspended = 9,
    OtaAgentStateShuttingDown = 10,
    OtaAgentStateStopped = 11,
    OtaAgentStateAll = 12
} OtaState_t;

typedef void (* OtaAppCallback_t)( OtaJobEvent_t eEvent,
                                   void * pData );

DECLARE_FAKE_VALUE_FUNC( OtaErr_t, OTA_ActivateNewImage );
DECLARE_FAKE_VALUE_FUNC( OtaState_t, OTA_GetState );
DECLARE_FAKE_VALUE_FUNC( OtaState_t,
                         OTA_Shutdown,
                         uint32_t,
                         uint8_t );
DECLARE_FAKE_VALUE_FUNC( OtaErr_t,
                         OTA_SetImageState,
                         OtaImageState_t );
DECLARE_FAKE_VALUE_FUNC( bool,
                         OTA_SignalEvent,
                         const OtaEventMsg_t * const );
DECLARE_FAKE_VALUE_FUNC( OtaErr_t,
                         OTA_Suspend );
DECLARE_FAKE_VALUE_FUNC( OtaErr_t,
                         OTA_Resume );
DECLARE_FAKE_VALUE_FUNC( OtaErr_t,
                         OTA_Init,
                         const OtaAppBuffer_t *,
                         const OtaInterfaces_t *,
                         const uint8_t *,
                         OtaAppCallback_t );
DECLARE_FAKE_VALUE_FUNC( OtaErr_t,
                         OTA_GetStatistics,
                         OtaAgentStatistics_t * );
DECLARE_FAKE_VOID_FUNC( OTA_EventProcessingTask,
                        const void * );

#endif /* OTA_H */
