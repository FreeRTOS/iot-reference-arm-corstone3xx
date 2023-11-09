/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "events.h"

/* System events group. */
EventGroupHandle_t xSystemEvents = NULL;
static StaticEventGroup_t xSystemEventsGroup;

int32_t xEventHelperInit( void )
{
    /* Create a system events group. */
    xSystemEvents = xEventGroupCreateStatic( &xSystemEventsGroup );
    return ( xSystemEvents == NULL ) ? -1 : 0;
}

void vWaitUntilNetworkIsUp( void )
{
    configASSERT( xSystemEvents != NULL );

    /* There is no need to check the return value of this API, since the task
     * is waiting for a particular bit to be set and is waiting forever. */
    ( void ) xEventGroupWaitBits( xSystemEvents,
                                  EVENT_MASK_NETWORK_UP,
                                  pdFALSE,
                                  pdFALSE,
                                  portMAX_DELAY );
}

void vWaitUntilMQTTAgentReady( void )
{
    configASSERT( xSystemEvents != NULL );

    /* There is no need to check the return value of this API, since the task
     * is waiting for a particular bit to be set and is waiting forever. */
    ( void ) xEventGroupWaitBits( xSystemEvents,
                                  EVENT_MASK_MQTT_INIT,
                                  pdFALSE,
                                  pdFALSE,
                                  portMAX_DELAY );
}

void vWaitUntilMQTTAgentConnected( void )
{
    configASSERT( xSystemEvents != NULL );

    /* There is no need to check the return value of this API, since the task
     * is waiting for a particular bit to be set and is waiting forever. */
    ( void ) xEventGroupWaitBits( xSystemEvents,
                                  EVENT_MASK_MQTT_CONNECTED,
                                  pdFALSE,
                                  pdFALSE,
                                  portMAX_DELAY );
}

bool xIsMqttAgentConnected( void )
{
    EventBits_t uxEvents = xEventGroupGetBits( xSystemEvents );

    return( ( bool ) ( uxEvents & EVENT_MASK_MQTT_CONNECTED ) );
}
