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

#include "system_interrupts.h"
#include "acamera_logger.h"
#include "acamera_types.h"
#include "system_hw_io.h"

#include "platform_irq.h"

#include CMSIS_device_header

system_interrupt_handler_t isp_handler;
void * isp_param;

void system_interrupts_init( void )
{
}

void ISP_C55_Handler( void )
{
    uint32_t mask = system_hw_read_32( 0x44 );

    isp_handler( isp_param, mask );
}

void system_interrupt_set_handler( system_interrupt_handler_t handler,
                                   void * param )
{
    isp_handler = handler;
    isp_param = param;
    NVIC_SetVector( ISP_IRQn, ( uint32_t ) ISP_C55_Handler );
}

void system_interrupts_enable( void )
{
    NVIC_EnableIRQ( ISP_IRQn );
}

void system_interrupts_disable( void )
{
    NVIC_DisableIRQ( ISP_IRQn );
}
