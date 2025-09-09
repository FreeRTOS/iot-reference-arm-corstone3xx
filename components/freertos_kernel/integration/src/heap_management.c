/* Copyright 2024-2025 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "heap_management.h"
#include <stdlib.h>

void * pvPortMalloc( size_t xWantedSize )
{
    return malloc( xWantedSize );
}

void vPortFree( void * pv )
{
    free( pv );
}

void * pvPortCalloc( size_t xNum,
                     size_t xSize )
{
    return calloc( xNum, xSize );
}

size_t xPortGetFreeHeapSize( void )
{
    return 0;
}

size_t xPortGetMinimumEverFreeHeapSize( void )
{
    return 0;
}
