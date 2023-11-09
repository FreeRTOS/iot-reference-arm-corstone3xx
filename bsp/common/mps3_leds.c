/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdlib.h>
#include "mps3_leds.h"
#include "arm_mps3_io_drv.h"
#include "device_cfg.h"
#include "device_definition.h"

#define TOTAL_NUMBER_OF_LEDS    10UL
#define MAX_LED_MASK            ( ( 1U << TOTAL_NUMBER_OF_LEDS ) - 1 )
#define GET_BIT_STATE( value, position )    ( ( ( ( value ) >> ( position ) ) & 1UL ) )
#define TOGGLE_BIT( bit )                   ( bit == 1 ? ( 0 ) : ( 1 ) )

static uint32_t leds_state;

size_t mps3_leds_get_total_number( void )
{
    return TOTAL_NUMBER_OF_LEDS;
}

void mps3_leds_init( void )
{
    arm_mps3_io_write_leds( &MPS3_IO_DEV, ARM_MPS3_IO_ACCESS_PORT, 0, 0 );
    leds_state = 0x0;
}

bool mps3_leds_set_state( uint32_t new_leds_state )
{
    /*check if desired state is possible */
    if( new_leds_state > MAX_LED_MASK )
    {
        return false;
    }

    for( size_t position = 0; position < TOTAL_NUMBER_OF_LEDS; position++ )
    {
        if( GET_BIT_STATE( ( new_leds_state ^ leds_state ), position ) )
        {
            /* New state for LED requested */
            uint32_t led_state = GET_BIT_STATE( new_leds_state, position );
            arm_mps3_io_write_leds(
                &MPS3_IO_DEV,
                ARM_MPS3_IO_ACCESS_PIN,
                position,
                led_state
                );
        }
    }

    leds_state = new_leds_state;

    return true;
}

uint32_t mps3_leds_get_state( void )
{
    return arm_mps3_io_read_leds( &MPS3_IO_DEV, ARM_MPS3_IO_ACCESS_PORT, 0 );
}

uint8_t mps3_leds_get_pin_state( uint8_t led_number )
{
    return ( uint8_t ) arm_mps3_io_read_leds( &MPS3_IO_DEV, ARM_MPS3_IO_ACCESS_PIN, led_number );
}

bool mps3_leds_turn_on( uint32_t leds )
{
    uint32_t new_state = ( mps3_leds_get_state() | leds );

    return mps3_leds_set_state( new_state );
}

bool mps3_leds_turn_off( uint32_t leds )
{
    uint32_t new_state = ( mps3_leds_get_state() & ~( leds ) );

    return mps3_leds_set_state( new_state );
}

bool mps3_leds_toggle( uint32_t leds )
{
    uint32_t new_state = ( mps3_leds_get_state() ^ leds );

    return mps3_leds_set_state( new_state );
}
