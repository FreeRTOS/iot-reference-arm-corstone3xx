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

#include "acamera_firmware_config.h"
#include "acamera_isp_config.h"
#include "acamera_logger.h"
#include "system_stdlib.h"

#include "FreeRTOS.h"

void * system_sw_alloc( uint32_t size,
                        void * rtrack_arr )
{
    return pvPortMalloc( size );
}

void system_sw_free( void * ptr )
{
    vPortFree( ptr );
}

int32_t init_sw_io( void )
{
    int32_t result = 0;

    return result;
}

int32_t close_sw_io( void )
{
    int32_t result = 0;

    return result;
}

uint32_t system_sw_read_32( uintptr_t addr )
{
    uint32_t result = 0;

    if( ( void * ) addr != NULL )
    {
        volatile const uint32_t * p_addr = ( volatile uint32_t * ) ( addr );
        result = *p_addr;
    }
    else
    {
        LOG( LOG_ERR, "Failed to read memory from address 0x%x. Base pointer is null ", addr );
    }

    return result;
}

uint16_t system_sw_read_16( uintptr_t addr )
{
    uint16_t result = 0;

    if( ( void * ) addr != NULL )
    {
        volatile const uint16_t * p_addr = ( volatile uint16_t * ) ( addr );
        result = *p_addr;
    }
    else
    {
        LOG( LOG_ERR, "Failed to read memory from address 0x%x. Base pointer is null ", addr );
    }

    return result;
}

uint8_t system_sw_read_8( uintptr_t addr )
{
    uint8_t result = 0;

    if( ( void * ) addr != NULL )
    {
        volatile const uint8_t * p_addr = ( volatile uint8_t * ) ( addr );
        result = *p_addr;
    }
    else
    {
        LOG( LOG_ERR, "Failed to read memory from address 0x%x. Base pointer is null ", addr );
    }

    return result;
}
void system_sw_write_32( uintptr_t addr,
                         uint32_t data )
{
    if( ( void * ) addr != NULL )
    {
        volatile uint32_t * p_addr = ( volatile uint32_t * ) ( addr );
        *p_addr = data;
    }
    else
    {
        LOG( LOG_ERR, "Failed to write %d to memory 0x%x. Base pointer is null ", data, addr );
    }
}

void system_sw_write_16( uintptr_t addr,
                         uint16_t data )
{
    if( ( void * ) addr != NULL )
    {
        volatile uint16_t * p_addr = ( volatile uint16_t * ) ( addr );
        *p_addr = data;
    }
    else
    {
        LOG( LOG_ERR, "Failed to write %d to memory 0x%x. Base pointer is null ", data, addr );
    }
}

void system_sw_write_8( uintptr_t addr,
                        uint8_t data )
{
    if( ( void * ) addr != NULL )
    {
        volatile uint8_t * p_addr = ( volatile uint8_t * ) ( addr );
        *p_addr = data;
    }
    else
    {
        LOG( LOG_ERR, "Failed to write %d to memory 0x%x. Base pointer is null ", data, addr );
    }
}
