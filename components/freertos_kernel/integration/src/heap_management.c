/* Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

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

/* These are dummy implementations as C standard library does not provide
 * functions to get the statistics of heap memory. These dummy implementation are needed
 * as these APIs are used as part of FreeRTOS Plus TCP code which is unused in the FRI code (removed by the linker)
 * but ARMClang linker requires all the compiled symbols to be defined.
 */
size_t xPortGetFreeHeapSize( void )
{
    return 0;
}

size_t xPortGetMinimumEverFreeHeapSize( void )
{
    return 0;
}
