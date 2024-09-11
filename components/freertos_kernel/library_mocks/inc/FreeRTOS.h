/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

#include "fff.h"
#include "portmacro.h"

typedef int StaticQueue_t;

/*
 * Definitions found in FreeTOSConfig.h.
 * Because for testing `freertos_command_pool.c` we cannot
 * directly include FreeRTOSConfig.h (this causes build failure),
 * nor can we prototype the configAssert macro from within the file.
 */

#ifndef FREERTOS_CONFIG_H
    #define FREERTOS_CONFIG_H

    DECLARE_FAKE_VOID_FUNC( vAssertCalled,
                            const char *,
                            unsigned long );
    #define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ );

#endif /* FREERTOS_CONFIG_H */

/* This file also contains some definitions usually found in
 * 'FreeRTOSConfig_target.h'.
 * See below. */
#define configTICK_RATE_HZ    ( 1000 )
#define TICKS_TO_pdMS( xTicks )    ( ( uint32_t ) xTicks )

#endif /* INC_FREERTOS_H */
