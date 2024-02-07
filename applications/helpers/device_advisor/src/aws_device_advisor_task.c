/*
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2023-2024 Arm Limited and/or its affiliates
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
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 */

#include <string.h>

#include "mqtt_agent_task.h"
#include "app_config.h"
#include "aws_device_advisor_task.h"
#include "core_mqtt_agent.h"
#include "subscription_manager.h"
#include "events.h"

#if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 )

/* Configure name and log level. */
    #ifndef LIBRARY_LOG_NAME
        #define LIBRARY_LOG_NAME     "AWS DEVICE ADVISOR"
    #endif
    #ifndef LIBRARY_LOG_LEVEL
        #define LIBRARY_LOG_LEVEL    LOG_INFO
    #endif
    #include "logging_stack.h"

/**
 *
 * @param[in] xQoS The quality of service (QoS) to use.  Can be zero or one
 * for all MQTT brokers.  Can also be QoS2 if supported by the broker.  AWS IoT
 * does not support QoS2.
 * @param[in] pcTopicFilter Pointer to the topic filter string to subscribe for.
 * @param[in] xTopicFilterLength Length of the topic filter string.
 */
    static MQTTStatus_t prvSubscribeToTopic( MQTTQoS_t xQoS,
                                             char * pcTopicFilter,
                                             size_t xTopicFilterLength );

/**
 * @brief Publishes the given payload using the given qos to the topic provided.
 *
 * Function queues a publish command with the MQTT agent and waits for response. For
 * Qos0 publishes command is successful when the message is sent out of network. For Qos1
 * publishes, the command succeeds once a puback is received. If publish is unsuccessful, the function
 * retries the publish for a configure number of tries.
 *
 * @param[in] xQoS The quality of service (QoS) to use.  Can be zero or one
 * for all MQTT brokers.  Can also be QoS2 if supported by the broker.  AWS IoT
 * does not support QoS2.
 * @param[in] pcTopic NULL terminated topic string to which message is published.
 * @param[in] xTopicLength Length of the topic string.
 * @param[in] pucPayload The payload blob to be published.
 * @param[in] xPayloadLength Length of the payload blob to be published.
 * @param[in] lNumTries Number of retries if the QoS1 publish is not acknowledged.
 */
    static MQTTStatus_t prvPublishToTopic( MQTTQoS_t xQoS,
                                           char * pcTopic,
                                           size_t xTopicLength,
                                           uint8_t * pucPayload,
                                           size_t xPayloadLength,
                                           int32_t lNumRetries );

/**
 * @brief Passed into MQTTAgent_Subscribe() as the callback to execute when the
 * broker ACKs the SUBSCRIBE message.  Its implementation sends a notification
 * to the task that called MQTTAgent_Subscribe() to let the task know the
 * SUBSCRIBE operation completed.  It also sets the xReturnStatus of the
 * structure passed in as the command's context to the value of the
 * xReturnStatus parameter - which enables the task to check the status of the
 * operation.
 *
 * See https://freertos.org/mqtt/mqtt-agent-demo.html#example_mqtt_api_call
 *
 * @param[in] pxCommandContext Context of the initial command.
 * @param[in].xReturnStatus The result of the command.
 */
    static void prvSubscribeCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                             MQTTAgentReturnInfo_t * pxReturnInfo );

/**
 * @brief Passed into MQTTAgent_Publish() as the callback to execute when the
 * broker ACKs the PUBLISH message.  Its implementation sends a notification
 * to the task that called MQTTAgent_Publish() to let the task know the
 * PUBLISH operation completed.  It also sets the xReturnStatus of the
 * structure passed in as the command's context to the value of the
 * xReturnStatus parameter - which enables the task to check the status of the
 * operation.
 *
 * See https://freertos.org/mqtt/mqtt-agent-demo.html#example_mqtt_api_call
 *
 * @param[in] pxCommandContext Context of the initial command.
 * @param[in].xReturnStatus The result of the command.
 */
    static void prvPublishCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                           MQTTAgentReturnInfo_t * pxReturnInfo );

    static void prvRegisterSubscribeCallback( const char * pTopicFilter,
                                              uint16_t topicFilterLength );

    static void prvIncomingPublishCallback( void * pvIncomingPublishCallbackContext,
                                            MQTTPublishInfo_t * pxPublishInfo );

    static void prvDeviceAdvisorTask( void * arg );

/**
 * @brief The MQTT agent manages the MQTT contexts.  This set the handle to the
 * context used by this demo.
 */
    extern MQTTAgentContext_t xGlobalMqttAgentContext;

    void vStartDeviceAdvisorTask( void )
    {
        if( xTaskCreate( prvDeviceAdvisorTask,
                         "DEVICE_ADVISOR_TASK",
                         deviceAdvisorTASK_STACK_SIZE,
                         NULL,
                         deviceAdvisorTASK_PRIORITY,
                         NULL ) != pdPASS )
        {
            LogError( ( "Failed to create Device Advisor Task\r\n" ) );
        }
    }

    static void prvDeviceAdvisorTask( void * arg )
    {
        ( void ) arg;

        vWaitUntilMQTTAgentReady();
        vWaitUntilMQTTAgentConnected();

        char cDeviceAdvisorTopicFilter[] = deviceAdvisorTOPIC_FORMAT;

        LogDebug( ( "Sending subscribe request to agent for topic filter: %.*s\n", deviceAdvisorTOPIC_BUFFER_LENGTH, cDeviceAdvisorTopicFilter ) );

        if( prvSubscribeToTopic( MQTTQoS1, cDeviceAdvisorTopicFilter, deviceAdvisorTOPIC_BUFFER_LENGTH ) != MQTTSuccess )
        {
            LogError( ( "Failed to subscribe to topic: %.*s\n",
                        deviceAdvisorTOPIC_BUFFER_LENGTH,
                        cDeviceAdvisorTopicFilter ) );
        }
        else
        {
            LogInfo( ( "Successfully subscribed to topic: %.*s\n",
                       deviceAdvisorTOPIC_BUFFER_LENGTH,
                       cDeviceAdvisorTopicFilter ) );
        }

        while( 1 )
        {
        }
    }

    static MQTTStatus_t prvSubscribeToTopic( MQTTQoS_t xQoS,
                                             char * pcTopicFilter,
                                             size_t xTopicFilterLength )
    {
        MQTTStatus_t xCommandStatus;
        static MQTTAgentSubscribeArgs_t xSubscribeArgs = { 0 };
        static MQTTSubscribeInfo_t xSubscribeInfo = { 0 };
        static MQTTAgentCommandContext_t xCommandContext = { 0UL };
        static MQTTAgentCommandInfo_t xCommandParams = { 0UL };
        uint32_t ulNotifiedValue = 0U;
        BaseType_t result;

        /* Complete the subscribe information.  The topic string must persist for
         * duration of subscription! */
        xSubscribeInfo.pTopicFilter = pcTopicFilter;

        xSubscribeInfo.topicFilterLength = ( uint16_t ) xTopicFilterLength;
        xSubscribeInfo.qos = xQoS;
        xSubscribeArgs.pSubscribeInfo = &xSubscribeInfo;
        xSubscribeArgs.numSubscriptions = 1;

        /* Complete an application defined context associated with this subscribe message.
         * This gets updated in the callback function so the variable must persist until
         * the callback executes. */
        xCommandContext.xTaskToNotify = xTaskGetCurrentTaskHandle();
        xCommandContext.pArgs = ( void * ) &xSubscribeArgs;

        xCommandParams.blockTimeMs = deviceAdvisorMAX_COMMAND_SEND_BLOCK_TIME_MS;
        xCommandParams.cmdCompleteCallback = prvSubscribeCommandCallback;
        xCommandParams.pCmdCompleteCallbackContext = ( void * ) &xCommandContext;

        xTaskNotifyStateClear( NULL );

        xCommandStatus = MQTTAgent_Subscribe( &xGlobalMqttAgentContext,
                                              &xSubscribeArgs,
                                              &xCommandParams );

        if( xCommandStatus == MQTTSuccess )
        {
            /*
             * If command was enqueued successfully, then agent will either process the packet successfully, or if
             * there is a disconnect, then it either retries the publish after reconnecting and resuming session
             * (only for persistent sessions) or cancel the operation and invokes the callback for failed response.
             * Hence the caller task wait indefinitely for a success or failure response from agent.
             */
            result = xTaskNotifyWait( 0UL,
                                      UINT32_MAX,
                                      &ulNotifiedValue,
                                      deviceAdvisorMAX_COMMAND_SEND_BLOCK_TIME_MS );

            if( result != pdTRUE )
            {
                xCommandStatus = MQTTSendFailed;
            }
            else
            {
                xCommandStatus = xCommandContext.xReturnStatus;
            }
        }

        if( ( xCommandStatus != MQTTSuccess ) )
        {
            LogError( ( "Failed to SUBSCRIBE to topic with error = %u.", xCommandStatus ) );
        }
        else
        {
            LogInfo( ( "Subscribed to topic %.*s.\n",
                       xTopicFilterLength,
                       pcTopicFilter ) );
        }

        return xCommandStatus;
    }

    static MQTTStatus_t prvPublishToTopic( MQTTQoS_t xQoS,
                                           char * pcTopic,
                                           size_t xTopicLength,
                                           uint8_t * pucPayload,
                                           size_t xPayloadLength,
                                           int32_t lNumRetries )
    {
        static MQTTPublishInfo_t xPublishInfo = { 0UL };
        static MQTTAgentCommandContext_t xCommandContext = { 0 };
        MQTTStatus_t xCommandStatus;
        static MQTTAgentCommandInfo_t xCommandParams = { 0UL };
        uint32_t ulNotifiedValue = 0U;
        BaseType_t result;

        xTaskNotifyStateClear( NULL );

        /* Configure the publish operation. */
        xPublishInfo.qos = xQoS;
        xPublishInfo.pTopicName = pcTopic;
        xPublishInfo.topicNameLength = ( uint16_t ) xTopicLength;
        xPublishInfo.pPayload = pucPayload;
        xPublishInfo.payloadLength = xPayloadLength;

        xCommandContext.xTaskToNotify = xTaskGetCurrentTaskHandle();

        xCommandParams.blockTimeMs = deviceAdvisorMAX_COMMAND_SEND_BLOCK_TIME_MS;
        xCommandParams.cmdCompleteCallback = prvPublishCommandCallback;
        xCommandParams.pCmdCompleteCallbackContext = &xCommandContext;

        xCommandStatus = MQTTAgent_Publish( &xGlobalMqttAgentContext,
                                            &xPublishInfo,
                                            &xCommandParams );

        if( xCommandStatus == MQTTSuccess )
        {
            /*
             * If command was enqueued successfully, then agent will either process the packet successfully, or if
             * there is a disconnect, then it either retries the publish after reconnecting and resuming session
             * (only for persistent sessions) or cancel the operation and invokes the callback for failed response.
             * Hence the caller task wait indefinitely for a success or failure response from agent.
             */
            result = xTaskNotifyWait( 0UL,
                                      UINT32_MAX,
                                      &ulNotifiedValue,
                                      pdMS_TO_TICKS( deviceAdvisorMAX_COMMAND_SEND_BLOCK_TIME_MS ) );

            if( result != pdTRUE )
            {
                xCommandStatus = MQTTSendFailed;
            }
            else
            {
                xCommandStatus = xCommandContext.xReturnStatus;
            }
        }

        if( ( xCommandStatus != MQTTSuccess ) )
        {
            LogError( ( "Failed to send PUBLISH packet to broker with error = %u.", xCommandStatus ) );
        }
        else
        {
            LogInfo( ( "Sent PUBLISH packet to broker %.*s to broker.\n",
                       xTopicLength,
                       pcTopic ) );
        }

        return xCommandStatus;
    }

    static void prvRegisterSubscribeCallback( const char * pTopicFilter,
                                              uint16_t topicFilterLength )
    {
        /* Add subscription so that incoming publishes are routed to the application callback. */
        bool subscriptionAdded = addSubscription( pTopicFilter,
                                                  topicFilterLength,
                                                  prvIncomingPublishCallback,
                                                  NULL );

        if( subscriptionAdded == false )
        {
            LogError( ( "Failed to register a receive callback for topic %.*s.",
                        pTopicFilter,
                        topicFilterLength ) );
        }
    }

    static void prvSubscribeCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                             MQTTAgentReturnInfo_t * pxReturnInfo )
    {
        if( pxReturnInfo->returnCode == MQTTSuccess )
        {
            MQTTAgentSubscribeArgs_t * pSubscribeArgs = ( MQTTAgentSubscribeArgs_t * ) ( pxCommandContext->pArgs );
            prvRegisterSubscribeCallback( pSubscribeArgs->pSubscribeInfo->pTopicFilter, pSubscribeArgs->pSubscribeInfo->topicFilterLength );
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

    static void prvPublishCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                           MQTTAgentReturnInfo_t * pxReturnInfo )
    {
        if( pxCommandContext->xTaskToNotify != NULL )
        {
            ( void ) xTaskNotify( pxCommandContext->xTaskToNotify,
                                  pxReturnInfo->returnCode,
                                  eSetValueWithOverwrite );
        }
    }

    static void prvIncomingPublishCallback( void * pvIncomingPublishCallbackContext,
                                            MQTTPublishInfo_t * pxPublishInfo )
    {
        ( void ) pvIncomingPublishCallbackContext;

        static char cTerminatedString[ deviceAdvisorSTRING_BUFFER_LENGTH ];

        /* Create a message that contains the incoming MQTT payload to the logger,
         * terminating the string first. */
        if( pxPublishInfo->payloadLength < deviceAdvisorSTRING_BUFFER_LENGTH )
        {
            memcpy( ( void * ) cTerminatedString, pxPublishInfo->pPayload, pxPublishInfo->payloadLength );
            cTerminatedString[ pxPublishInfo->payloadLength ] = 0x00;
        }
        else
        {
            memcpy( ( void * ) cTerminatedString, pxPublishInfo->pPayload, deviceAdvisorSTRING_BUFFER_LENGTH );
            cTerminatedString[ deviceAdvisorSTRING_BUFFER_LENGTH - 1 ] = 0x00;
        }

        LogInfo( ( "Received incoming publish message %s\n", cTerminatedString ) );
    }

#endif /* if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 ) */
