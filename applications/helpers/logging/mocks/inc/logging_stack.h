/*
 * FreeRTOS Common V1.1.3
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
 */

#ifndef LOGGING_STACK_H_
#define LOGGING_STACK_H_

#include "fff.h"

DECLARE_FAKE_VOID_FUNC_VARARG( SdkLogError,
                               const char *,
                               ... );
#define LogError( message )    ( SdkLogError message )
/* The macro suppresses errors for now. */
DECLARE_FAKE_VOID_FUNC_VARARG( SdkLogWarn,
                               const char *,
                               ... );
#define LogWarn( message )    ( SdkLogWarn message )
DECLARE_FAKE_VOID_FUNC_VARARG( SdkLogInfo,
                               const char *,
                               ... );
#define LogInfo( message )    ( SdkLogInfo message )
DECLARE_FAKE_VOID_FUNC_VARARG( SdkLogDebug,
                               const char *,
                               ... );
#define LogDebug( message )    ( SdkLogDebug message )

#endif /* ifndef LOGGING_STACK_H_ */
