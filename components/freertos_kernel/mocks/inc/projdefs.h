/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef PROJDEFS_H
#define PROJDEFS_H

#include "portmacro.h"

typedef void (* TaskFunction_t)( void * );
#define pdFALSE    ( ( BaseType_t ) 0 )
#define pdTRUE     ( ( BaseType_t ) 1 )

#define pdPASS     ( pdTRUE )
#define pdFAIL     ( pdFALSE )

#ifndef pdMS_TO_TICKS
    #define pdMS_TO_TICKS( xTimeInMs )    ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000U ) )
#endif

#endif // PROJDEFS_H
