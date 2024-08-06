/*
 * FreeRTOS V202011.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.]
 * Copyright 2024 Arm Limited and/or its affiliates
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
 * https://aws.amazon.com/freertos
 *
 */

#ifndef SUBSCRIPTION_MANAGER_H
#define SUBSCRIPTION_MANAGER_H


#include "fff.h"
#include <inttypes.h>
#include "core_mqtt_serializer.h"

typedef struct SubscriptionElement
{
    int usFilterStringLength;
    const char * pcSubscriptionFilterString;
} SubscriptionElement_t;

#ifndef SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS
    #define SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS    10U
#endif

typedef void (* IncomingPubCallback_t)( void * pvIncomingPublishCallbackContext,
                                        MQTTPublishInfo_t * pxPublishInfo );

DECLARE_FAKE_VALUE_FUNC( bool,
                         addSubscription,
                         const char *,
                         uint16_t,
                         IncomingPubCallback_t,
                         void * );
DECLARE_FAKE_VOID_FUNC( removeSubscription,
                        const char *,
                        uint16_t );

DECLARE_FAKE_VALUE_FUNC( bool,
                         handleIncomingPublishes,
                         MQTTPublishInfo_t * );

#endif /* SUBSCRIPTION_MANAGER_H */
