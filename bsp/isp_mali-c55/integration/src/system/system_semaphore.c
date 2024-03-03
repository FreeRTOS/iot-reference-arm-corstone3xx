/*
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 * - Neither the name of ARM nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "system_semaphore.h"
#include "acamera_logger.h"
#include "acamera_types.h"
#include "system_semaphore_platform.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define SEMAPHORE_DEFAULT_COUNTING_LIMIT    100

int32_t system_semaphore_init( semaphore_t * sem )
{
    return system_semaphore_init_counting( sem, SEMAPHORE_DEFAULT_COUNTING_LIMIT, 0 );
}

int32_t system_semaphore_init_counting( semaphore_t * sem,
                                        uint32_t max_count,
                                        uint32_t initial_count )
{
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateCounting( max_count, initial_count );

    if( xSemaphore == NULL )
    {
        return -1;
    }

    *sem = xSemaphore;
    return 0;
}

int32_t system_semaphore_raise( semaphore_t sem )
{
    LOG( LOG_DEBUG, "RAISE 0x%x", ( uint32_t ) sem );

    if( xSemaphoreGive( ( SemaphoreHandle_t ) sem ) != pdTRUE )
    {
        LOG( LOG_DEBUG, "    FAIL" );
        return -1;
    }

    return 0;
}

int32_t system_semaphore_wait( semaphore_t sem,
                               uint32_t timeout_ms )
{
    LOG( LOG_DEBUG, "WAIT 0x%x %dms", ( uint32_t ) sem, timeout_ms );

    if( xSemaphoreTake( ( SemaphoreHandle_t ) sem, pdMS_TO_TICKS( timeout_ms ) ) != pdTRUE )
    {
        LOG( LOG_DEBUG, "    FAIL" );
        return -1;
    }

    return 0;
}

int32_t system_semaphore_destroy( semaphore_t sem )
{
    vSemaphoreDelete( ( SemaphoreHandle_t ) sem );
    return 0;
}
