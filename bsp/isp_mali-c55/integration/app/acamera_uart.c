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

#include "acamera_uart.h"
#include "uart_cmsdk_drv.h"
#include "uart_cmsdk_reg_map.h"

#include "Driver_USART.h"
#include "device_cfg.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include CMSIS_device_header

#include "FreeRTOS.h"
#include "task.h"

/* FIXME: Should use USART2 */
extern ARM_DRIVER_USART Driver_USART0;
#define ISP_UART_DRIVER    Driver_USART0

#define ASSERT_HIGH( X )    assert( X == ARM_DRIVER_OK )

volatile uint32_t UART_PLEASE_HOLD_STREAM = 0;
#define ACAMERA_UART_RECEIVE_BUFFER_SIZE    ( 4096 )
static uint8_t receive_buffer[ ACAMERA_UART_RECEIVE_BUFFER_SIZE ];
static uint32_t rb_last_read = 0;
static uint32_t rb_next_free = 0;
void uart_cb( uint32_t event )
{
    if( event == ARM_USART_EVENT_RECEIVE_COMPLETE )
    {
        UART_PLEASE_HOLD_STREAM = 1;
        rb_next_free++;

        if( rb_next_free >= ACAMERA_UART_RECEIVE_BUFFER_SIZE )
        {
            rb_next_free -= ACAMERA_UART_RECEIVE_BUFFER_SIZE;

            if( rb_next_free > rb_last_read )
            {
                printf( "Circular buffer overflow!\r\n" );
                return;
            }
        }

        ISP_UART_DRIVER.Receive( &receive_buffer[ rb_next_free ], 1 );
    }
}

void acamera_uart_init( acamera_uart_t * uart )
{
    int32_t ret;

    ret = ISP_UART_DRIVER.Initialize( uart_cb );
    ASSERT_HIGH( ret );

    ret = ISP_UART_DRIVER.PowerControl( ARM_POWER_FULL );
    ASSERT_HIGH( ret );

    /* FVP Receive gets unstable if this is too big */
    ret = ISP_UART_DRIVER.Control( ARM_USART_MODE_ASYNCHRONOUS, 38400 );
    ASSERT_HIGH( ret );
    ( void ) ret;

    ( void ) ISP_UART_DRIVER.Control( ARM_USART_CONTROL_RX, 1 );
    ( void ) ISP_UART_DRIVER.Control( ARM_USART_CONTROL_TX, 1 );
    *uart = ( uint32_t ) &ISP_UART_DRIVER;

    ISP_UART_DRIVER.Receive( receive_buffer, 1 );
}

int acamera_uart_read( void * p_ctrl,
                       uint8_t * data,
                       int size )
{
    uint32_t available, size_copied;

    if( size == 0 )
    {
        return 0;
    }

    available = rb_next_free < rb_last_read ? ( rb_next_free + ACAMERA_UART_RECEIVE_BUFFER_SIZE - rb_last_read )
                : ( rb_next_free - rb_last_read );

    while( available < size )
    {
        if( available == 0 )
        {
            vTaskDelay( 1 );
        }

        available = rb_next_free < rb_last_read ? ( rb_next_free + ACAMERA_UART_RECEIVE_BUFFER_SIZE - rb_last_read )
                    : ( rb_next_free - rb_last_read );
    }

    size_copied = 0;

    if( rb_last_read + size > ACAMERA_UART_RECEIVE_BUFFER_SIZE )
    {
        /* Read until the end of circular buffer */
        uint32_t to_copy = ACAMERA_UART_RECEIVE_BUFFER_SIZE - rb_last_read;
        memcpy( data, &receive_buffer[ rb_last_read ], to_copy );
        size_copied += to_copy;
        rb_last_read = 0;
        size -= size_copied;
    }

    memcpy( &data[ size_copied ], &receive_buffer[ rb_last_read ], size );
    rb_last_read += size;
    size += size_copied;
    return size;
}

int acamera_uart_write( void * p_ctrl,
                        const uint8_t * data,
                        int size )
{
    int32_t ret;

    /* Add a busy before sending. */
    while( ISP_UART_DRIVER.GetStatus().tx_busy )
    {
    }

    ret = ISP_UART_DRIVER.Send( data, size );

    if( ret != ARM_DRIVER_OK )
    {
        return 0;
    }

    /* Add a busy wait after sending. */
    while( ISP_UART_DRIVER.GetStatus().tx_busy )
    {
    }

    uint32_t available = rb_next_free < rb_last_read ? ( rb_next_free + ACAMERA_UART_RECEIVE_BUFFER_SIZE - rb_last_read )
                         : ( rb_next_free - rb_last_read );

    if( available == 0 )
    {
        UART_PLEASE_HOLD_STREAM = 0;
        taskYIELD();
    }

    return ISP_UART_DRIVER.GetTxCount();
}
