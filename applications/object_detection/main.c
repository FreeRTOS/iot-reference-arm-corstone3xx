/* Copyright 2023-2024, Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "app_config.h"
#include "aws_clientcredential.h"
#include "blink_task.h"
#include "dev_mode_key_provisioning.h"
#include "events.h"
#include "mbedtls/platform.h"
#include "mbedtls/threading.h"
#include "ml_interface.h"
#include "mqtt_agent_task.h"
#include "tfm_ns_interface.h"
#include "bsp_serial.h"
#include "aws_mbedtls_config.h"
#include "threading_alt.h"
#include "psa/crypto.h"
#include "tfm_ns_interface.h"

#include "FreeRTOSConfig.h"

#ifdef AUDIO_VSI
    #include "Driver_SAI.h"
#endif

/*
 * Semihosting is a mechanism that enables code running on an ARM target
 * to communicate and use the Input/Output facilities of a host computer
 * that is running a debugger.
 * There is an issue where if you use armclang at -O0 optimisation with
 * no parameters specified in the main function, the initialisation code
 * contains a breakpoint for semihosting by default. This will stop the
 * code from running before main is reached.
 * Semihosting can be disabled by defining __ARM_use_no_argv symbol
 * (or using higher optimization level).
 */
#if defined( __ARMCC_VERSION ) && ( __ARMCC_VERSION >= 6010050 )
    __asm( "  .global __ARM_use_no_argv\n" );
#endif

extern void vStartOtaTask( void );
extern int32_t network_startup( void );

#ifdef AUDIO_VSI
    void vVsiCallbackTask( void * arg );
    static void prvStartVsiCallbackTask( void );
    extern uint32_t ulVsiEvent;
    extern void (* pxOnVsiEvent)( void * );
    extern void * pvVsiContext;
    TaskHandle_t xVsiTaskHandle = NULL;
#endif

psa_key_handle_t xOTACodeVerifyKeyHandle = 0;
QueueHandle_t xMlMqttQueue = NULL;

static bool prvAreAwsCredentialsValid( void )
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

void vOtaActiveHook( void )
{
    vMlTaskInferenceStop();
}

void vOtaNotActiveHook( void )
{
    vMlTaskInferenceStart();
}

void vAssertCalled( const char * pcFile,
                    unsigned long ulLine )
{
    printf( "ASSERT failed! file %s:%lu, \n", pcFile, ulLine );

    taskENTER_CRITICAL();
    {
        /* Use the debugger to set ul to a non-zero value in order to step out
         *      of this function to determine why it was called. */
        volatile uint32_t ulLooping = 0;

        while( ulLooping == 0LU )
        {
            portNOP();
        }
    }
    taskEXIT_CRITICAL();
}

BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    return ( BaseType_t ) ( psa_generate_random( ( uint8_t * ) ( pulNumber ), sizeof( uint32_t ) ) == PSA_SUCCESS );
}

uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                             uint16_t usSourcePort,
                                             uint32_t ulDestinationAddress,
                                             uint16_t usDestinationPort )
{
    ( void ) ulSourceAddress;
    ( void ) usSourcePort;
    ( void ) ulDestinationAddress;
    ( void ) usDestinationPort;

    uint32_t uxRandomValue = 0U;
    psa_status_t xPsaStatus = psa_generate_random( ( uint8_t * ) ( &uxRandomValue ), sizeof( uint32_t ) );

    if( xPsaStatus != PSA_SUCCESS )
    {
        LogError( ( "psa_generate_random failed with %d.", xPsaStatus ) );
        configASSERT( 0 );
    }

    return uxRandomValue;
}

int main( void )
{
    bsp_serial_init();

    xLoggingTaskInitialize( appCONFIG_LOGGING_TASK_STACK_SIZE,
                            appCONFIG_LOGGING_TASK_PRIORITY,
                            appCONFIG_LOGGING_MESSAGE_QUEUE_LENGTH );

    UBaseType_t uxStatus = tfm_ns_interface_init();

    if( uxStatus != 0 )
    {
        LogError( ( "TF-M non-secure interface init failed with [%d]. Exiting...\n", uxStatus ) );
        return EXIT_FAILURE;
    }

    LogInfo( ( "PSA Framework version is: %d\n", psa_framework_version() ) );

    if( xEventHelperInit() != 0 )
    {
        LogError( ( "System events group was not initialised successfully" ) );
        return EXIT_FAILURE;
    }

    ( void ) mbedtls_platform_set_calloc_free( mbedtls_platform_calloc, mbedtls_platform_free );

    mbedtls_threading_set_alt( mbedtls_platform_mutex_init,
                               mbedtls_platform_mutex_free,
                               mbedtls_platform_mutex_lock,
                               mbedtls_platform_mutex_unlock );

    #if defined PSA_CRYPTO_IMPLEMENTATION_MBEDTLS
        psa_status_t xResult = psa_crypto_init();

        if( xResult != PSA_SUCCESS )
        {
            printf( "Psa crypto init failed with return code = %d\r\n", xResult );
        }
    #endif

    UBaseType_t xReturnValue = vDevModeKeyProvisioning();

    if( xReturnValue != CKR_OK )
    {
        LogError( ( "Device key provisioning failed [%d]\n", xReturnValue ) );
        LogError( ( "Device cannot connect to IoT Core. Exiting...\n" ) );
        return EXIT_FAILURE;
    }

    LogInfo( ( "Device key provisioning succeeded \n" ) );

    /* FIXME: Magic value */
    uxStatus = xOtaProvisionCodeSigningKey( &xOTACodeVerifyKeyHandle, 3072 );

    if( uxStatus != PSA_SUCCESS )
    {
        LogError( ( "OTA signing key provision failed [%d]\n", uxStatus ) );
    }

    LogInfo( ( "OTA signing key provisioning succeeded \n" ) );

    /* The next initializations are done as a part of the main */
    /* function as these resources are shared between tasks */
    /* and it is not guranteed that the task which initialise */
    /* these resources will start first before the tasks using them. */
    xMlMqttQueue = xQueueCreate( 20, sizeof( MLMqttMsg_t ) );

    if( xMlMqttQueue == NULL )
    {
        LogError( ( "Failed to create xMlMqttQueue\r\n" ) );
        return EXIT_FAILURE;
    }

    vMlTaskInferenceStop();

    if( prvAreAwsCredentialsValid() == true )
    {
        if( network_startup() != 0 )
        {
            return EXIT_FAILURE;
        }

        vStartMqttAgentTask();

        vStartOtaTask();

        vStartMlMqttTask();
    }
    else
    {
        vMlTaskInferenceStart();
    }

    vStartBlinkTask();

    vStartMlTask( NULL );

    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks to be created.  See the memory management section on the
     * FreeRTOS web site for more details.  NOTE: This demo uses static allocation
     * for the idle and timer tasks so this line should never execute. */
    while( 1 )
    {
    }

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
        static StaticTask_t xIdleTCB = { 0 };
        static StackType_t xIdleStack[ configMINIMAL_STACK_SIZE ] = { 0 };

        *ppxIdleTaskTCBBuffer = &xIdleTCB;
        *ppxIdleTaskStackBuffer = &xIdleStack[ 0 ];
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
        static StaticTask_t xTimerTCB = { 0 };
        static StackType_t xTimerStack[ configTIMER_TASK_STACK_DEPTH ] = { 0 };

        *ppxTimerTaskTCBBuffer = &xTimerTCB;
        *ppxTimerTaskStackBuffer = &xTimerStack[ 0 ];
        *pulTimerTaskStackSize = ( uint32_t ) configTIMER_TASK_STACK_DEPTH;
    }
#endif /* if ( configSUPPORT_STATIC_ALLOCATION == 1 ) */

#ifdef AUDIO_VSI
    void vVsiCallbackTask( void * arg )
    {
        ( void ) arg;

        while( 1 )
        {
            ( void ) ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

            if( ( ulVsiEvent & ARM_SAI_EVENT_RECEIVE_COMPLETE ) && ( pxOnVsiEvent ) )
            {
                pxOnVsiEvent( pvVsiContext );
            }

            if( ulVsiEvent & ARM_SAI_EVENT_RX_OVERFLOW )
            {
                LogError( ( "VSI Receive Overflow Error \r\n" ) );
            }
        }
    }

    static void prvStartVsiCallbackTask( void )
    {
        if( xTaskCreate( vVsiCallbackTask,
                         "VSI_CALLBACK_TASK",
                         appCONFIG_VSI_CALLBACK_TASK_STACK_SIZE,
                         NULL,
                         appCONFIG_VSI_CALLBACK_TASK_PRIORITY,
                         &xVsiTaskHandle ) != pdPASS )
        {
            LogError( ( "Failed to create Vsi Callback Task\r\n" ) );
        }
    }
#endif /* ifdef AUDIO_VSI */
