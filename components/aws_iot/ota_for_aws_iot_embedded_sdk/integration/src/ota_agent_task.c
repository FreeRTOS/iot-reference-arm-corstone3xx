/*
 * FreeRTOS V202012.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "app_config.h"

#include "mqtt_agent_task.h"

/* includes for TFM */
#include "psa/update.h"

/* includes for OTA PAL PSA */
#include "version/application_version.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Demo config includes. */
#include "demo_config.h"
#include "iot_default_root_certificates.h"

/* Library config includes. */
#include "ota_config.h"

/* Subscription manager header include. */
#include "subscription_manager.h"

/* OTA Library include. */
#include "ota.h"

/* OTA Library Interface include. */
#include "ota_os_freertos.h"
#include "ota_mqtt_interface.h"
#include "ota_platform_interface.h"

/* Include firmware version struct definition. */
#include "ota_appversion32.h"

/* Include platform abstraction header. */
#include "ota_pal.h"

/*------------- Demo configurations -------------------------*/

/**
 * @brief The maximum size of the file paths used in the demo.
 */
#define otaexampleMAX_FILE_PATH_SIZE                     ( 260 )

/**
 * @brief The maximum size of the stream name required for downloading update file
 * from streaming service.
 */
#define otaexampleMAX_STREAM_NAME_SIZE                   ( 128 )

/**
 * @brief The delay used in the OTA demo task to periodically output the OTA
 * statistics like number of packets received, dropped, processed and queued per connection.
 */
#define otaexampleTASK_DELAY_MS                          ( 10000U )

/**
 * @brief The maximum time for which OTA demo waits for an MQTT operation to be complete.
 * This involves receiving an acknowledgment for broker for SUBSCRIBE, UNSUBSCRIBE and non
 * QOS0 publishes.
 */
#define otaexampleMQTT_TIMEOUT_MS                        ( 5000U )

/**
 * @brief The common prefix for all OTA topics.
 *
 * Thing name is substituted with a wildcard symbol `+`. OTA agent
 * registers with MQTT broker with the thing name in the topic. This topic
 * filter is used to match incoming packet received and route them to OTA.
 * Thing name is not needed for this matching.
 */
#define OTA_TOPIC_PREFIX                                 "$aws/things/+/"

/**
 * @brief Wildcard topic filter for job notification.
 * The filter is used to match the constructed job notify topic filter from OTA agent and register
 * appropriate callback for it.
 */
#define OTA_JOB_NOTIFY_TOPIC_FILTER                      OTA_TOPIC_PREFIX "jobs/notify-next"

/**
 * @brief Length of job notification topic filter.
 */
#define OTA_JOB_NOTIFY_TOPIC_FILTER_LENGTH               ( ( uint16_t ) ( sizeof( OTA_JOB_NOTIFY_TOPIC_FILTER ) - 1 ) )

/**
 * @brief Wildcard topic filter for matching job response messages.
 * This topic filter is used to match the responses from OTA service for OTA agent job requests. THe
 * topic filter is a reserved topic which is not subscribed with MQTT broker.
 *
 */
#define OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER           OTA_TOPIC_PREFIX "jobs/$next/get/accepted"

/**
 * @brief Length of job accepted response topic filter.
 */
#define OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER_LENGTH    ( ( uint16_t ) ( sizeof( OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER ) - 1 ) )


/**
 * @brief Wildcard topic filter for matching OTA data packets.
 *  The filter is used to match the constructed data stream topic filter from OTA agent and register
 * appropriate callback for it.
 */
#define OTA_DATA_STREAM_TOPIC_FILTER           OTA_TOPIC_PREFIX  "streams/#"

/**
 * @brief Length of data stream topic filter.
 */
#define OTA_DATA_STREAM_TOPIC_FILTER_LENGTH    ( ( uint16_t ) ( sizeof( OTA_DATA_STREAM_TOPIC_FILTER ) - 1 ) )


/**
 * @brief Starting index of client identifier within OTA topic.
 */
#define OTA_TOPIC_CLIENT_IDENTIFIER_START_IDX    ( 12U )

/**
 * @brief Default topic filter for OTA.
 * This is used to route all the packets for OTA reserved topics which OTA agent has not subscribed for.
 */
#define OTA_DEFAULT_TOPIC_FILTER                 OTA_TOPIC_PREFIX "jobs/#"

/**
 * @brief Length of default topic filter.
 */
#define OTA_DEFAULT_TOPIC_FILTER_LENGTH          ( ( uint16_t ) ( sizeof( OTA_DEFAULT_TOPIC_FILTER ) - 1 ) )

/**
 * @brief Used to clear bits in a task's notification value.
 */
#define otaexampleMAX_UINT32                     ( 0xffffffff )

/**
 * @brief Stack size required for OTA agent task.
 */
#define OTA_AGENT_TASK_STACK_SIZE                ( 5000U )

/**
 * @brief Priority required for OTA agent task.
 */
#define OTA_AGENT_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1 )

/**
 * @brief The timeout for waiting for the agent to get suspended after closing the
 * connection.
 *
 * Timeout value should be large enough for OTA agent to finish any pending MQTT operations
 * and suspend itself.
 *
 */
#define OTA_SUSPEND_TIMEOUT_MS                   ( 10000U )

/*---------------------------------------------------------*/

/**
 * @brief Structure used to store the topic filter to ota callback mappings.
 */
typedef struct OtaTopicFilterCallback
{
    const char * pTopicFilter;
    uint16_t topicFilterLength;
    IncomingPubCallback_t callback;
} OtaTopicFilterCallback_t;

/*---------------------------------------------------------*/

/**
 * @brief Buffer used to store the firmware image file path.
 * Buffer is passed to the OTA agent during initialization.
 */
static uint8_t updateFilePath[ otaexampleMAX_FILE_PATH_SIZE ];

/**
 * @brief Buffer used to store the code signing certificate file path.
 * Buffer is passed to the OTA agent during initialization.
 */
static uint8_t certFilePath[ otaexampleMAX_FILE_PATH_SIZE ];

/**
 * @brief Buffer used to store the name of the data stream.
 * Buffer is passed to the OTA agent during initialization.
 */
static uint8_t streamName[ otaexampleMAX_STREAM_NAME_SIZE ];

/**
 * @brief Buffer used decode the CBOR message from the MQTT payload.
 * Buffer is passed to the OTA agent during initialization.
 */
static uint8_t decodeMem[ ( 1U << otaconfigLOG2_FILE_BLOCK_SIZE ) ];

/**
 * @brief Application buffer used to store the bitmap for requesting firmware image
 * chunks from MQTT broker. Buffer is passed to the OTA agent during initialization.
 */
static uint8_t bitmap[ OTA_MAX_BLOCK_BITMAP_SIZE ];

/**
 * @brief A statically allocated array of event buffers used by the OTA agent.
 * Maximum number of buffers are determined by how many chunks are requested
 * by OTA agent at a time along with an extra buffer to handle control message.
 * The size of each buffer is determined by the maximum size of firmware image
 * chunk, and other metadata send along with the chunk.
 */
static OtaEventData_t eventBuffer[ otaconfigMAX_NUM_OTA_DATA_BUFFERS ] = { 0 };

/*
 * @brief Mutex used to manage thread safe access of OTA event buffers.
 */
static SemaphoreHandle_t xBufferSemaphore;

/*---------------------------------------------------------*/

/**
 * @brief The MQTT agent manages the MQTT contexts.  This set the handle to the
 * context used by this demo.
 */
extern MQTTAgentContext_t xGlobalMqttAgentContext;

/*---------------------------------------------------------*/

/**
 * @brief Fetch an unused OTA event buffer from the pool.
 *
 * Demo uses a simple statically allocated array of fixed size event buffers. The
 * number of event buffers is configured by the param otaconfigMAX_NUM_OTA_DATA_BUFFERS
 * within ota_config.h. This function is used to fetch a free buffer from the pool for processing
 * by the OTA agent task. It uses a mutex for thread safe access to the pool.
 *
 * @return A pointer to an unusued buffer. NULL if there are no buffers available.
 */
static OtaEventData_t * prvOTAEventBufferGet( void );

/**
 * @brief Free an event buffer back to pool
 *
 * OTA demo uses a statically allocated array of fixed size event buffers . The
 * number of event buffers is configured by the param otaconfigMAX_NUM_OTA_DATA_BUFFERS
 * within ota_config.h. The function is used by the OTA application callback to free a buffer,
 * after OTA agent has completed processing with the event. The access to the pool is made thread safe
 * using a mutex.
 *
 * @param[in] pxBuffer Pointer to the buffer to be freed.
 */
static void prvOTAEventBufferFree( OtaEventData_t * const pxBuffer );

/**
 * @brief The function which runs the OTA agent task.
 *
 * The function runs the OTA Agent Event processing loop, which waits for
 * any events for OTA agent and process them. The loop never returns until the OTA agent
 * is shutdown. The tasks exits gracefully by freeing up all resources in the event of an
 *  OTA agent shutdown.
 *
 * @param[in] pvParam Any parameters to be passed to OTA agent task.
 */
static void prvOTAAgentTask( void * pvParam );


/**
 * @brief The function which runs the OTA demo task.
 *
 * The demo task initializes the OTA agent an loops until OTA agent is shutdown.
 * It reports OTA update statistics (which includes number of blocks received, processed and dropped),
 * at regular intervals.
 *
 * @param[in] pvParam Any parameters to be passed to OTA Demo task.
 */
static void vOtaDemoTask( void * pvParam );

/**
 * @brief The function which implements the flow for OTA demo.
 *
 * @return pdPASS if success or pdFAIL.
 */
static BaseType_t prvRunOTADemo( void );

/**
 * @brief Callback registered with the OTA library that notifies the OTA agent
 * of an incoming PUBLISH containing a job document.
 *
 * @param[in] pContext MQTT context which stores the connection.
 * @param[in] pPublishInfo MQTT packet information which stores details of the
 * job document.
 */
static void prvMqttJobCallback( void * pContext,
                                MQTTPublishInfo_t * pPublish );


/**
 * @brief Callback that notifies the OTA library when a data block is received.
 *
 * @param[in] pContext MQTT context which stores the connection.
 * @param[in] pPublishInfo MQTT packet that stores the information of the file block.
 */
static void prvMqttDataCallback( void * pContext,
                                 MQTTPublishInfo_t * pPublish );

/**
 * @brief Default callback used to receive unsolicited messages for OTA.
 *
 * The callback is not subscribed with MQTT broker, but only with local subscription manager.
 * A wildcard OTA job topic is used for subscription so that all unsolicited messages related to OTA is
 * forwarded to this callback for filtration. Right now the callback is used to filter responses to job requests
 * from the OTA service.
 *
 * @param[in] pvIncomingPublishCallbackContext MQTT context which stores the connection.
 * @param[in] pPublishInfo MQTT packet that stores the information of the file block.
 */
static void prvMqttDefaultCallback( void * pvIncomingPublishCallbackContext,
                                    MQTTPublishInfo_t * pxPublishInfo );

/**
 * @brief Register OTA callbacks with the subscription manager.
 *
 * @param[in] pTopicFilter The topic filter for which a  callback needs to be registered for.
 * @param[in] topicFilterLength length of the topic filter.
 *
 */
static void prvRegisterOTACallback( const char * pTopicFilter,
                                    uint16_t topicFilterLength );

/**
 * @brief Suspend OTA demo.
 *
 * @return   pPASS or pdFAIL.
 */
static BaseType_t prvSuspendOTA( void );

/**
 * @brief Resume OTA demo.
 *
 * @return   pPASS or pdFAIL.
 */
static BaseType_t prvResumeOTA( void );

/**
 * @brief Set OTA interfaces.
 *
 * @param[in]  pOtaInterfaces pointer to OTA interface structure.
 *
 * @return   None.
 */
static void setOtaInterfaces( OtaInterfaces_t * pOtaInterfaces );

/**
 * @brief Structure containing all application allocated buffers used by the OTA agent.
 * Structure is passed to the OTA agent during initialization.
 */
static OtaAppBuffer_t otaBuffer =
{
    .pUpdateFilePath    = updateFilePath,
    .updateFilePathsize = otaexampleMAX_FILE_PATH_SIZE,
    .pCertFilePath      = certFilePath,
    .certFilePathSize   = otaexampleMAX_FILE_PATH_SIZE,
    .pStreamName        = streamName,
    .streamNameSize     = otaexampleMAX_STREAM_NAME_SIZE,
    .pDecodeMemory      = decodeMem,
    .decodeMemorySize   = ( 1U << otaconfigLOG2_FILE_BLOCK_SIZE ),
    .pFileBitmap        = bitmap,
    .fileBitmapSize     = OTA_MAX_BLOCK_BITMAP_SIZE
};

/**
 * @brief Registry for all  mqtt topic filters to their corresponding callbacks for OTA.
 */
static OtaTopicFilterCallback_t otaTopicFilterCallbacks[] =
{
    {
        .pTopicFilter = OTA_JOB_NOTIFY_TOPIC_FILTER,
        .topicFilterLength = OTA_JOB_NOTIFY_TOPIC_FILTER_LENGTH,
        .callback = prvMqttJobCallback
    },
    {
        .pTopicFilter = OTA_DATA_STREAM_TOPIC_FILTER,
        .topicFilterLength = OTA_DATA_STREAM_TOPIC_FILTER_LENGTH,
        .callback = prvMqttDataCallback
    },
    {
        .pTopicFilter = OTA_DEFAULT_TOPIC_FILTER,
        .topicFilterLength = OTA_DEFAULT_TOPIC_FILTER_LENGTH,
        .callback = prvMqttDefaultCallback
    }
};

/*-----------------------------------------------------------*/

static void prvOTAEventBufferFree( OtaEventData_t * const pxBuffer )
{
    if( xSemaphoreTake( xBufferSemaphore, portMAX_DELAY ) == pdTRUE )
    {
        pxBuffer->bufferUsed = false;
        ( void ) xSemaphoreGive( xBufferSemaphore );
    }
}

/*-----------------------------------------------------------*/

static OtaEventData_t * prvOTAEventBufferGet( void )
{
    OtaEventData_t * pFreeBuffer = NULL;

    if( xSemaphoreTake( xBufferSemaphore, portMAX_DELAY ) == pdTRUE )
    {
        uint32_t ulIndex;

        for( ulIndex = 0; ulIndex < otaconfigMAX_NUM_OTA_DATA_BUFFERS; ulIndex++ )
        {
            if( eventBuffer[ ulIndex ].bufferUsed == false )
            {
                eventBuffer[ ulIndex ].bufferUsed = true;
                pFreeBuffer = &eventBuffer[ ulIndex ];
                break;
            }
        }

        ( void ) xSemaphoreGive( xBufferSemaphore );
    }

    return pFreeBuffer;
}

/*-----------------------------------------------------------*/

/**
 * @brief The OTA agent has completed the update job or it is in
 * self test mode. If it was accepted, we want to activate the new image.
 * This typically means we should reset the device to run the new firmware.
 * If now is not a good time to reset the device, it may be activated later
 * by your user code. If the update was rejected, just return without doing
 * anything and we will wait for another job. If it reported that we should
 * start test mode, normally we would perform some kind of system checks to
 * make sure our new firmware does the basic things we think it should do
 * but we will just go ahead and set the image as accepted for demo purposes.
 * The accept function varies depending on your platform. Refer to the OTA
 * PAL implementation for your platform in ota_pal.c to see what it
 * does for you.
 *
 * @param[in] event Specify if this demo is running with the AWS IoT
 * MQTT server. Set this to `false` if using another MQTT server.
 * @param[in] pData Data associated with the event.
 * @return None.
 */
static void otaAppCallback( OtaJobEvent_t event,
                            void * pData )
{
    OtaErr_t err = OtaErrUninitialized;

    switch( event )
    {
        case OtaJobEventActivate:
            LogInfo( ( "Received OtaJobEventActivate callback from OTA Agent." ) );

            /**
             * Activate the new firmware image immediately. Applications can choose to postpone
             * the activation to a later stage if needed.
             */
            ( void ) OTA_ActivateNewImage();

            /**
             * Activation of the new image failed. This indicates an error that requires a follow
             * up through manual activation by resetting the device. The demo reports the error
             * and shuts down the OTA agent.
             */
            LogError( ( "New image activation failed." ) );

            /* Shutdown OTA Agent, if it is required that the unsubscribe operations are not
             * performed while shutting down please set the second parameter to 0 instead of 1. */
            OTA_Shutdown( 0, 1 );


            break;

        case OtaJobEventFail:

            /**
             * No user action is needed here. OTA agent handles the job failure event.
             */
            LogInfo( ( "Received an OtaJobEventFail notification from OTA Agent." ) );

            break;

        case OtaJobEventStartTest:

            /* This demo just accepts the image since it was a good OTA update and networking
             * and services are all working (or we would not have made it this far). If this
             * were some custom device that wants to test other things before validating new
             * image, this would be the place to kick off those tests before calling
             * OTA_SetImageState() with the final result of either accepted or rejected. */

            LogInfo( ( "Received OtaJobEventStartTest callback from OTA Agent." ) );

            err = OTA_SetImageState( OtaImageStateAccepted );

            if( err == OtaErrNone )
            {
                LogInfo( ( "New image validation succeeded in self test mode." ) );
            }
            else
            {
                LogError( ( "Failed to set image state as accepted with error %d.", err ) );
            }

            break;

        case OtaJobEventProcessed:

            LogDebug( ( "OTA Event processing completed. Freeing the event buffer to pool." ) );
            configASSERT( pData != NULL );
            prvOTAEventBufferFree( ( OtaEventData_t * ) pData );

            break;

        case OtaJobEventSelfTestFailed:
            LogDebug( ( "Received OtaJobEventSelfTestFailed callback from OTA Agent." ) );

            /* Requires manual activation of previous image as self-test for
             * new image downloaded failed.*/
            LogError( ( "OTA Self-test failed for new image. shutting down OTA Agent." ) );

            /* Shutdown OTA Agent, if it is required that the unsubscribe operations are not
             * performed while shutting down please set the second parameter to 0 instead of 1. */
            OTA_Shutdown( 0, 1 );

            break;

        default:
            LogWarn( ( "Received an unhandled callback event from OTA Agent, event = %d", event ) );

            break;
    }
}

static void prvMqttJobCallback( void * pvIncomingPublishCallbackContext,
                                MQTTPublishInfo_t * pxPublishInfo )
{
    OtaEventData_t * pData;
    OtaEventMsg_t eventMsg = { 0 };

    configASSERT( pxPublishInfo != NULL );
    ( void ) pvIncomingPublishCallbackContext;

    LogInfo( ( "Received job message callback, size %ld.\n", pxPublishInfo->payloadLength ) );

    pData = prvOTAEventBufferGet();

    if( pData != NULL )
    {
        memcpy( pData->data, pxPublishInfo->pPayload, pxPublishInfo->payloadLength );
        pData->dataLength = pxPublishInfo->payloadLength;
        eventMsg.eventId = OtaAgentEventReceivedJobDocument;
        eventMsg.pEventData = pData;

        /* Send job document received event. */
        OTA_SignalEvent( &eventMsg );
    }
    else
    {
        LogError( ( "Error: No OTA data buffers available.\n" ) );
    }
}

/*-----------------------------------------------------------*/
static void prvMqttDefaultCallback( void * pvIncomingPublishCallbackContext,
                                    MQTTPublishInfo_t * pxPublishInfo )
{
    bool isMatch = false;

    ( void ) MQTT_MatchTopic( pxPublishInfo->pTopicName,
                              pxPublishInfo->topicNameLength,
                              OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER,
                              OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER_LENGTH,
                              &isMatch );

    if( isMatch == true )
    {
        prvMqttJobCallback( pvIncomingPublishCallbackContext, pxPublishInfo );
    }
}

/*-----------------------------------------------------------*/
static void prvMqttDataCallback( void * pvIncomingPublishCallbackContext,
                                 MQTTPublishInfo_t * pxPublishInfo )
{
    OtaEventData_t * pxData;
    OtaEventMsg_t eventMsg = { 0 };

    configASSERT( pxPublishInfo != NULL );
    ( void ) pvIncomingPublishCallbackContext;

    LogInfo( ( "Received data message callback, size %zu.\n", pxPublishInfo->payloadLength ) );

    pxData = prvOTAEventBufferGet();

    if( pxData != NULL )
    {
        memcpy( pxData->data, pxPublishInfo->pPayload, pxPublishInfo->payloadLength );
        pxData->dataLength = pxPublishInfo->payloadLength;
        eventMsg.eventId = OtaAgentEventReceivedFileBlock;
        eventMsg.pEventData = pxData;

        /* Send job document received event. */
        OTA_SignalEvent( &eventMsg );
    }
    else
    {
        LogError( ( "Error: No OTA data buffers available.\n" ) );
    }
}

/*-----------------------------------------------------------*/

static void prvRegisterOTACallback( const char * pTopicFilter,
                                    uint16_t topicFilterLength )
{
    bool isMatch = false;
    MQTTStatus_t mqttStatus = MQTTSuccess;
    uint16_t index = 0U;
    uint16_t numTopicFilters = sizeof( otaTopicFilterCallbacks ) / sizeof( OtaTopicFilterCallback_t );


    bool subscriptionAdded;

    ( void ) mqttStatus;

    /* Match the input topic filter against the wild-card pattern of topics filters
    * relevant for the OTA Update service to determine the type of topic filter. */
    for( ; index < numTopicFilters; index++ )
    {
        mqttStatus = MQTT_MatchTopic( pTopicFilter,
                                      topicFilterLength,
                                      otaTopicFilterCallbacks[ index ].pTopicFilter,
                                      otaTopicFilterCallbacks[ index ].topicFilterLength,
                                      &isMatch );
        assert( mqttStatus == MQTTSuccess );

        if( isMatch )
        {
            /* Add subscription so that incoming publishes are routed to the application callback. */
            subscriptionAdded = addSubscription( pTopicFilter,
                                                 topicFilterLength,
                                                 otaTopicFilterCallbacks[ index ].callback,
                                                 NULL );

            if( subscriptionAdded == false )
            {
                LogError( ( "Failed to register a publish callback for topic %.*s.",
                            pTopicFilter,
                            topicFilterLength ) );
            }
        }
    }
}

static void prvMQTTSubscribeCompleteCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                              MQTTAgentReturnInfo_t * pxReturnInfo )
{
    if( pxReturnInfo->returnCode == MQTTSuccess )
    {
        MQTTAgentSubscribeArgs_t * pSubscribeArgs = ( MQTTAgentSubscribeArgs_t * ) ( pxCommandContext->pArgs );
        prvRegisterOTACallback( pSubscribeArgs->pSubscribeInfo->pTopicFilter, pSubscribeArgs->pSubscribeInfo->topicFilterLength );
    }

    /* Store the result in the application defined context so the task that
     * initiated the publish can check the operation's status. */
    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

    if( pxCommandContext->xTaskToNotify != NULL )
    {
        /* Send the context's ulNotificationValue as the notification value so
         * the receiving task can check the value it set in the context matches
         * the value it receives in the notification. */
        xTaskNotify( pxCommandContext->xTaskToNotify, ( uint32_t ) ( pxReturnInfo->returnCode ), eSetValueWithOverwrite );
    }
}

static void prvMQTTUnsubscribeCompleteCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                                MQTTAgentReturnInfo_t * pxReturnInfo )
{
    /* Store the result in the application defined context so the task that
     * initiated the publish can check the operation's status. */
    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

    if( pxCommandContext->xTaskToNotify != NULL )
    {
        /* Send the context's ulNotificationValue as the notification value so
         * the receiving task can check the value it set in the context matches
         * the value it receives in the notification. */
        xTaskNotify( pxCommandContext->xTaskToNotify, ( uint32_t ) ( pxReturnInfo->returnCode ), eSetValueWithOverwrite );
    }
}

static OtaMqttStatus_t prvMQTTSubscribe( const char * pTopicFilter,
                                         uint16_t topicFilterLength,
                                         uint8_t ucQoS )
{
    MQTTStatus_t mqttStatus;
    uint32_t ulNotifiedValue;
    static MQTTAgentSubscribeArgs_t xSubscribeArgs = { 0 };
    static MQTTSubscribeInfo_t xSubscribeInfo = { 0 };
    BaseType_t result;
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };
    static MQTTAgentCommandContext_t xApplicationDefinedContext = { 0 };
    OtaMqttStatus_t otaRet = OtaMqttSuccess;

    configASSERT( pTopicFilter != NULL );
    configASSERT( topicFilterLength > 0 );

    xSubscribeInfo.pTopicFilter = pTopicFilter;
    xSubscribeInfo.topicFilterLength = topicFilterLength;
    xSubscribeInfo.qos = ucQoS;
    xSubscribeArgs.pSubscribeInfo = &xSubscribeInfo;
    xSubscribeArgs.numSubscriptions = 1;

    xApplicationDefinedContext.xTaskToNotify = xTaskGetCurrentTaskHandle();
    xApplicationDefinedContext.pArgs = &xSubscribeArgs;
    xApplicationDefinedContext.xReturnStatus = MQTTSendFailed;

    xCommandParams.blockTimeMs = otaexampleMQTT_TIMEOUT_MS;
    xCommandParams.cmdCompleteCallback = prvMQTTSubscribeCompleteCallback;
    xCommandParams.pCmdCompleteCallbackContext = ( void * ) &xApplicationDefinedContext;

    xTaskNotifyStateClear( NULL );

    mqttStatus = MQTTAgent_Subscribe( &xGlobalMqttAgentContext,
                                      &xSubscribeArgs,
                                      &xCommandParams );

    /* Wait for command to complete so MQTTSubscribeInfo_t remains in scope for the
     * duration of the command. */
    if( mqttStatus == MQTTSuccess )
    {
        result = xTaskNotifyWait( 0, otaexampleMAX_UINT32, &ulNotifiedValue, pdMS_TO_TICKS( otaexampleMQTT_TIMEOUT_MS ) );

        if( result == pdTRUE )
        {
            mqttStatus = xApplicationDefinedContext.xReturnStatus;
        }
        else
        {
            mqttStatus = MQTTRecvFailed;
        }
    }

    if( mqttStatus != MQTTSuccess )
    {
        LogError( ( "Failed to SUBSCRIBE to topic with error = %u.",
                    mqttStatus ) );
        otaRet = OtaMqttSubscribeFailed;
    }
    else
    {
        LogInfo( ( "Subscribed to topic %.*s.\n",
                   topicFilterLength,
                   pTopicFilter ) );
        otaRet = OtaMqttSuccess;
    }

    return otaRet;
}

static void prvOTAPublishCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                          MQTTAgentReturnInfo_t * pxReturnInfo )
{
    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

    if( pxCommandContext->xTaskToNotify != NULL )
    {
        xTaskNotify( pxCommandContext->xTaskToNotify, ( uint32_t ) ( pxReturnInfo->returnCode ), eSetValueWithOverwrite );
    }
}

static OtaMqttStatus_t prvMQTTPublish( const char * const pacTopic,
                                       uint16_t topicLen,
                                       const char * pMsg,
                                       uint32_t msgSize,
                                       uint8_t qos )
{
    BaseType_t result;
    MQTTStatus_t mqttStatus = MQTTBadParameter;
    static MQTTPublishInfo_t publishInfo = { 0 };
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };
    static MQTTAgentCommandContext_t xCommandContext = { 0 };
    OtaMqttStatus_t otaRet = OtaMqttSuccess;

    publishInfo.pTopicName = pacTopic;
    publishInfo.topicNameLength = topicLen;
    publishInfo.qos = qos;
    publishInfo.pPayload = pMsg;
    publishInfo.payloadLength = msgSize;

    xCommandContext.xTaskToNotify = xTaskGetCurrentTaskHandle();
    xTaskNotifyStateClear( NULL );

    xCommandParams.blockTimeMs = otaexampleMQTT_TIMEOUT_MS;
    xCommandParams.cmdCompleteCallback = prvOTAPublishCommandCallback;
    xCommandParams.pCmdCompleteCallbackContext = ( void * ) &xCommandContext;

    mqttStatus = MQTTAgent_Publish( &xGlobalMqttAgentContext,
                                    &publishInfo,
                                    &xCommandParams );

    /* Wait for command to complete so MQTTSubscribeInfo_t remains in scope for the
     * duration of the command. */
    if( mqttStatus == MQTTSuccess )
    {
        result = xTaskNotifyWait( 0, otaexampleMAX_UINT32, NULL, pdMS_TO_TICKS( otaexampleMQTT_TIMEOUT_MS ) );

        if( result != pdTRUE )
        {
            mqttStatus = MQTTSendFailed;
        }
        else
        {
            mqttStatus = xCommandContext.xReturnStatus;
        }
    }

    if( mqttStatus != MQTTSuccess )
    {
        LogError( ( "Failed to send PUBLISH packet to broker with error = %u.", mqttStatus ) );
        otaRet = OtaMqttPublishFailed;
    }
    else
    {
        LogInfo( ( "Sent PUBLISH packet to broker %.*s to broker.\n",
                   topicLen,
                   pacTopic ) );
        otaRet = OtaMqttSuccess;
    }

    return otaRet;
}

static OtaMqttStatus_t prvMQTTUnsubscribe( const char * pTopicFilter,
                                           uint16_t topicFilterLength,
                                           uint8_t ucQoS )
{
    MQTTStatus_t mqttStatus;
    uint32_t ulNotifiedValue;
    static MQTTAgentSubscribeArgs_t xSubscribeArgs = { 0 };
    static MQTTSubscribeInfo_t xSubscribeInfo = { 0 };
    BaseType_t result;
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };
    static MQTTAgentCommandContext_t xApplicationDefinedContext = { 0 };
    OtaMqttStatus_t otaRet = OtaMqttSuccess;

    configASSERT( pTopicFilter != NULL );
    configASSERT( topicFilterLength > 0 );

    xSubscribeInfo.pTopicFilter = pTopicFilter;
    xSubscribeInfo.topicFilterLength = topicFilterLength;
    xSubscribeInfo.qos = ucQoS;
    xSubscribeArgs.pSubscribeInfo = &xSubscribeInfo;
    xSubscribeArgs.numSubscriptions = 1;


    xApplicationDefinedContext.xTaskToNotify = xTaskGetCurrentTaskHandle();

    xCommandParams.blockTimeMs = otaexampleMQTT_TIMEOUT_MS;
    xCommandParams.cmdCompleteCallback = prvMQTTUnsubscribeCompleteCallback;
    xCommandParams.pCmdCompleteCallbackContext = ( void * ) &xApplicationDefinedContext;

    LogInfo( ( " Unsubscribing to topic filter: %s", pTopicFilter ) );
    xTaskNotifyStateClear( NULL );


    mqttStatus = MQTTAgent_Unsubscribe( &xGlobalMqttAgentContext,
                                        &xSubscribeArgs,
                                        &xCommandParams );

    /* Wait for command to complete so MQTTSubscribeInfo_t remains in scope for the
     * duration of the command. */
    if( mqttStatus == MQTTSuccess )
    {
        result = xTaskNotifyWait( 0, otaexampleMAX_UINT32, &ulNotifiedValue, pdMS_TO_TICKS( otaexampleMQTT_TIMEOUT_MS ) );

        if( result == pdTRUE )
        {
            mqttStatus = xApplicationDefinedContext.xReturnStatus;
        }
        else
        {
            mqttStatus = MQTTRecvFailed;
        }
    }

    if( mqttStatus != MQTTSuccess )
    {
        LogError( ( "Failed to UNSUBSCRIBE from topic %.*s with error = %u.",
                    topicFilterLength,
                    pTopicFilter,
                    mqttStatus ) );
        otaRet = OtaMqttUnsubscribeFailed;
    }
    else
    {
        LogInfo( ( "UNSUBSCRIBED from topic %.*s.\n",
                   topicFilterLength,
                   pTopicFilter ) );
        otaRet = OtaMqttSuccess;
    }

    return otaRet;
}

static void setOtaInterfaces( OtaInterfaces_t * pOtaInterfaces )
{
    configASSERT( pOtaInterfaces != NULL );

    /* Initialize OTA library OS Interface. */
    pOtaInterfaces->os.event.init = OtaInitEvent_FreeRTOS;
    pOtaInterfaces->os.event.send = OtaSendEvent_FreeRTOS;
    pOtaInterfaces->os.event.recv = OtaReceiveEvent_FreeRTOS;
    pOtaInterfaces->os.event.deinit = OtaDeinitEvent_FreeRTOS;
    pOtaInterfaces->os.timer.start = OtaStartTimer_FreeRTOS;
    pOtaInterfaces->os.timer.stop = OtaStopTimer_FreeRTOS;
    pOtaInterfaces->os.timer.delete = OtaDeleteTimer_FreeRTOS;
    pOtaInterfaces->os.mem.malloc = Malloc_FreeRTOS;
    pOtaInterfaces->os.mem.free = Free_FreeRTOS;

    /* Initialize the OTA library MQTT Interface.*/
    pOtaInterfaces->mqtt.subscribe = prvMQTTSubscribe;
    pOtaInterfaces->mqtt.publish = prvMQTTPublish;
    pOtaInterfaces->mqtt.unsubscribe = prvMQTTUnsubscribe;

    /* Initialize the OTA library PAL Interface.*/
    pOtaInterfaces->pal.getPlatformImageState = otaPal_GetPlatformImageState;
    pOtaInterfaces->pal.setPlatformImageState = otaPal_SetPlatformImageState;
    pOtaInterfaces->pal.writeBlock = otaPal_WriteBlock;
    pOtaInterfaces->pal.activate = otaPal_ActivateNewImage;
    pOtaInterfaces->pal.closeFile = otaPal_CloseFile;
    pOtaInterfaces->pal.reset = otaPal_ResetDevice;
    pOtaInterfaces->pal.abort = otaPal_Abort;
    pOtaInterfaces->pal.createFile = otaPal_CreateFileForRx;
}

/*-----------------------------------------------------------*/

static void prvOTAAgentTask( void * pParam )
{
    /* Calling OTA agent task. */
    OTA_EventProcessingTask( pParam );
    LogInfo( ( "OTA Agent stopped." ) );

    vTaskDelete( NULL );
}

static BaseType_t prvSuspendOTA( void )
{
    /* OTA library return status. */
    OtaErr_t otaRet = OtaErrNone;
    BaseType_t status = pdPASS;

    otaRet = OTA_Suspend();

    if( otaRet == OtaErrNone )
    {
        uint32_t suspendTimeout = OTA_SUSPEND_TIMEOUT_MS;

        while( ( OTA_GetState() != OtaAgentStateSuspended ) && ( suspendTimeout > 0 ) )
        {
            /* Wait for OTA Library state to suspend */
            vTaskDelay( pdMS_TO_TICKS( otaexampleTASK_DELAY_MS ) );
            suspendTimeout -= otaexampleTASK_DELAY_MS;
        }

        if( OTA_GetState() != OtaAgentStateSuspended )
        {
            LogError( ( "Failed to suspend OTA." ) );
            status = pdFAIL;
        }
    }
    else
    {
        LogError( ( "Error while trying to suspend OTA agent %d", otaRet ) );
        status = pdFAIL;
    }

    return status;
}

static BaseType_t prvResumeOTA( void )
{
    /* OTA library return status. */
    OtaErr_t otaRet = OtaErrNone;
    BaseType_t status = pdPASS;

    otaRet = OTA_Resume();

    if( otaRet == OtaErrNone )
    {
        uint32_t suspendTimeout = OTA_SUSPEND_TIMEOUT_MS;

        while( ( OTA_GetState() == OtaAgentStateSuspended ) && ( suspendTimeout > 0 ) )
        {
            /* Wait for OTA Library state to suspend */
            vTaskDelay( pdMS_TO_TICKS( otaexampleTASK_DELAY_MS ) );
            suspendTimeout -= otaexampleTASK_DELAY_MS;
        }

        if( OTA_GetState() == OtaAgentStateSuspended )
        {
            LogError( ( "Failed to resume OTA." ) );
            status = pdFAIL;
        }
    }
    else
    {
        LogError( ( "Error while trying to resume OTA agent %d", otaRet ) );
        status = pdFAIL;
    }

    return status;
}

static BaseType_t prvRunOTADemo( void )
{
    /* Status indicating a successful demo or not. */
    BaseType_t xStatus = pdPASS;

    /* OTA event message used for sending event to OTA Agent.*/
    OtaEventMsg_t eventMsg = { 0 };

    /* OTA interface context required for library interface functions.*/
    OtaInterfaces_t otaInterfaces;

    /* OTA library packet statistics per job.*/
    OtaAgentStatistics_t otaStatistics = { 0 };

    /* OTA Agent state returned from calling OTA_GetState.*/
    OtaState_t state;

    /* Set OTA Library interfaces.*/
    setOtaInterfaces( &otaInterfaces );

    vWaitUntilMQTTAgentReady();
    vWaitUntilMQTTAgentConnected();

    /****************************** Init OTA Library. ******************************/

    if( xStatus == pdPASS )
    {
        /* OTA library return status. */
        OtaErr_t otaRet;

        if( ( otaRet = OTA_Init( &otaBuffer,
                                 &otaInterfaces,
                                 ( const uint8_t * ) ( democonfigCLIENT_IDENTIFIER ),
                                 otaAppCallback ) ) != OtaErrNone )
        {
            LogError( ( "Failed to initialize OTA Agent, exiting = %u.",
                        otaRet ) );

            xStatus = pdFAIL;
        }
    }

    /****************************** Create OTA Agent Task. ******************************/

    if( xStatus == pdPASS )
    {
        xStatus = xTaskCreate( prvOTAAgentTask,
                               "OTA Agent Task",
                               OTA_AGENT_TASK_STACK_SIZE,
                               NULL,
                               OTA_AGENT_TASK_PRIORITY,
                               NULL );

        if( xStatus != pdPASS )
        {
            LogError( ( "Failed to create OTA agent task:" ) );
        }
    }

    /**
     * Register a callback for receiving messages intended for OTA agent from broker,
     * for which the topic has not been subscribed for.
     */
    prvRegisterOTACallback( OTA_DEFAULT_TOPIC_FILTER, OTA_DEFAULT_TOPIC_FILTER_LENGTH );

    /****************************** Start OTA ******************************/

    if( xStatus == pdPASS )
    {
        /* Send start event to OTA Agent.*/
        eventMsg.eventId = OtaAgentEventStart;
        OTA_SignalEvent( &eventMsg );

        /* Loop and display OTA statistics */
        while( ( state = OTA_GetState() ) != OtaAgentStateStopped )
        {
            /* Get OTA statistics for currently executing job. */
            if( state != OtaAgentStateSuspended )
            {
                OTA_GetStatistics( &otaStatistics );

                LogInfo( ( " Received: %u   Queued: %u   Processed: %u   Dropped: %u",
                           otaStatistics.otaPacketsReceived,
                           otaStatistics.otaPacketsQueued,
                           otaStatistics.otaPacketsProcessed,
                           otaStatistics.otaPacketsDropped ) );
            }

            if( !xIsMqttAgentConnected() )
            {
                xStatus = prvSuspendOTA();
                configASSERT( xStatus == pdPASS );

                LogInfo( ( "Suspended OTA agent." ) );
            }
            else
            {
                if( OTA_GetState() == OtaAgentStateSuspended )
                {
                    xStatus = prvResumeOTA();
                    configASSERT( xStatus == pdPASS );

                    LogInfo( ( "Resumed OTA agent." ) );
                }
            }

            vTaskDelay( pdMS_TO_TICKS( otaexampleTASK_DELAY_MS ) );
        }
    }

    /**
     * Remove callback for receiving messages intended for OTA agent from broker,
     * for which the topic has not been subscribed for.
     */
    removeSubscription( OTA_DEFAULT_TOPIC_FILTER,
                        OTA_DEFAULT_TOPIC_FILTER_LENGTH );

    return xStatus;
}

/**
 * @brief Entry point of Ota demo task.
 *
 * This example initializes the OTA library to enable OTA updates via the
 * MQTT broker. It simply connects to the MQTT broker with the users
 * credentials and spins in an indefinite loop to allow MQTT messages to be
 * forwarded to the OTA agent for possible processing. The OTA agent does all
 * of the real work; checking to see if the message topic is one destined for
 * the OTA agent. If not, it is simply ignored.
 *
 */
static void vOtaDemoTask( void * pvParam )
{
    ( void ) pvParam;

    if( GetImageVersionPSA( FWU_COMPONENT_ID_NONSECURE ) == 0 )
    {
        LogInfo( ( "OTA over MQTT, Application version from appFirmwareVersion %u.%u.%u\n",
                   appFirmwareVersion.u.x.major,
                   appFirmwareVersion.u.x.minor,
                   appFirmwareVersion.u.x.build ) );
    }
    else
    {
        LogError( ( "OTA over MQTT, unable to get application versions" ) );
    }

    /* Initialize semaphore for buffer operations. */
    xBufferSemaphore = xSemaphoreCreateMutex();

    if( xBufferSemaphore == NULL )
    {
        LogError( ( "Failed to initialize buffer semaphore." ) );
    }
    else
    {
        /****************************** Start OTA Demo. ******************************/

        /* Start OTA demo. The function returns only if OTA completes successfully and a
         * shutdown of OTA is triggered for a manual restart of the device. */
        if( prvRunOTADemo() != pdPASS )
        {
            LogError( ( "Failed to complete OTA successfully." ) );
        }

        /* / ****************************** Cleanup ****************************** / */

        /* Cleanup semaphore created for buffer operations. */
        vSemaphoreDelete( xBufferSemaphore );
    }
}

/*
 * @brief Create the task that demonstrates the Ota demo.
 */
void vStartOtaTask( void )
{
    xTaskCreate( vOtaDemoTask,                             /* Function that implements the task. */
                 "OTA Task ",                              /* Text name for the task - only used for debugging. */
                 appCONFIG_OTA_MQTT_AGENT_TASK_STACK_SIZE, /* Size of stack (in words, not bytes) to allocate for the task. */
                 NULL,                                     /* Optional - task parameter - not used in this case. */
                 appCONFIG_OTA_MQTT_AGENT_TASK_PRIORITY,   /* Task priority, must be between 0 and configMAX_PRIORITIES - 1. */
                 NULL );                                   /* Optional - used to pass out a handle to the created task. */
}
