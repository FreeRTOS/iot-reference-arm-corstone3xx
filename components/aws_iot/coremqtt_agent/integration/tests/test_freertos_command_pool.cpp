/* Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "fff.h"

#include "gtest/gtest.h"


extern "C" {
#include "FreeRTOSConfig.h"
#include "core_mqtt_config.h"
#include "FreeRTOS.h"
#include "logging_stack.h"
#include "queue.h"
#include "freertos_command_pool.h"

/* Directly copy-paste mock headers from the file under test's directory.
 * Otherwise, the non-mock files are detected. */

/* freertos_agent_message.h */
#ifndef FREERTOS_AGENT_MESSAGE_H
    #define FREERTOS_AGENT_MESSAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "core_mqtt_agent_message_interface.h"
#include "fff.h"
#include "queue.h"

    struct MQTTAgentMessageContext
    {
        QueueHandle_t queue;
    };

    DECLARE_FAKE_VALUE_FUNC( bool,
                             Agent_MessageSend,
                             MQTTAgentMessageContext_t *,
                             MQTTAgentCommand_t * const *,
                             uint32_t );
    DECLARE_FAKE_VALUE_FUNC( bool,
                             Agent_MessageReceive,
                             MQTTAgentMessageContext_t *,
                             MQTTAgentCommand_t * *,
                             uint32_t );
#endif /* FREERTOS_AGENT_MESSAGE_H */

/* freertos_agent_message.c */

DEFINE_FAKE_VALUE_FUNC( bool,
                        Agent_MessageSend,
                        MQTTAgentMessageContext_t *,
                        MQTTAgentCommand_t * const *,
                        uint32_t );
DEFINE_FAKE_VALUE_FUNC( bool,
                        Agent_MessageReceive,
                        MQTTAgentMessageContext_t *,
                        MQTTAgentCommand_t * *,
                        uint32_t );

/* Functions usually defined by main.c */
DEFINE_FAKE_VOID_FUNC( vAssertCalled,
                       const char *,
                       unsigned long );
DEFINE_FAKE_VOID_FUNC_VARARG( SdkLogError,
                              const char *,
                              ... );
DEFINE_FAKE_VOID_FUNC_VARARG( SdkLogWarn,
                              const char *,
                              ... );
DEFINE_FAKE_VOID_FUNC_VARARG( SdkLogInfo,
                              const char *,
                              ... );
DEFINE_FAKE_VOID_FUNC_VARARG( SdkLogDebug,
                              const char *,
                              ... );
}

DEFINE_FFF_GLOBALS

#define ASSERTION_FAILURE    1

/* Mocks for vAssertCalled */
void throw_assertion_failure( const char * pcFile,
                              unsigned long ulLine )
{
    /*
     * Behaviour wanted:
     * - Encounters assertion fail, stops running any more code. E.g. does not go to next line.
     * - But checks all assertions in the google test program hold.
     */
    throw ( ASSERTION_FAILURE );
}
void do_nothing_on_assertion_failure( const char * pcFile,
                                      unsigned long ulLine ) /* do nothing */
{
}

class TestFreertosCommandPool : public ::testing::Test {
public:
    TestFreertosCommandPool()
    {
        /* May be called but not used in the test suite.*/
        RESET_FAKE( SdkLogError );
        RESET_FAKE( SdkLogDebug );
        RESET_FAKE( SdkLogWarn );
        RESET_FAKE( SdkLogInfo );

        /* Used in the test suite. */
        RESET_FAKE( xQueueCreateStatic );
        RESET_FAKE( Agent_MessageSend );
        RESET_FAKE( Agent_MessageReceive );
        RESET_FAKE( vAssertCalled );              /* used to trap errors. */
        vAssertCalled_fake.custom_fake = throw_assertion_failure;
        Agent_MessageSend_fake.return_val = true; /* success for InitializePool. */
    }
};

void expect_no_errors( void )
{
    EXPECT_EQ( vAssertCalled_fake.call_count, 0 );
    EXPECT_EQ( SdkLogError_fake.call_count, 0 );
}
void expect_errors( void )
{
    EXPECT_NE( vAssertCalled_fake.call_count + SdkLogError_fake.call_count, 0 );
}

TEST_F( TestFreertosCommandPool, correct_command_pool_initialisation_causes_no_errors )
{
    Agent_MessageSend_fake.return_val = true;
    QueueDefinition queue = { 10 };
    QueueHandle_t handle = &queue;
    xQueueCreateStatic_fake.return_val = handle;

    Agent_InitializePool();

    expect_no_errors();
}

TEST_F( TestFreertosCommandPool, initialisation_errors_if_queue_creation_fails )
{
    vAssertCalled_fake.custom_fake = do_nothing_on_assertion_failure;
    Agent_MessageSend_fake.return_val = true;
    QueueHandle_t handle; /* invalid handle */
    xQueueCreateStatic_fake.return_val = handle;

    Agent_InitializePool();

    expect_errors();
}

TEST_F( TestFreertosCommandPool, errors_if_initialisation_cannot_send_items_to_queue )
{
    vAssertCalled_fake.custom_fake = do_nothing_on_assertion_failure;
    Agent_MessageSend_fake.return_val = false;
    QueueDefinition queue = { 10 };
    QueueHandle_t handle = &queue;
    xQueueCreateStatic_fake.return_val = handle;

    Agent_InitializePool();

    expect_errors();
}

TEST_F( TestFreertosCommandPool, get_command_tries_to_obtain_a_command_if_the_command_pool_is_not_full )
{
    Agent_MessageReceive_fake.return_val = true;
    Agent_MessageSend_fake.return_val = true;
    QueueDefinition queue = { 10 };
    QueueHandle_t handle = &queue;
    xQueueCreateStatic_fake.return_val = handle;

    Agent_InitializePool();
    expect_no_errors();
    EXPECT_EQ( Agent_MessageReceive_fake.call_count, 0 );

    /* 20ms is used as the block time */
    Agent_GetCommand( 20 );

    /* This test cannot run if the pool cannot contain anything. */
    EXPECT_NE( MQTT_COMMAND_CONTEXTS_POOL_SIZE, 0 );
    EXPECT_NE( Agent_MessageReceive_fake.call_count, 0 );
}

TEST_F( TestFreertosCommandPool, does_not_try_to_get_command_if_pool_not_initialized )
{
    Agent_MessageReceive_fake.return_val = true;
    Agent_MessageSend_fake.return_val = true;

    /* We expect MessageReceive to never be called on unsafe memory. */
    try{
        Agent_GetCommand( 20 );
    }
    catch( int num ) {
        if( num != ASSERTION_FAILURE )
        {
            throw ( num );
        }
    }
    expect_errors();
    EXPECT_EQ( Agent_MessageReceive_fake.call_count, 0 );
}

TEST_F( TestFreertosCommandPool, tries_to_wait_the_correct_amount_of_time_to_receive_a_message )
{
    QueueDefinition queue = { 10 };
    QueueHandle_t handle = &queue;

    xQueueCreateStatic_fake.return_val = handle; /* used to intialise pool. */
    uint32_t blockDurationMs = 20;

    Agent_InitializePool();
    expect_no_errors();

    Agent_MessageReceive_fake.return_val = true;
    EXPECT_EQ( Agent_MessageReceive_fake.call_count, 0 );
    Agent_GetCommand( blockDurationMs );
    EXPECT_NE( Agent_MessageReceive_fake.call_count, 0 );
    EXPECT_EQ( Agent_MessageReceive_fake.arg2_val, blockDurationMs );
}

/* This checks that memory locations are not accessed without checking first. */
TEST_F( TestFreertosCommandPool, trying_to_release_a_bad_command_pointer_does_not_cause_crashes )
{
    QueueDefinition queue = { 10 };
    QueueHandle_t handle = &queue;

    xQueueCreateStatic_fake.return_val = handle;

    Agent_InitializePool();
    expect_no_errors();

    Agent_ReleaseCommand( nullptr );
    expect_no_errors();
}

TEST_F( TestFreertosCommandPool, failing_to_release_command_returns_false )
{
    QueueDefinition queue = { 10 };
    QueueHandle_t handle = &queue;

    xQueueCreateStatic_fake.return_val = handle;

    Agent_InitializePool();
    expect_no_errors();

    EXPECT_FALSE( Agent_ReleaseCommand( nullptr ) );
    expect_no_errors();
}
