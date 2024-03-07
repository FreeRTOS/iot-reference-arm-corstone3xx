/* Copyright 2021-2024, Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "FreeRTOS.h"
#include "blink_task.h"
#include "mps3_leds.h"
#include "task.h"

#include <stdbool.h>
#include <stdio.h>

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "Blink"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

#define blinky_taskBLINK_TIMER_PERIOD_MS    250

enum
{
    LED1 = 1 << 0,
    LED2 = 1 << 1,
    LED3 = 1 << 2,
    LED4 = 1 << 3,
    LED5 = 1 << 4,
    LED6 = 1 << 5,
    LED_ALIVE = LED6,
    LED7 = 1 << 6,
    LED8 = 1 << 7,
    LED9 = 1 << 8,
    LED10 = 1 << 9,
    LED_ALL = 0xFF
};


void vStartBlinkTask( void )
{
    if(
        xTaskCreate(
            vBlinkTask,
            "BLINK_TASK ",
            appCONFIG_BLINK_TASK_STACK_SIZE,
            NULL,
            appCONFIG_BLINK_TASK_PRIORITY,
            NULL
            ) != pdPASS
        )
    {
        LogError( ( "Failed to create Blink Task\r\n" ) );
    }
}

void vBlinkTask( void * pvParameters )
{
    ( void ) pvParameters;

    LogInfo( ( "Blink task started\r\n" ) );

    mps3_leds_init();

    if( mps3_leds_turn_off( LED_ALL ) != true )
    {
        LogError( ( "Failed to turn all LEDs off\r\n" ) );
        return;
    }

    const uint32_t ulTicksInterval = blinky_taskBLINK_TIMER_PERIOD_MS * configTICK_RATE_HZ / 1000;

    while( 1 )
    {
        if( ulTicksInterval == 0U )
        {
            return;
        }

        vTaskDelay( ulTicksInterval );

        if( mps3_leds_toggle( LED_ALIVE ) != true )
        {
            LogError( ( "Failed to toggle LED_ALIVE\r\n" ) );
            return;
        }
    }
}
