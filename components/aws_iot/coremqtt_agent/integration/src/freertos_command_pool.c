/*
 * FreeRTOS V202104.00
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
 * @file freertos_command_pool.c
 * @brief Implements functions to obtain and release commands.
 */

/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"

/* Header include. */
#include "freertos_command_pool.h"
#include "freertos_agent_message.h"

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "COMMAND POOL"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"
#include "core_mqtt_config.h"

/*-----------------------------------------------------------*/

#define QUEUE_NOT_INITIALIZED    ( 0U )
#define QUEUE_INITIALIZED        ( 1U )

/**
 * @brief The pool of command structures used to hold information on commands (such
 * as PUBLISH or SUBSCRIBE) between the command being created by an API call and
 * completion of the command by the execution of the command's callback.
 */
static MQTTAgentCommand_t commandStructurePool[ MQTT_COMMAND_CONTEXTS_POOL_SIZE ];

/**
 * @brief The message context used to guard the pool of MQTTAgentCommand_t structures.
 * For FreeRTOS, this is implemented with a queue. Structures may be
 * obtained by receiving a pointer from the queue, and returned by
 * sending the pointer back into it.
 */
static MQTTAgentMessageContext_t commandStructMessageCtx;

/**
 * @brief Initialization status of the queue.
 */
static volatile uint8_t initStatus = QUEUE_NOT_INITIALIZED;

/*-----------------------------------------------------------*/

void Agent_InitializePool( void )
{
    MQTTAgentCommand_t * pCommand;
    static StaticQueue_t staticQueueStructure;

    if( initStatus == QUEUE_NOT_INITIALIZED )
    {
        static uint8_t staticQueueStorageArea[ MQTT_COMMAND_CONTEXTS_POOL_SIZE * sizeof( MQTTAgentCommand_t * ) ];
        memset( ( void * ) commandStructurePool, 0x00, sizeof( commandStructurePool ) );
        commandStructMessageCtx.queue = xQueueCreateStatic( MQTT_COMMAND_CONTEXTS_POOL_SIZE,
                                                            sizeof( MQTTAgentCommand_t * ),
                                                            staticQueueStorageArea,
                                                            &staticQueueStructure );
        configASSERT( commandStructMessageCtx.queue );

        size_t i;

        /* Populate the queue. */
        for( i = 0; i < MQTT_COMMAND_CONTEXTS_POOL_SIZE; i++ )
        {
            /* Store the address as a variable. */
            pCommand = &commandStructurePool[ i ];
            /* Send the pointer to the queue. */
            bool commandAdded = Agent_MessageSend( &commandStructMessageCtx, &pCommand, 0U );
            configASSERT( commandAdded );
        }

        initStatus = QUEUE_INITIALIZED;
    }
}

/*-----------------------------------------------------------*/

MQTTAgentCommand_t * Agent_GetCommand( uint32_t blockTimeMs )
{
    MQTTAgentCommand_t * structToUse = NULL;
    bool structRetrieved = false;

    /* Check queue has been created. */
    configASSERT( initStatus == QUEUE_INITIALIZED );

    /* Retrieve a struct from the queue. */
    structRetrieved = Agent_MessageReceive( &commandStructMessageCtx, &( structToUse ), blockTimeMs );

    if( !structRetrieved )
    {
        LogDebug( ( "No command structure available.\n" ) );
    }

    return structToUse;
}

/*-----------------------------------------------------------*/

bool Agent_ReleaseCommand( MQTTAgentCommand_t * pCommandToRelease )
{
    bool structReturned = false;

    configASSERT( initStatus == QUEUE_INITIALIZED );

    /* See if the structure being returned is actually from the pool. */
    if( ( pCommandToRelease >= commandStructurePool ) &&
        ( pCommandToRelease < ( commandStructurePool + MQTT_COMMAND_CONTEXTS_POOL_SIZE ) ) )
    {
        structReturned = Agent_MessageSend( &commandStructMessageCtx, &pCommandToRelease, 0U );

        /* The send should not fail as the queue was created to hold every command
         * in the pool. */
        configASSERT( structReturned );
        LogDebug( ( "Returned Command Context %d to pool\n",
                    ( int ) ( pCommandToRelease - commandStructurePool ) ) );
    }

    return structReturned;
}
