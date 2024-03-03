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
#include "acamera_types.h"

#define SYSTEM_ISP_HW_ADDR_BASE    ( uint32_t ) ISP_SOC_START_ADDR

int32_t pcie_init_isp_io()
{
    return -1;
}

int32_t pcie_close_isp_io()
{
    return -1;
}

uint32_t system_isp_read_32( uint32_t addr )
{
    if( addr < ACAMERA_ISP_MAX_ADDR )
    {
        return *( ( uint32_t * ) ( SYSTEM_ISP_HW_ADDR_BASE + addr ) );
    }

    return 0;
}

uint16_t system_isp_read_16( uint32_t addr )
{
    if( addr < ACAMERA_ISP_MAX_ADDR )
    {
        return *( ( uint16_t * ) ( SYSTEM_ISP_HW_ADDR_BASE + addr ) );
    }

    return 0;
}

uint8_t system_isp_read_8( uint32_t addr )
{
    if( addr < ACAMERA_ISP_MAX_ADDR )
    {
        return *( ( uint8_t * ) ( SYSTEM_ISP_HW_ADDR_BASE + addr ) );
    }

    return 0;
}

void system_isp_write_32( uint32_t addr,
                          uint32_t data )
{
    if( addr < ACAMERA_ISP_MAX_ADDR )
    {
        *( ( uint32_t * ) ( SYSTEM_ISP_HW_ADDR_BASE + addr ) ) = data;
    }
}

void system_isp_write_16( uint32_t addr,
                          uint16_t data )
{
    if( addr < ACAMERA_ISP_MAX_ADDR )
    {
        *( ( uint16_t * ) ( SYSTEM_ISP_HW_ADDR_BASE + addr ) ) = data;
    }
}

void system_isp_write_8( uint32_t addr,
                         uint8_t data )
{
    if( addr < ACAMERA_ISP_MAX_ADDR )
    {
        *( ( uint8_t * ) ( SYSTEM_ISP_HW_ADDR_BASE + addr ) ) = data;
    }
}
