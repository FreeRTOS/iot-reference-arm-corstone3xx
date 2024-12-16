/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mqtt_agent_task.h"

#include "mqtt_helpers.h"
#include "ota_register_callback.h"
#include "ota_types_definitions.h"

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Configure name and log level for the OTA library. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "OTA"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

/* Provides external linkage only when running unit test */
#ifdef UNIT_TESTING
    #define STATIC    /* as nothing */
#else /* ifdef UNIT_TESTING */
    #define STATIC    static
#endif /* UNIT_TESTING */

/**
 * @brief The MQTT agent manages the MQTT contexts. It will set this handle to
 * the context used by this demo.
 */
extern MQTTAgentContext_t xGlobalMqttAgentContext;

STATIC void prvMQTTSubscribeCompleteCallback( MQTTAgentCommandContext_t * pxCommandContext,
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

STATIC void prvOTAPublishCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                          MQTTAgentReturnInfo_t * pxReturnInfo )
{
    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

    if( pxCommandContext->xTaskToNotify != NULL )
    {
        xTaskNotify( pxCommandContext->xTaskToNotify, ( uint32_t ) ( pxReturnInfo->returnCode ), eSetValueWithOverwrite );
    }
}

STATIC void prvMQTTUnsubscribeCompleteCallback( MQTTAgentCommandContext_t * pxCommandContext,
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

OtaMqttStatus_t prvMQTTSubscribe( const char * pTopicFilter,
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

OtaMqttStatus_t prvMQTTPublish( const char * const pacTopic,
                                uint16_t topicLen,
                                const char * pMsg,
                                uint32_t msgSize,
                                uint8_t ucQoS )
{
    BaseType_t result;
    MQTTStatus_t mqttStatus = MQTTBadParameter;
    static MQTTPublishInfo_t publishInfo = { 0 };
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };
    static MQTTAgentCommandContext_t xCommandContext = { 0 };
    OtaMqttStatus_t otaRet = OtaMqttSuccess;

    publishInfo.pTopicName = pacTopic;
    publishInfo.topicNameLength = topicLen;
    publishInfo.qos = ucQoS;
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

OtaMqttStatus_t prvMQTTUnsubscribe( const char * pTopicFilter,
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
