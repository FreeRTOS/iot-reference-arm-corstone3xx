/*
 * backoffAlgorithm v1.3.0
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

#ifndef BACKOFF_ALGORITHM_H_
#define BACKOFF_ALGORITHM_H_

#include "fff.h"
#include <stdint.h>

#define BACKOFF_ALGORITHM_RETRY_FOREVER    ( UINT32_MAX )

typedef enum BackoffAlgorithmStatus
{
    BackoffAlgorithmSuccess = 0,
    BackoffAlgorithmRetriesExhausted
} BackoffAlgorithmStatus_t;

typedef struct BackoffAlgorithmContext
{
    int dummy;
} BackoffAlgorithmContext_t;

DECLARE_FAKE_VALUE_FUNC( BackoffAlgorithmStatus_t,
                         BackoffAlgorithm_GetNextBackoff,
                         BackoffAlgorithmContext_t *,
                         uint32_t,
                         uint16_t * );


DECLARE_FAKE_VOID_FUNC( BackoffAlgorithm_InitializeParams,
                        BackoffAlgorithmContext_t *,
                        uint16_t,
                        uint16_t,
                        uint32_t );

#endif /* ifndef BACKOFF_ALGORITHM_H_ */
