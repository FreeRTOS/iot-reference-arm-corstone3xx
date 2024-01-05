/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "app_config.h"
#include "aws_clientcredential.h"
#include "dev_mode_key_provisioning.h"

#include "mqtt_agent_task.h"

#include "bsp_serial.h"
#include "tfm_ns_interface.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "mbedtls/threading.h"
#include "mbedtls/platform.h"
#include "threading_alt.h"

#include "events.h"

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "MAIN"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

psa_key_handle_t xOTACodeVerifyKeyHandle = 0;

#ifdef INTEGRATION_TESTS
    extern void RunQualificationTest( void );

    static void qual_task( void * arg )
    {
        ( void ) arg;
        RunQualificationTest();
        LogInfo( ( "RunQualificationTest returned\n" ) );
        vTaskDelete( NULL );
    }
#endif // INTEGRATION_TESTS

extern void vStartOtaTask( void );
extern int32_t network_startup( void );
extern BaseType_t xStartPubSubTasks( uint32_t ulNumPubsubTasks,
                                     configSTACK_DEPTH_TYPE uxStackSize,
                                     UBaseType_t uxPriority );

extern uint32_t tfm_ns_interface_init( void );

static bool xAreAwsCredentialsValid( void )
{
    if( ( strcmp( clientcredentialMQTT_BROKER_ENDPOINT, "dummy.endpointid.amazonaws.com" ) == 0 ) ||
        ( strcmp( clientcredentialIOT_THING_NAME, "dummy_thingname" ) == 0 ) )
    {
        printf( "[ERR] INVALID BROKER ENDPOINT AND/OR THING NAME.\r\n" );
        printf( "[ERR] Set the right credentials in aws_clientcredential.h\r\n" );
        return false;
    }

    return true;
}

void vAssertCalled( const char * pcFile,
                    unsigned long ulLine )
{
    printf( "ASSERT failed! file %s:%lu, \n", pcFile, ulLine );

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

/**
 * TODO: This function is only used in the PKCS#11 test case. In the PKCS#11 test,
 * it calls the mbedtls steps to generate the random number, so this function
 * is needed. But in the PKCS#11 library, we call the C_GenerateRandom to
 * generate a random number and do not need to call this function.
 */
int mbedtls_hardware_poll( void * data,
                           unsigned char * output,
                           size_t len,
                           size_t * olen )
{
    ( void ) ( data );
    ( void ) ( len );

    static uint32_t random_number = 0;

    random_number += 8;
    memcpy( output, &random_number, sizeof( uint32_t ) );
    *olen = sizeof( uint32_t );

    return 0;
}

BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    psa_status_t xPsaStatus = PSA_ERROR_PROGRAMMER_ERROR;

    xPsaStatus = psa_generate_random( ( uint8_t * ) ( pulNumber ), sizeof( uint32_t ) );

    if( xPsaStatus == PSA_SUCCESS )
    {
        return pdTRUE;
    }
    else
    {
        return pdFALSE;
    }
}

uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                             uint16_t usSourcePort,
                                             uint32_t ulDestinationAddress,
                                             uint16_t usDestinationPort )
{
    psa_status_t xPsaStatus = PSA_ERROR_PROGRAMMER_ERROR;
    uint32_t uxRandomValue = 0U;

    /* Unused parameters. */
    ( void ) ulSourceAddress;
    ( void ) usSourcePort;
    ( void ) ulDestinationAddress;
    ( void ) usDestinationPort;

    xPsaStatus = psa_generate_random( ( uint8_t * ) ( &uxRandomValue ), sizeof( uint32_t ) );

    if( xPsaStatus != PSA_SUCCESS )
    {
        LogError( ( "psa_generate_random failed with %d.", xPsaStatus ) );
        configASSERT( 0 );
    }

    return uxRandomValue;
}

int main()
{
    UBaseType_t xRetVal;
    int32_t status;

    bsp_serial_init();

    if( xAreAwsCredentialsValid() != true )
    {
        return EXIT_FAILURE;
    }

    /* Create logging task */
    xLoggingTaskInitialize( appCONFIG_LOGGING_TASK_STACK_SIZE,
                            appCONFIG_LOGGING_TASK_PRIORITY,
                            appCONFIG_LOGGING_MESSAGE_QUEUE_LENGTH );

    xRetVal = tfm_ns_interface_init();

    if( xRetVal != 0 )
    {
        LogError( ( "TF-M non-secure interface init failed with [%d]. Exiting...\n", xRetVal ) );
        return EXIT_FAILURE;
    }
    else
    {
        LogInfo( ( "PSA Framework version is: %d\n", psa_framework_version() ) );
    }

    /* Initialise system events group. */
    vEventHelperInit();

    /* Configure Mbed TLS memory APIs to use FreeRTOS heap APIs */
    mbedtls_platform_set_calloc_free( mbedtls_platform_calloc, mbedtls_platform_free );

    xRetVal = vDevModeKeyProvisioning();

    if( xRetVal != CKR_OK )
    {
        LogError( ( "Device key provisioning failed [%d]\n", xRetVal ) );
        LogError( ( "Device cannot connect to IoT Core. Exiting...\n" ) );
        return EXIT_FAILURE;
    }
    else
    {
        LogInfo( ( "Device key provisioning succeeded \n" ) );
        status = xOtaProvisionCodeSigningKey( &xOTACodeVerifyKeyHandle, 3072 );

        if( status != PSA_SUCCESS )
        {
            LogError( ( "OTA signing key provision failed [%d]\n", status ) );
        }

        LogInfo( ( "OTA signing key provisioning succeeded \n" ) );
    }

    status = network_startup();

    if( status == 0 )
    {
        mbedtls_threading_set_alt( mbedtls_platform_mutex_init,
                                   mbedtls_platform_mutex_free,
                                   mbedtls_platform_mutex_lock,
                                   mbedtls_platform_mutex_unlock );

        #ifdef INTEGRATION_TESTS
            xTaskCreate( qual_task,
                         "qual",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         tskIDLE_PRIORITY + 1,
                         NULL );
        #else
            /* Start MQTT agent task */
            vStartMqttAgentTask();

            /* Start OTA task*/
            vStartOtaTask();

            /*Start demo task once agent task is started. */
            ( void ) xStartPubSubTasks( appCONFIG_MQTT_NUM_PUBSUB_TASKS,
                                        appCONFIG_MQTT_PUBSUB_TASK_STACK_SIZE,
                                        appCONFIG_MQTT_PUBSUB_TASK_PRIORITY );
        #endif // INTEGRATION_TESTS

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
    }

    /* Code execution will never reach this line */
    return EXIT_FAILURE;
}

/**
 * Dummy implementation of the callback function vApplicationStackOverflowHook().
 */
#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )
    __WEAK void vApplicationStackOverflowHook( TaskHandle_t xTask,
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

    __WEAK void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
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
    __WEAK void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
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
