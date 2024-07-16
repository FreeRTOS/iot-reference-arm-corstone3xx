/*
 * FreeRTOS Transport Secure Sockets V1.0.1
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
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

#ifndef TRANSPORT_INTERFACE_API_H
#define TRANSPORT_INTERFACE_API_H

#include "fff.h"
#include <stdint.h>
#include <stdbool.h>

typedef int NetworkContext_t;
typedef enum TransportStatus
{
    TRANSPORT_STATUS_SUCCESS = 0,
    TRANSPORT_STATUS_CONNECT_FAILURE
} TransportStatus_t;

typedef struct TLSParams
{
    const char * pAlpnProtos;
    bool disableSni;
    size_t maxFragmentLength;
    const char * pRootCa;
    size_t rootCaSize;
    char * pPrivateKeyLabel;
    char * pClientCertLabel;
    char * pLoginPIN;
} TLSParams_t;

typedef struct ServerInfo
{
    const char * pHostName;
    size_t hostNameLength;
    uint16_t port;
} ServerInfo_t;

/* The below two are usually functions, but for */
/* mqtt_agent_task they are not called merely assigned. */
static int Transport_Send = 1;
static int Transport_Recv = 2;

DECLARE_FAKE_VALUE_FUNC( TransportStatus_t,
                         Transport_Connect,
                         NetworkContext_t *,
                         const ServerInfo_t *,
                         const TLSParams_t *,
                         uint32_t,
                         uint32_t );

DECLARE_FAKE_VALUE_FUNC( TransportStatus_t,
                         Transport_Disconnect,
                         NetworkContext_t * );

#endif /* TRANSPORT_INTERFACE_API_H */
