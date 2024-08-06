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

#ifndef OTA_OS_INTERFACE_H
#define OTA_OS_INTERFACE_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/* Dummy enums added for mocking. Would normally be functions. */
typedef enum
{
    OtaInitEvent_FreeRTOS = 1,
} OtaInitEvent_t;
typedef enum
{
    OtaSendEvent_FreeRTOS = 1,
} OtaSendEvent_t;
typedef enum
{
    OtaReceiveEvent_FreeRTOS = 1,
} OtaReceiveEvent_t;
typedef enum
{
    OtaDeinitEvent_FreeRTOS = 1,
} OtaDeinitEvent_t;

typedef enum
{
    OtaStartTimer_FreeRTOS = 1,
} OtaStartTimer_t;
typedef enum
{
    OtaStopTimer_FreeRTOS = 1,
} OtaStopTimer_t;
typedef enum
{
    OtaDeleteTimer_FreeRTOS = 1,
} OtaDeleteTimer_t;

typedef enum
{
    Malloc_FreeRTOS = 1,
} OtaMalloc_t;
typedef enum
{
    Free_FreeRTOS = 1,
} OtaFree_t;

typedef struct OtaEventInterface
{
    OtaInitEvent_t init;
    OtaSendEvent_t send;
    OtaReceiveEvent_t recv;
    OtaDeinitEvent_t deinit;
} OtaEventInterface_t;

typedef struct OtaTimerInterface
{
    OtaStartTimer_t start;
    OtaStopTimer_t stop;
    OtaDeleteTimer_t delete;
} OtaTimerInterface_t;

typedef struct OtaMallocInterface
{
    OtaMalloc_t malloc;
    OtaFree_t free;
} OtaMallocInterface_t;


typedef struct OtaOSInterface
{
    OtaEventInterface_t event;
    OtaTimerInterface_t timer;
    OtaMallocInterface_t mem;
} OtaOSInterface_t;


/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* ifndef OTA_OS_INTERFACE_H */
