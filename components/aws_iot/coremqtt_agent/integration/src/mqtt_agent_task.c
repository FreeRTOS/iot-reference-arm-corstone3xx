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

#include "mqtt_agent_task.h"

/* MQTT agent configuration. */
#include "app_config.h"

/* PKCS#11 includes. */
#include "core_pkcs11_config.h"

/* includes for TFM */
#include "psa/crypto.h"
#include "psa/error.h"

/* Demo config includes. */
#include "demo_config.h"
#include "iot_default_root_certificates.h"

/* MQTT library includes. */
#include "core_mqtt_agent.h"

/* MQTT Agent ports. */
#include "freertos_agent_message.h"
#include "freertos_command_pool.h"

/* Transport interface header file. */
#include "transport_interface_api.h"

/* Subscription manager header include. */
#include "subscription_manager.h"

/* Exponential backoff retry include. */
#include "backoff_algorithm.h"

/* System events header. */
#include "events.h"

/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "MQTT Agent Task"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

/**
 * @brief Dimensions the buffer used to serialize and deserialize MQTT packets.
 * @note Specified in bytes.  Must be large enough to hold the maximum
 * anticipated MQTT payload.
 */
#if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 )
    #define MQTT_AGENT_NETWORK_BUFFER_SIZE    ( 20480 )
#else
    #define MQTT_AGENT_NETWORK_BUFFER_SIZE    ( 10240 )
#endif

/**
 * @brief The maximum amount of time in milliseconds to wait for the commands
 * to be posted to the MQTT agent should the MQTT agent's command queue be full.
 * Tasks wait in the Blocked state, so don't use any CPU time.
 */
#define MQTT_AGENT_SEND_BLOCK_TIME_MS             ( 200U )

/**
 * @brief This demo uses task notifications to signal tasks from MQTT callback
 * functions.  mqttexampleMS_TO_WAIT_FOR_NOTIFICATION defines the time, in ticks,
 * to wait for such a callback.
 */
#define MQTT_AGENT_MS_TO_WAIT_FOR_NOTIFICATION    ( 5000U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying failed operation
 *  with server.
 */
#if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 )
    #define RETRY_MAX_BACKOFF_DELAY_MS    ( 10000U )
#else
    #define RETRY_MAX_BACKOFF_DELAY_MS    ( 20000U )
#endif

/**
 * @brief The base back-off delay (in milliseconds) to use for network operation retry
 * attempts.
 */
#define RETRY_BACKOFF_BASE_MS                        ( 10U )

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 *  between two Control Packets.
 *
 *  It is the responsibility of the Client to ensure that the interval between
 *  Control Packets being sent does not exceed the this Keep Alive value. In the
 *  absence of sending any other Control Packets, the Client MUST send a
 *  PINGREQ Packet.
 */
#define MQTT_AGENT_KEEP_ALIVE_INTERVAL_SECONDS       ( 60U )

/**
 * @brief Socket send and receive timeouts to use.  Specified in milliseconds.
 */
#define MQTT_AGENT_TRANSPORT_SEND_RECV_TIMEOUT_MS    ( 750 )

/**
 * @brief Timeout for receiving CONNACK after sending an MQTT CONNECT packet.
 * Defined in milliseconds.
 */
#define MQTT_AGENT_CONNACK_RECV_TIMEOUT_MS           ( 1000U )

/*-----------------------------------------------------------*/

/**
 * @brief Global entry time into the application to use as a reference timestamp
 * in the #prvGetTimeMs function. #prvGetTimeMs will always return the difference
 * between the current time and the global entry time. This will reduce the chances
 * of overflow for the 32 bit unsigned integer used for holding the timestamp.
 */
static uint32_t ulGlobalEntryTimeMs;

/**
 * @brief The buffer is used to hold the serialized packets for transmission to and from
 * the transport interface.
 */
static uint8_t xNetworkBuffer[ MQTT_AGENT_NETWORK_BUFFER_SIZE ];

/**
 * @brief FreeRTOS blocking queue to be used as MQTT Agent context.
 */
static MQTTAgentMessageContext_t xCommandQueue;

/**
 * @brief The network context used by the MQTT library transport interface.
 * See https://www.freertos.org/network-interface.html
 */
static NetworkContext_t xNetworkContextMqtt;

/**
 * @brief MQTT CONNECT packet parameters.
 */
static MQTTConnectInfo_t xConnectInfo = { 0 };

/*-----------------------------------------------------------*/

/**
 * @brief Static handle used for MQTT agent context.
 */
MQTTAgentContext_t xGlobalMqttAgentContext;

/*-----------------------------------------------------------*/

/**
 * @brief The global array of subscription elements.
 *
 * @note No thread safety is required to this array, since the updates the array
 * elements are done only from one task at a time. The subscription manager
 * implementation expects that the array of the subscription elements used for
 * storing subscriptions to be initialized to 0. As this is a global array, it
 * will be intialized to 0 by default.
 */
extern SubscriptionElement_t xGlobalSubscriptionList[ SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS ];

/*-----------------------------------------------------------*/

/**
 * @brief Task for MQTT agent.
 * Task runs MQTT agent command loop, which returns only when the user disconnects
 * MQTT, terminates agent, or the mqtt connection is broken. If the mqtt connection is broken, the task
 * tries to reconnect to the broker.
 *
 * @param[in] pParam Can be used to pass down functionality to the agent task
 */
static void prvMQTTAgentTask( void * pParam );

/**
 * @brief Retry logic to establish a connection to the MQTT broker.
 *
 * If the connection fails, keep retrying with exponentially increasing
 * timeout value, until max retries, max timeout or successful connect.
 *
 * @param[in] pNetworkContext Network context to connect on.
 * @return int pdFALSE if connection failed after retries.
 */
static BaseType_t prvSocketConnect( NetworkContext_t * pNetworkContext );

/**
 * @brief Disconnects from the MQTT broker.
 * Initiates an MQTT disconnect and then teardown underlying TCP connection.
 *
 */
static void prvDisconnectFromMQTTBroker( void );

/**
 * @brief Initializes an MQTT context, including transport interface and
 * network buffer.
 *
 * @return `MQTTSuccess` if the initialization succeeds, else `MQTTBadParameter`.
 */
static MQTTStatus_t prvMqttInit( void );

/**
 * @brief Sends an MQTT Connect packet over the already connected TCP socket.
 *
 * @param[in] pxMQTTContext MQTT context pointer.
 * @param[in] xCleanSession If a clean session should be established.
 *
 * @return `MQTTSuccess` if connection succeeds, else appropriate error code
 * from MQTT_Connect.
 */
static MQTTStatus_t prvMQTTConnect( void );

/*-----------------------------------------------------------*/

static uint32_t prvGetTimeMs( void )
{
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;

    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();

    /* Convert the ticks to milliseconds. */
    ulTimeMs = ( uint32_t ) TICKS_TO_pdMS( xTickCount );

    /* Reduce ulGlobalEntryTimeMs from obtained time so as to always return the
     * elapsed time in the application. */
    ulTimeMs = ( uint32_t ) ( ulTimeMs - ulGlobalEntryTimeMs );

    return ulTimeMs;
}

static UBaseType_t prvGetRandomNumber( void )
{
    psa_status_t xPsaStatus = PSA_ERROR_PROGRAMMER_ERROR;
    UBaseType_t uxRandomValue = 0U;

    xPsaStatus = psa_generate_random( ( uint8_t * ) ( &uxRandomValue ), sizeof( UBaseType_t ) );

    if( xPsaStatus != PSA_SUCCESS )
    {
        LogError( ( "psa_generate_random failed with %d.", xPsaStatus ) );
        LogError( ( "Using xTaskGetTickCount() as random number generator" ) );
    }
    else
    {
        uxRandomValue = xTaskGetTickCount();
    }

    return uxRandomValue;
}

static BaseType_t prvSocketConnect( NetworkContext_t * pxNetworkContext )
{
    BaseType_t xConnected = pdFAIL;

    TransportStatus_t xNetworkStatus = TRANSPORT_STATUS_CONNECT_FAILURE;
    TLSParams_t xTLSParams = { 0 };
    ServerInfo_t xServerInfo = { 0 };

    #ifdef democonfigUSE_AWS_IOT_CORE_BROKER

    /* ALPN protocols must be a NULL-terminated list of strings. Therefore,
     * the first entry will contain the actual ALPN protocol string while the
     * second entry must remain NULL. */
    const char * pcAlpnProtocols[] = { NULL, NULL };

    /* The ALPN string changes depending on whether username/password authentication is used. */
    #ifdef democonfigCLIENT_USERNAME
        pcAlpnProtocols[ 0 ] = AWS_IOT_CUSTOM_AUTH_ALPN;
    #else
        pcAlpnProtocols[ 0 ] = AWS_IOT_MQTT_ALPN;
    #endif
    xTLSParams.pAlpnProtos = pcAlpnProtocols;
    #endif /* ifdef democonfigUSE_AWS_IOT_CORE_BROKER */


    /* Initializer server information. */
    xServerInfo.pHostName = democonfigMQTT_BROKER_ENDPOINT;
    xServerInfo.hostNameLength = strlen( democonfigMQTT_BROKER_ENDPOINT );
    xServerInfo.port = democonfigMQTT_BROKER_PORT;

    /* Set the credentials for establishing a TLS connection. */
    xTLSParams.pRootCa = tlsATS1_ROOT_CERTIFICATE_PEM;
    xTLSParams.rootCaSize = tlsATS1_ROOT_CERTIFICATE_LENGTH;
    xTLSParams.pClientCertLabel = pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS;
    xTLSParams.pPrivateKeyLabel = pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS;
    xTLSParams.disableSni = false;
    xTLSParams.pLoginPIN = configPKCS11_DEFAULT_USER_PIN;

    /* Establish a TCP connection with the MQTT broker. This example connects to
     * the MQTT broker as specified in democonfigMQTT_BROKER_ENDPOINT and
     * democonfigMQTT_BROKER_PORT */
    LogInfo( ( "Creating a TLS connection to %s:%d.",
               democonfigMQTT_BROKER_ENDPOINT,
               democonfigMQTT_BROKER_PORT ) );

    xNetworkStatus = Transport_Connect( pxNetworkContext,
                                        &xServerInfo,
                                        &xTLSParams,
                                        MQTT_AGENT_TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                        MQTT_AGENT_TRANSPORT_SEND_RECV_TIMEOUT_MS );

    xConnected = ( xNetworkStatus == TRANSPORT_STATUS_SUCCESS ) ? pdPASS : pdFAIL;

    if( xConnected )
    {
        LogInfo( ( "Successfully created a TLS connection to %s:%d.",
                   democonfigMQTT_BROKER_ENDPOINT,
                   democonfigMQTT_BROKER_PORT ) );
    }

    return xConnected;
}

static BaseType_t prvSocketDisconnect( NetworkContext_t * pxNetworkContext )
{
    BaseType_t xDisconnected = pdFAIL;

    LogInfo( ( "Disconnecting TLS connection.\n" ) );
    Transport_Disconnect( pxNetworkContext );
    xDisconnected = pdPASS;

    ( void ) xEventGroupClearBits( xSystemEvents, EVENT_MASK_MQTT_CONNECTED );

    return xDisconnected;
}

static void prvIncomingPublishCallback( MQTTAgentContext_t * pMqttAgentContext,
                                        uint16_t packetId,
                                        MQTTPublishInfo_t * pxPublishInfo )
{
    bool xPublishHandled = false;

    ( void ) packetId;

    /* Fan out the incoming publishes to the callbacks registered using
     * subscription manager. */
    xPublishHandled = handleIncomingPublishes( pxPublishInfo );

    /* If there are no callbacks to handle the incoming publishes,
     * handle it as an unsolicited publish. */
    if( xPublishHandled != true )
    {
        /* Ensure the topic string is terminated for printing.  This will over-
         * write the message ID, which is restored afterwards. */
        char * pcLocation = ( char * ) &( pxPublishInfo->pTopicName[ pxPublishInfo->topicNameLength ] );
        char cOriginalChar = *pcLocation;
        *pcLocation = 0x00;
        LogWarn( ( "Received an unsolicited publish from topic %s", pxPublishInfo->pTopicName ) );
        *pcLocation = cOriginalChar;
    }
}

static void prvReSubscriptionCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                              MQTTAgentReturnInfo_t * pxReturnInfo )
{
    MQTTAgentSubscribeArgs_t * pxSubscribeArgs = ( MQTTAgentSubscribeArgs_t * ) pxCommandContext;

    /* If the return code is success, no further action is required as all the topic filters
     * are already part of the subscription list. */
    if( pxReturnInfo->returnCode != MQTTSuccess )
    {
        size_t xIndex;

        /* Check through each of the suback codes and determine if there are any failures. */
        for( xIndex = 0; xIndex < pxSubscribeArgs->numSubscriptions; xIndex++ )
        {
            /* This demo doesn't attempt to resubscribe in the event that a SUBACK failed. */
            if( pxReturnInfo->pSubackCodes[ xIndex ] == MQTTSubAckFailure )
            {
                LogError( ( "Failed to resubscribe to topic %.*s.",
                            pxSubscribeArgs->pSubscribeInfo[ xIndex ].topicFilterLength,
                            pxSubscribeArgs->pSubscribeInfo[ xIndex ].pTopicFilter ) );
                /* Remove subscription callback for unsubscribe. */
                removeSubscription( pxSubscribeArgs->pSubscribeInfo[ xIndex ].pTopicFilter,
                                    pxSubscribeArgs->pSubscribeInfo[ xIndex ].topicFilterLength );
            }
        }

        /* Hit an assert as some of the tasks won't be able to proceed correctly without
         * the subscriptions. This logic will be updated with exponential backoff and retry.  */
        configASSERT( pdTRUE );
    }
}

static MQTTStatus_t prvMQTTInit( void )
{
    TransportInterface_t xTransport = { 0 };
    MQTTStatus_t xReturn;
    MQTTFixedBuffer_t xFixedBuffer = { .pBuffer = xNetworkBuffer, .size = MQTT_AGENT_NETWORK_BUFFER_SIZE };
    static uint8_t staticQueueStorageArea[ MQTT_AGENT_COMMAND_QUEUE_LENGTH * sizeof( MQTTAgentCommand_t * ) ];
    static StaticQueue_t staticQueueStructure;
    MQTTAgentMessageInterface_t messageInterface =
    {
        .pMsgCtx        = NULL,
        .send           = Agent_MessageSend,
        .recv           = Agent_MessageReceive,
        .getCommand     = Agent_GetCommand,
        .releaseCommand = Agent_ReleaseCommand
    };

    LogDebug( ( "Creating command queue." ) );
    xCommandQueue.queue = xQueueCreateStatic( MQTT_AGENT_COMMAND_QUEUE_LENGTH,
                                              sizeof( MQTTAgentCommand_t * ),
                                              staticQueueStorageArea,
                                              &staticQueueStructure );
    configASSERT( xCommandQueue.queue );
    messageInterface.pMsgCtx = &xCommandQueue;

    /* Initialize the task pool. */
    Agent_InitializePool();

    /* Fill in Transport Interface send and receive function pointers. */
    xTransport.pNetworkContext = &xNetworkContextMqtt;
    xTransport.send = Transport_Send;
    xTransport.recv = Transport_Recv;

    /* Initialize MQTT library. */
    xReturn = MQTTAgent_Init( &xGlobalMqttAgentContext,
                              &messageInterface,
                              &xFixedBuffer,
                              &xTransport,
                              prvGetTimeMs,
                              prvIncomingPublishCallback,
                              NULL );

    if( xReturn != MQTTSuccess )
    {
        LogError( ( "MQTTAgent_Init failed." ) );
    }
    else
    {
        ( void ) xEventGroupSetBits( xSystemEvents, EVENT_MASK_MQTT_INIT );
    }

    return xReturn;
}

static MQTTStatus_t prvHandleResubscribe( void )
{
    MQTTStatus_t xResult = MQTTBadParameter;
    uint32_t ulIndex = 0U;
    uint16_t usNumSubscriptions = 0U;

    /* These variables need to stay in scope until command completes. */
    static MQTTAgentSubscribeArgs_t xSubArgs = { 0 };
    static MQTTSubscribeInfo_t xSubInfo[ SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS ] = { 0 };
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };

    /* Loop through each subscription in the subscription list and add a subscribe
     * command to the command queue. */
    for( ulIndex = 0U; ulIndex < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS; ulIndex++ )
    {
        /* Check if there is a subscription in the subscription list. This demo
         * doesn't check for duplicate subscriptions. */
        if( xGlobalSubscriptionList[ ulIndex ].usFilterStringLength != 0 )
        {
            xSubInfo[ usNumSubscriptions ].pTopicFilter = xGlobalSubscriptionList[ ulIndex ].pcSubscriptionFilterString;
            xSubInfo[ usNumSubscriptions ].topicFilterLength = xGlobalSubscriptionList[ ulIndex ].usFilterStringLength;

            /* QoS1 is used for all the subscriptions in this demo. */
            xSubInfo[ usNumSubscriptions ].qos = MQTTQoS1;

            LogInfo( ( "Resubscribe to the topic %.*s will be attempted.",
                       xSubInfo[ usNumSubscriptions ].topicFilterLength,
                       xSubInfo[ usNumSubscriptions ].pTopicFilter ) );

            usNumSubscriptions++;
        }
    }

    if( usNumSubscriptions > 0U )
    {
        xSubArgs.pSubscribeInfo = xSubInfo;
        xSubArgs.numSubscriptions = usNumSubscriptions;

        /* The block time can be 0 as the command loop is not running at this point. */
        xCommandParams.blockTimeMs = 0U;
        xCommandParams.cmdCompleteCallback = prvReSubscriptionCommandCallback;
        xCommandParams.pCmdCompleteCallbackContext = ( void * ) &xSubArgs;

        /* Enqueue subscribe to the command queue. These commands will be processed only
         * when command loop starts. */
        xResult = MQTTAgent_Subscribe( &xGlobalMqttAgentContext, &xSubArgs, &xCommandParams );
    }
    else
    {
        /* Mark the resubscribe as success if there is nothing to be subscribed. */
        xResult = MQTTSuccess;
    }

    if( xResult != MQTTSuccess )
    {
        LogError( ( "Failed to enqueue the MQTT subscribe command. xResult=%s.",
                    MQTT_Status_strerror( xResult ) ) );
    }

    return xResult;
}

static MQTTStatus_t prvMQTTConnect( void )
{
    MQTTStatus_t xResult;
    bool xSessionPresent = false;

    /* The client identifier is used to uniquely identify this MQTT client to
     * the MQTT broker. In a production device the identifier can be something
     * unique, such as a device serial number. */
    xConnectInfo.pClientIdentifier = democonfigCLIENT_IDENTIFIER;
    xConnectInfo.clientIdentifierLength = ( uint16_t ) strlen( democonfigCLIENT_IDENTIFIER );

    /* Set MQTT keep-alive period. It is the responsibility of the application
     * to ensure that the interval between Control Packets being sent does not
     * exceed the Keep Alive value. In the absence of sending any other Control
     * Packets, the Client MUST send a PINGREQ Packet.  This responsibility will
     * be moved inside the agent. */
    xConnectInfo.keepAliveSeconds = MQTT_AGENT_KEEP_ALIVE_INTERVAL_SECONDS;

    LogInfo( ( "Creating an MQTT connection to the broker. \n" ) );

    ( void ) MQTTAgent_CancelAll( &( xGlobalMqttAgentContext ) );

    /* Send MQTT CONNECT packet to broker. MQTT's Last Will and Testament feature
     * is not used in this demo, so it is passed as NULL. */
    xResult = MQTT_Connect( &( xGlobalMqttAgentContext.mqttContext ),
                            &xConnectInfo,
                            NULL,
                            MQTT_AGENT_CONNACK_RECV_TIMEOUT_MS,
                            &xSessionPresent );

    /* Resume a session if desired. */
    if( ( xResult == MQTTSuccess ) &&
        ( xConnectInfo.cleanSession == false ) )
    {
        LogInfo( ( "Resuming persistent MQTT Session." ) );
        xResult = MQTTAgent_ResumeSession( &xGlobalMqttAgentContext, xSessionPresent );

        /* Resubscribe to all the subscribed topics. */
        if( ( xResult == MQTTSuccess ) && ( xSessionPresent == false ) )
        {
            xResult = prvHandleResubscribe();
        }
    }
    else if( xResult == MQTTSuccess )
    {
        LogInfo( ( "Successfully connected to the MQTT broker." ) );
        LogInfo( ( "Session present: %d\n", xSessionPresent ) );
        LogInfo( ( "Starting a clean MQTT Session." ) );
        /* Further reconnects will include a session resume operation */
        xConnectInfo.cleanSession = false;

        ( void ) xEventGroupSetBits( xSystemEvents, EVENT_MASK_MQTT_CONNECTED );
    }
    else
    {
        LogError( ( "Failed to connect to the MQTT broker." ) );
    }

    return xResult;
}

static void prvDisconnectCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                          MQTTAgentReturnInfo_t * pxReturnInfo )
{
    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

    if( pxCommandContext->xTaskToNotify != NULL )
    {
        xTaskNotify( pxCommandContext->xTaskToNotify, ( uint32_t ) ( pxReturnInfo->returnCode ), eSetValueWithOverwrite );
    }
}

static void prvDisconnectFromMQTTBroker( void )
{
    static MQTTAgentCommandContext_t xCommandContext = { 0 };
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };
    MQTTStatus_t xCommandStatus;

    /* Disconnect from broker. */
    LogInfo( ( "Disconnecting the MQTT connection with %s.", democonfigMQTT_BROKER_ENDPOINT ) );

    xCommandParams.blockTimeMs = MQTT_AGENT_SEND_BLOCK_TIME_MS;
    xCommandParams.cmdCompleteCallback = prvDisconnectCommandCallback;
    xCommandParams.pCmdCompleteCallbackContext = &xCommandContext;
    xCommandContext.xTaskToNotify = xTaskGetCurrentTaskHandle();
    xCommandContext.pArgs = NULL;
    xCommandContext.xReturnStatus = MQTTSendFailed;

    /* Disconnect MQTT session. */
    xCommandStatus = MQTTAgent_Disconnect( &xGlobalMqttAgentContext, &xCommandParams );
    configASSERT( xCommandStatus == MQTTSuccess );

    xTaskNotifyWait( 0,
                     0,
                     NULL,
                     pdMS_TO_TICKS( MQTT_AGENT_MS_TO_WAIT_FOR_NOTIFICATION ) );

    /* End TLS session, then close TCP connection. */
    prvSocketDisconnect( &xNetworkContextMqtt );
}

static void prvMQTTAgentTask( void * pParam )
{
    BaseType_t xResult;
    MQTTStatus_t xMQTTStatus = MQTTSuccess;
    BackoffAlgorithmStatus_t xBackoffAlgStatus;
    BackoffAlgorithmContext_t xReconnectParams = { 0 };
    uint16_t usNextRetryBackOff = 0U;

    ( void ) pParam;

    vWaitUntilNetworkIsUp();

    /* Initialize the MQTT context with the buffer and transport interface. */
    xMQTTStatus = prvMQTTInit();

    if( xMQTTStatus != MQTTSuccess )
    {
        LogError( ( "MQTT agent init failed." ) );
        configASSERT( 0 );
    }

    /* We will use a retry mechanism with an exponential backoff mechanism and
     * jitter.  That is done to prevent a fleet of IoT devices all trying to
     * reconnect at exactly the same time should they become disconnected at
     * the same time. We initialize reconnect attempts and interval here. */
    BackoffAlgorithm_InitializeParams( &xReconnectParams,
                                       RETRY_BACKOFF_BASE_MS,
                                       RETRY_MAX_BACKOFF_DELAY_MS,
                                       BACKOFF_ALGORITHM_RETRY_FOREVER );

    while( true )
    {
        /* Connect a TCP socket to the broker. */
        xResult = prvSocketConnect( &xNetworkContextMqtt );

        if( xResult != pdPASS )
        {
            xBackoffAlgStatus = BackoffAlgorithm_GetNextBackoff( &xReconnectParams, prvGetRandomNumber(), &usNextRetryBackOff );

            if( xBackoffAlgStatus == BackoffAlgorithmSuccess )
            {
                #if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 )
                    usNextRetryBackOff += 8000U;
                #endif

                LogWarn( ( "TLS connection to the broker failed. "
                           "Retrying connection in %hu ms.",
                           usNextRetryBackOff ) );
                vTaskDelay( pdMS_TO_TICKS( usNextRetryBackOff ) );
            }
            else
            {
                LogError( ( "BackoffAlgorithm_GetNextBackoff failed with %d. ",
                            usNextRetryBackOff ) );
            }

            continue;
        }

        /* Start with a clean session i.e. direct the MQTT broker to discard any
         * previous session data. Also, establishing a connection with clean session
         * will ensure that the broker does not store any data when this client
         * gets disconnected. */
        xConnectInfo.cleanSession = true;

        /* Form an MQTT connection without a persistent session. */
        xMQTTStatus = prvMQTTConnect();

        if( xMQTTStatus != MQTTSuccess )
        {
            /* End TLS session, then close TCP connection. */
            prvSocketDisconnect( &xNetworkContextMqtt );

            xBackoffAlgStatus = BackoffAlgorithm_GetNextBackoff( &xReconnectParams, prvGetRandomNumber(), &usNextRetryBackOff );

            if( xBackoffAlgStatus == BackoffAlgorithmSuccess )
            {
                #if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 )
                    usNextRetryBackOff += 8000U;
                #endif

                LogWarn( ( "Connection to the MQTT broker failed. "
                           "Retrying connection in %hu ms.",
                           usNextRetryBackOff ) );
                vTaskDelay( pdMS_TO_TICKS( usNextRetryBackOff ) );
            }
            else
            {
                LogError( ( "BackoffAlgorithm_GetNextBackoff failed with %d. ",
                            usNextRetryBackOff ) );
            }

            continue;
        }

        /* MQTTAgent_CommandLoop() is effectively the agent implementation.  It
         * will manage the MQTT protocol until such time that an error occurs,
         * which could be a disconnect.  If an error occurs the MQTT context on
         * which the error happened is returned so there can be an attempt to
         * clean up and reconnect however the application writer prefers. */
        xMQTTStatus = MQTTAgent_CommandLoop( &xGlobalMqttAgentContext );

        ( void ) xEventGroupClearBits( xSystemEvents, EVENT_MASK_MQTT_CONNECTED );


        LogError( ( "MQTTAgent_CommandLoop returned with status: %s.",
                    MQTT_Status_strerror( xMQTTStatus ) ) );

        ( void ) MQTTAgent_CancelAll( &( xGlobalMqttAgentContext ) );

        /* Success is returned for application initiated disconnect or termination. The socket will also be disconnected by the caller. */
        if( xMQTTStatus == MQTTSuccess )
        {
            break;
        }

        /* End TLS session, then close TCP connection. */
        prvSocketDisconnect( &xNetworkContextMqtt );
    }

    ( void ) xEventGroupClearBits( xSystemEvents, EVENT_MASK_MQTT_INIT | EVENT_MASK_MQTT_CONNECTED );

    LogError( ( "Terminating MqttAgentTask." ) );

    vTaskDelete( NULL );
}

/*-----------------------------------------------------------*/

/*
 * @brief Create MQTT agent task.
 */
void vStartMqttAgentTask( void )
{
    xTaskCreate( prvMQTTAgentTask,
                 "MQTT Agent Task ",
                 appCONFIG_MQTT_AGENT_TASK_STACK_SIZE,
                 NULL,
                 appCONFIG_MQTT_AGENT_TASK_PRIORITY,
                 NULL );
}

/*-----------------------------------------------------------*/
