/*
 * FreeRTOS V202212.00
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

/**
 * @file subscription_manager.c
 * @brief Functions for managing MQTT subscriptions.
 */

/* Standard includes. */
#include <string.h>

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "SUB MGR"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

/* Subscription manager header include. */
#include "subscription_manager.h"

/**
 * @brief The global array of subscription elements.
 *
 * @note No thread safety is required to this array, since the updates the array
 * elements are done only from one task at a time. The subscription manager
 * implementation expects that the array of the subscription elements used for
 * storing subscriptions to be initialized to 0. As this is a global array, it
 * will be intialized to 0 by default.
 */
SubscriptionElement_t xGlobalSubscriptionList[ SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS ];


bool addSubscription( const char * pcTopicFilterString,
                      uint16_t usTopicFilterLength,
                      IncomingPubCallback_t pxIncomingPublishCallback,
                      void * pvIncomingPublishCallbackContext )
{
    bool xReturnStatus = false;

    if( ( pcTopicFilterString == NULL ) ||
        ( usTopicFilterLength == 0U ) ||
        ( pxIncomingPublishCallback == NULL ) )
    {
        LogError( ( "Invalid parameter. spcTopicFilterString=%p,"
                    " usTopicFilterLength=%u, pxIncomingPublishCallback=%p.",
                    pcTopicFilterString,
                    ( unsigned int ) usTopicFilterLength,
                    pxIncomingPublishCallback ) );
    }
    else
    {
        int32_t lIndex;
        size_t xAvailableIndex = SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS;

        /* Start at end of array, so that we will insert at the first available index.
         * Scans backwards to find duplicates. */
        for( lIndex = ( int32_t ) SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS - 1; lIndex >= 0; lIndex-- )
        {
            if( xGlobalSubscriptionList[ lIndex ].usFilterStringLength == 0 )
            {
                xAvailableIndex = lIndex;
            }
            else if( ( xGlobalSubscriptionList[ lIndex ].usFilterStringLength == usTopicFilterLength ) &&
                     ( strncmp( pcTopicFilterString, xGlobalSubscriptionList[ lIndex ].pcSubscriptionFilterString, ( size_t ) usTopicFilterLength ) == 0 ) )
            {
                /* If a subscription already exists, don't do anything. */
                if( ( xGlobalSubscriptionList[ lIndex ].pxIncomingPublishCallback == pxIncomingPublishCallback ) &&
                    ( xGlobalSubscriptionList[ lIndex ].pvIncomingPublishCallbackContext == pvIncomingPublishCallbackContext ) )
                {
                    LogWarn( ( "Subscription already exists.\n" ) );
                    xAvailableIndex = SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS;
                    xReturnStatus = true;
                    break;
                }
            }
        }

        if( xAvailableIndex < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS )
        {
            xGlobalSubscriptionList[ xAvailableIndex ].pcSubscriptionFilterString = pcTopicFilterString;
            xGlobalSubscriptionList[ xAvailableIndex ].usFilterStringLength = usTopicFilterLength;
            xGlobalSubscriptionList[ xAvailableIndex ].pxIncomingPublishCallback = pxIncomingPublishCallback;
            xGlobalSubscriptionList[ xAvailableIndex ].pvIncomingPublishCallbackContext = pvIncomingPublishCallbackContext;
            xReturnStatus = true;
        }
    }

    return xReturnStatus;
}

/*-----------------------------------------------------------*/

void removeSubscription( const char * pcTopicFilterString,
                         uint16_t usTopicFilterLength )
{
    if( ( pcTopicFilterString == NULL ) ||
        ( usTopicFilterLength == 0U ) )
    {
        LogError( ( "Invalid parameter.  pcTopicFilterString=%p,"
                    " usTopicFilterLength=%u.",
                    pcTopicFilterString,
                    ( unsigned int ) usTopicFilterLength ) );
    }
    else
    {
        int32_t lIndex;

        for( lIndex = 0; lIndex < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS; lIndex++ )
        {
            if( xGlobalSubscriptionList[ lIndex ].usFilterStringLength == usTopicFilterLength )
            {
                if( strncmp( xGlobalSubscriptionList[ lIndex ].pcSubscriptionFilterString, pcTopicFilterString, usTopicFilterLength ) == 0 )
                {
                    memset( &( xGlobalSubscriptionList[ lIndex ] ), 0x00, sizeof( SubscriptionElement_t ) );
                }
            }
        }
    }
}

/*-----------------------------------------------------------*/

bool handleIncomingPublishes( MQTTPublishInfo_t * pxPublishInfo )
{
    bool isMatched = false, publishHandled = false;

    if( pxPublishInfo == NULL )
    {
        LogError( ( "Invalid parameter. pxPublishInfo=%p,",
                    pxPublishInfo ) );
    }
    else
    {
        int32_t lIndex;

        for( lIndex = 0; lIndex < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS; lIndex++ )
        {
            if( xGlobalSubscriptionList[ lIndex ].usFilterStringLength > 0 )
            {
                MQTT_MatchTopic( pxPublishInfo->pTopicName,
                                 pxPublishInfo->topicNameLength,
                                 xGlobalSubscriptionList[ lIndex ].pcSubscriptionFilterString,
                                 xGlobalSubscriptionList[ lIndex ].usFilterStringLength,
                                 &isMatched );

                if( isMatched == true )
                {
                    xGlobalSubscriptionList[ lIndex ].pxIncomingPublishCallback( xGlobalSubscriptionList[ lIndex ].pvIncomingPublishCallbackContext,
                                                                                 pxPublishInfo );
                    publishHandled = true;
                }
            }
        }
    }

    return publishHandled;
}
