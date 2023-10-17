/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "bsp_serial.h"
#include "tfm_ns_interface.h"

#include <stdio.h>
#include <stdlib.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

extern uint32_t tfm_ns_interface_init( void );

void vAssertCalled( const char * pcFile,
                    unsigned long ulLine )
{
    printf( "ASSERT failed! file %s:%lu, \r\n", pcFile, ulLine );

    taskENTER_CRITICAL();
    {
        volatile unsigned long looping = 0;

        /* Use the debugger to set ul to a non-zero value in order to step out
         *      of this function to determine why it was called. */
        while( looping == 0LU )
        {
            portNOP();
        }
    }
    taskEXIT_CRITICAL();
}

static void app_task( void * arg )
{
    TickType_t xNextWakeTime;
    uint32_t led_status = 1;

    /* Prevent the compiler warning about the unused parameter. */
    ( void ) arg;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    printf( "psa_framework_version is: %d\n", psa_framework_version() );

    while( 1 )
    {
        if( led_status )
        {
            printf( "LED on\r\n" );
            led_status = 0;
        }
        else
        {
            printf( "LED off\r\n" );
            led_status = 1;
        }

        /* Place this task in the blocked state until it is time to run again.
         *  The block time is specified in ticks, pdMS_TO_TICKS() was used to
         *  convert a time specified in milliseconds into a time specified in ticks.
         *  While in the Blocked state this task will not consume any CPU time.
         */
        vTaskDelayUntil( &xNextWakeTime, pdMS_TO_TICKS( 1000UL ) );
    }
}

int main()
{
    bsp_serial_init();

    uint32_t ret = tfm_ns_interface_init();

    if( ret != 0 )
    {
        printf( "tfm_ns_interface_init() failed: %u\r\n", ret );
        return EXIT_FAILURE;
    }

    xTaskCreate( app_task,                 /* The function that implements the task. */
                 "App",                    /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                 configMINIMAL_STACK_SIZE, /* The size of the stack to allocate to the task. */
                 NULL,                     /* The parameter passed to the task - not used in this simple case. */
                 tskIDLE_PRIORITY + 1,     /* The priority assigned to the task. */
                 NULL );                   /* The task handle is not required, so NULL is passed. */

    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details.  NOTE: This demo uses static allocation
     * for the idle and timer tasks so this line should never execute. */
    for( ; ; )
    {
    }

    /* Code execution will never reach this line */
    return EXIT_FAILURE;
}

/**
 * Dummy implementation of the callback function vApplicationStackOverflowHook().
 */
#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )
    void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName )
    {
        ( void ) xTask;
        ( void ) pcTaskName;

        /* Assert when stack overflow is enabled but no application defined function exists */
        configASSERT( 0 );
    }
#endif

/*---------------------------------------------------------------------------*/
#if ( configSUPPORT_STATIC_ALLOCATION == 1 )

/*
 * vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
 * equals to 1 and is required for static memory allocation support.
 */
    void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                        StackType_t ** ppxIdleTaskStackBuffer,
                                        uint32_t * pulIdleTaskStackSize )
    {
        /* Idle task control block and stack */
        static StaticTask_t Idle_TCB;
        static StackType_t Idle_Stack[ configMINIMAL_STACK_SIZE ];

        *ppxIdleTaskTCBBuffer = &Idle_TCB;
        *ppxIdleTaskStackBuffer = &Idle_Stack[ 0 ];
        *pulIdleTaskStackSize = ( uint32_t ) configMINIMAL_STACK_SIZE;
    }

/*
 * vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
 * equals to 1 and is required for static memory allocation support.
 */
    void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                         StackType_t ** ppxTimerTaskStackBuffer,
                                         uint32_t * pulTimerTaskStackSize )
    {
        /* Timer task control block and stack */
        static StaticTask_t Timer_TCB;
        static StackType_t Timer_Stack[ configTIMER_TASK_STACK_DEPTH ];

        *ppxTimerTaskTCBBuffer = &Timer_TCB;
        *ppxTimerTaskStackBuffer = &Timer_Stack[ 0 ];
        *pulTimerTaskStackSize = ( uint32_t ) configTIMER_TASK_STACK_DEPTH;
    }
#endif /* if ( configSUPPORT_STATIC_ALLOCATION == 1 ) */
