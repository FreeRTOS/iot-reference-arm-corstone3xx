/* Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "fff.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace std;

extern "C" {
#include "FreeRTOSConfig.h"
#include "backoff_algorithm.h"
#include "core_mqtt_agent_message_interface.h"
#include "core_mqtt_serializer.h"
#include "event_groups.h"
#include "events.h"
#include "logging_stack.h"
#include "mqtt_agent_task.h"
#include "psa/crypto.h"
#include "psa/error.h"
#include "task.h"
#include "transport_interface_api.h"
#include "queue.h"

/*
 * Exposed static functions to test
 * The below functions are found in `mqtt_agent_task.c`, which is
 * found by the inclusion of `mqtt_agent_task.h`.
 */

extern void prvMQTTAgentTask( void * pParam );
extern BaseType_t prvSocketConnect( NetworkContext_t * pNetworkContext );
extern void prvDisconnectFromMQTTBroker( void );
extern MQTTStatus_t prvMQTTInit( void );
extern MQTTStatus_t prvMQTTConnect( void );
extern uint32_t prvGetTimeMs( void );
extern UBaseType_t prvGetRandomNumber( void );
extern BaseType_t prvSocketConnect( NetworkContext_t * pxNetworkContext );
extern BaseType_t prvSocketDisconnect( NetworkContext_t * pxNetworkContext );
extern void prvIncomingPublishCallback( MQTTAgentContext_t * pMqttAgentContext,
                                        uint16_t packetId,
                                        MQTTPublishInfo_t * pxPublishInfo );
extern void prvReSubscriptionCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                              MQTTAgentReturnInfo_t * pxReturnInfo );
extern MQTTStatus_t prvHandleResubscribe( void );
extern MQTTStatus_t prvMQTTConnect( void );
extern void prvDisconnectCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                          MQTTAgentReturnInfo_t * pxReturnInfo );
extern void prvDisconnectFromMQTTBroker( void );
extern void prvMQTTAgentTask( void * pParam );

/* Directly copy-paste mock headers from the file under test's directory.
 * Otherwise, the non-mock files are detected. */

/* subscription_manager.h */
#ifndef SUBSCRIPTION_MANAGER_H
    #define SUBSCRIPTION_MANAGER_H
    typedef struct SubscriptionElement
    {
        int usFilterStringLength;
        const char * pcSubscriptionFilterString;
    } SubscriptionElement_t;

    #ifndef SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS
        #define SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS    10U
    #endif

    DECLARE_FAKE_VOID_FUNC( removeSubscription,
                            const char *,
                            uint16_t );

    DECLARE_FAKE_VALUE_FUNC( bool,
                             handleIncomingPublishes,
                             MQTTPublishInfo_t * );

    DEFINE_FAKE_VOID_FUNC( removeSubscription,
                           const char *,
                           uint16_t );

    DEFINE_FAKE_VALUE_FUNC( bool,
                            handleIncomingPublishes,
                            MQTTPublishInfo_t * );

    SubscriptionElement_t xGlobalSubscriptionList[ SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS ];
#endif /* SUBSCRIPTION_MANAGER_H */

/* freertos_command_pool.h */
#ifndef FREERTOS_COMMAND_POOL_H
    #define FREERTOS_COMMAND_POOL_H
    DECLARE_FAKE_VOID_FUNC( Agent_InitializePool );
    DECLARE_FAKE_VALUE_FUNC( MQTTAgentCommand_t *,
                             Agent_GetCommand,
                             uint32_t );
    DECLARE_FAKE_VALUE_FUNC( bool,
                             Agent_ReleaseCommand,
                             MQTTAgentCommand_t * );
    DEFINE_FAKE_VOID_FUNC( Agent_InitializePool );
    DEFINE_FAKE_VALUE_FUNC( MQTTAgentCommand_t *,
                            Agent_GetCommand,
                            uint32_t );
    DEFINE_FAKE_VALUE_FUNC( bool,
                            Agent_ReleaseCommand,
                            MQTTAgentCommand_t * );
#endif /* FREERTOS_COMMAND_POOL_H */

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

#define ASSERTION_FAIL    int

/* Mock for vAssertCalled */
void throw_assertion_failure( const char * pcFile,
                              unsigned long ulLine )
{
    throw ( 1 );

    /*
     * Behaviour wanted:
     * - Encounters assertion fail, stops running any more code. E.g. does not go to next line.
     * - But checks all assertions in the google test program hold.
     */
}

/* Being under this test class denotes a valid test that needs a corresponding fix, but we do not want clogging the testsuite. */
class SkipTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        GTEST_SKIP() << "Skipping all tests under this suite";
    }
};

class TestMqttAgentTask : public ::testing::Test {
public:
    TestMqttAgentTask()
    {
        RESET_FAKE( Agent_InitializePool );
        RESET_FAKE( BackoffAlgorithm_InitializeParams );
        RESET_FAKE( BackoffAlgorithm_GetNextBackoff );
        RESET_FAKE( handleIncomingPublishes );
        RESET_FAKE( MQTTAgent_CancelAll );
        RESET_FAKE( MQTTAgent_CommandLoop );
        RESET_FAKE( MQTTAgent_Init );
        RESET_FAKE( MQTTAgent_ResumeSession )
        RESET_FAKE( MQTTAgent_Subscribe );
        RESET_FAKE( MQTT_Connect );
        RESET_FAKE( MQTT_Status_strerror );
        RESET_FAKE( psa_generate_random );
        RESET_FAKE( SdkLogError );
        RESET_FAKE( SdkLogInfo );
        RESET_FAKE( SdkLogWarn );
        RESET_FAKE( Transport_Disconnect );
        RESET_FAKE( Transport_Connect );
        RESET_FAKE( vAssertCalled );
        RESET_FAKE( vTaskDelay );
        RESET_FAKE( vTaskDelete );
        RESET_FAKE( vWaitUntilNetworkIsUp );
        RESET_FAKE( xEventGroupClearBits );
        RESET_FAKE( xEventGroupSetBits );
        RESET_FAKE( xTaskCreate );
        RESET_FAKE( xTaskGetCurrentTaskHandle );
        RESET_FAKE( xTaskGetTickCount );
        RESET_FAKE( xTaskNotifyWait );
        RESET_FAKE( xQueueCreateStatic );

        /* Wrap functions expected to fail an assertion in EXPECT_THROW from GoogleTest. */
        vAssertCalled_fake.custom_fake = throw_assertion_failure;
    }
};

/* Test helper functions */
void expect_no_errors( void )
{
    ASSERT_EQ( SdkLogError_fake.call_count, 0 );
    ASSERT_EQ( vAssertCalled_fake.call_count, 0 );
}
void expect_errors( void )
{
    ASSERT_NE( SdkLogError_fake.call_count + vAssertCalled_fake.call_count, 0 ) << "Expected an error reported by LogError or an assertion failure.";
}
/* expect throws */
void expect_errors_or_warnings( void )
{
    ASSERT_NE( SdkLogError_fake.call_count + SdkLogWarn_fake.call_count + vAssertCalled_fake.call_count, 0 ) << "Expected an error reported by LogError, LogWarn or an assertion failure.";
}
void expect_no_errors_or_warnings( void )
{
    ASSERT_EQ( SdkLogError_fake.call_count + SdkLogWarn_fake.call_count + vAssertCalled_fake.call_count, 0 );
}
/* Custom fake for xEventGroupClearBits */
int expect_clearing_MQTT_event_mask( void * unused,
                                     const int mask )
{
    EXPECT_EQ( mask, EVENT_MASK_MQTT_CONNECTED );
    return 1;
}
int expect_mqtt_connected_event_mask( void * unused,
                                      const int mask )
{
    EXPECT_EQ( mask, EVENT_MASK_MQTT_CONNECTED );
    return 1;
}
/* Custom fake for xTaskGetTickCount() */
TickType_t sharedCounter = 0;
TickType_t increment_shared_counter_and_return( void )
{
    sharedCounter = sharedCounter + 1;
    return sharedCounter;
}
/* Custom fake for psa_generate_random */
psa_status_t set_random_variable_to_three_and_return_success( uint8_t * randomVar,
                                                              size_t outputSize )
{
    *randomVar = 3;
    return PSA_SUCCESS;
}
/* Custom fakes for xTaskNotify */
BaseType_t return_pdpass_and_expect_task_handle_points_to_five( TaskHandle_t handle,
                                                                uint32_t returnCode,
                                                                eNotifyAction unused )
{
    EXPECT_EQ( *handle, 5 );
    return pdPASS;
}
BaseType_t return_pdpass_and_expect_mqtt_success_return_code( TaskHandle_t handle,
                                                              uint32_t returnCode,
                                                              eNotifyAction unused )
{
    EXPECT_EQ( returnCode, MQTTSuccess );
    return pdPASS;
}
BaseType_t return_pdpass_and_expect_mqtt_bad_parameter_return_code( TaskHandle_t handle,
                                                                    uint32_t returnCode,
                                                                    eNotifyAction unused )
{
    EXPECT_EQ( returnCode, MQTTBadParameter );
    return pdPASS;
}
/* Custom fake for Transport_Connect */
TransportStatus_t check_if_timeout_less_than_ten_seconds( NetworkContext_t * pNetworkContext,
                                                          const ServerInfo_t * pServerInfo,
                                                          const TLSParams_t * pTLSParams,
                                                          uint32_t sendTimeoutMs,
                                                          uint32_t recvTimeoutMs )
{
    uint32_t MAX_TIMEOUT_IN_MS = 10000;

    EXPECT_LE( sendTimeoutMs, MAX_TIMEOUT_IN_MS );
    EXPECT_LE( recvTimeoutMs, MAX_TIMEOUT_IN_MS );
    return TRANSPORT_STATUS_SUCCESS;
}

/* The  file under test contains static functions which the tests in this file assume are made visible
 * by conditional compiling macros. This test verifies these macros are defined. */
TEST_F( TestMqttAgentTask, Can_test_static_functions )
{
    #ifndef UNIT_TESTING
        FAIL() << "The macro UNIT_TESTING is not defined, please add this to your CMake compile definitions.";
    #endif /* UNIT_TESTING */
}

/* Testing vStartMqttAgentTask */

TEST_F( TestMqttAgentTask, Starting_the_agent_creates_a_task )
{
    ASSERT_EQ( xTaskCreate_fake.call_count, 0 );
    vStartMqttAgentTask();
    ASSERT_NE( xTaskCreate_fake.call_count, 0 );
    expect_no_errors();
}

/* Testing prvSocketConnect */

TEST_F( TestMqttAgentTask, Socket_connect_tries_to_make_a_connection )
{
    EXPECT_EQ( Transport_Connect_fake.call_count, 0 );
    prvSocketConnect( 0 );
    EXPECT_NE( Transport_Connect_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Socket_connect_returns_success_on_successful_connection )
{
    Transport_Connect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    EXPECT_EQ( prvSocketConnect( 0 ), pdPASS );
    expect_no_errors();
}
TEST_F( TestMqttAgentTask, Socket_connect_does_not_error_on_unsuccessful_connection )
{
    Transport_Connect_fake.return_val = TRANSPORT_STATUS_CONNECT_FAILURE;
    prvSocketConnect( 0 );
    expect_no_errors();
}
TEST_F( TestMqttAgentTask, Socket_connection_reattempt_does_not_continue_past_reasonable_time )
{
    Transport_Connect_fake.custom_fake = check_if_timeout_less_than_ten_seconds;
    prvSocketConnect( 0 );
    expect_no_errors();
}
TEST_F( TestMqttAgentTask, Socket_connect_returns_failure_on_unsuccessful_connection )
{
    Transport_Connect_fake.return_val = TRANSPORT_STATUS_CONNECT_FAILURE;
    EXPECT_EQ( prvSocketConnect( 0 ), pdFALSE );
    expect_no_errors();
}

/* Testing prvSocketDisconnect */

TEST_F( TestMqttAgentTask, Socket_disconnect_tries_to_close_a_connection )
{
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    xEventGroupClearBits_fake.return_val = 1;
    int dummy = 1;
    EXPECT_EQ( Transport_Disconnect_fake.call_count, 0 );
    prvSocketDisconnect( &dummy );
    EXPECT_NE( Transport_Disconnect_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Socket_disconnect_returns_success_when_disconnecting_succeeds )
{
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    xEventGroupClearBits_fake.return_val = 1;
    int dummy = 1;
    EXPECT_EQ( prvSocketDisconnect( &dummy ), pdPASS );
}
TEST_F( TestMqttAgentTask, Socket_disconnect_returns_failure_when_disconnecting_fails )
{
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_INVALID_PARAMETER;
    xEventGroupClearBits_fake.return_val = 1;
    int dummy = 1;
    EXPECT_EQ( prvSocketDisconnect( &dummy ), pdFAIL );
}
TEST_F( TestMqttAgentTask, Socket_disconnect_informs_system_there_is_no_MQTT_connection_on_connection_closure )
{
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    xEventGroupClearBits_fake.custom_fake = expect_clearing_MQTT_event_mask;
    EXPECT_EQ( xEventGroupClearBits_fake.call_count, 0 );
    int dummy = 1;
    prvSocketDisconnect( &dummy );
    EXPECT_NE( xEventGroupClearBits_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Socket_disconnect_does_not_inform_system_there_is_no_MQTT_connection_if_connection_not_closed )
{
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_INVALID_PARAMETER;
    xEventGroupClearBits_fake.custom_fake = expect_clearing_MQTT_event_mask;
    EXPECT_EQ( xEventGroupClearBits_fake.call_count, 0 );
    int dummy = 1;
    prvSocketDisconnect( &dummy );
    EXPECT_EQ( xEventGroupClearBits_fake.call_count, 0 );
}

/* Testing prvDisconnectFromMQTTBroker */

TEST_F( TestMqttAgentTask, Disconnect_from_broker_tries_to_disconnect_from_MQTT_broker )
{
    int dummy = 1;

    xTaskGetCurrentTaskHandle_fake.return_val = &dummy;
    xTaskNotifyWait_fake.return_val = 1;
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    xEventGroupClearBits_fake.return_val = 1;
    EXPECT_EQ( MQTTAgent_Disconnect_fake.call_count, 0 );
    prvDisconnectFromMQTTBroker();
    EXPECT_NE( MQTTAgent_Disconnect_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Disconnect_from_broker_tries_to_close_TCP_connection_if_successful )
{
    int dummy = 1;

    xTaskGetCurrentTaskHandle_fake.return_val = &dummy;
    xTaskNotifyWait_fake.return_val = 1;
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    xEventGroupClearBits_fake.return_val = 1;
    /* Successful mqtt closure. */
    MQTTAgent_Disconnect_fake.return_val = MQTTSuccess;
    EXPECT_EQ( Transport_Disconnect_fake.call_count, 0 );
    prvDisconnectFromMQTTBroker();
    EXPECT_NE( Transport_Disconnect_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Disconnect_from_broker_errors_on_failure_to_close_TCP_connection )
{
    int dummy = 1;

    xTaskGetCurrentTaskHandle_fake.return_val = &dummy;
    xTaskNotifyWait_fake.return_val = 1;
    xEventGroupClearBits_fake.return_val = 1;
    /* Fails to close mqtt connection. */
    MQTTAgent_Disconnect_fake.return_val = MQTTSuccess;
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_INVALID_PARAMETER;
    prvDisconnectFromMQTTBroker();
    expect_errors();
}
TEST_F( TestMqttAgentTask, Waits_for_MQTT_connection_to_close_before_trying_to_close_TCP_connection )
{
    int dummy = 1;

    xTaskGetCurrentTaskHandle_fake.return_val = &dummy;
    xTaskNotifyWait_fake.return_val = 1;
    xEventGroupClearBits_fake.return_val = 1;
    /* Closes mqtt connection but fails for TCP. */
    MQTTAgent_Disconnect_fake.return_val = MQTTSuccess;
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    EXPECT_EQ( xTaskNotifyWait_fake.call_count, 0 );
    prvDisconnectFromMQTTBroker();
    EXPECT_NE( xTaskNotifyWait_fake.call_count, 0 );

    /* Check this is not the case if MQTT closure fails. */
    RESET_FAKE( xTaskNotifyWait );
    MQTTAgent_Disconnect_fake.return_val = MQTTBadParameter;
    EXPECT_EQ( xTaskNotifyWait_fake.call_count, 0 ) << "Failed to reset fake.";
    EXPECT_THROW( prvDisconnectFromMQTTBroker();
                  EXPECT_EQ( xTaskNotifyWait_fake.call_count, 0 ), ASSERTION_FAIL );
}
TEST_F( TestMqttAgentTask, Disconnect_from_broker_errors_on_failure_to_close_MQTT_connection )
{
    int dummy = 1;

    xTaskGetCurrentTaskHandle_fake.return_val = &dummy;
    xTaskNotifyWait_fake.return_val = 1;
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    xEventGroupClearBits_fake.return_val = 1;
    /* Fails to close mqtt connection. */
    MQTTAgent_Disconnect_fake.return_val = MQTTBadParameter;
    Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
    EXPECT_THROW( prvDisconnectFromMQTTBroker(), ASSERTION_FAIL );
    expect_errors();
}

/* Testing prvMQTTConnect */

class TestMqttAgentTaskConnect : public TestMqttAgentTask {
public:
    TestMqttAgentTaskConnect()
    {
        /* The below may be overwritten within individual tests */
        MQTTAgent_CancelAll_fake.return_val = MQTTSuccess;
        MQTT_Connect_fake.return_val = MQTTSuccess;
        MQTTAgent_ResumeSession_fake.return_val = MQTTSuccess;
        xEventGroupSetBits_fake.return_val = 1;
        /* Make prvHandleResubscribe produce a defined output */
        int dummy = 1;
        xTaskGetCurrentTaskHandle_fake.return_val = &dummy;
        xTaskNotifyWait_fake.return_val = 1;
        Transport_Disconnect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
        xEventGroupClearBits_fake.return_val = 1;
        MQTTAgent_Disconnect_fake.return_val = MQTTSuccess;
    }
};
TEST_F( TestMqttAgentTaskConnect, MQTT_connect_tries_to_create_a_connection )
{
    EXPECT_EQ( MQTT_Connect_fake.call_count, 0 );
    prvMQTTConnect();
    EXPECT_NE( MQTT_Connect_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTaskConnect, MQTT_connect_returns_failure_if_connection_fails )
{
    MQTT_Connect_fake.return_val = MQTTBadParameter;
    EXPECT_NE( prvMQTTConnect(), MQTTSuccess );
}
TEST_F( TestMqttAgentTaskConnect, MQTT_connect_returns_success_if_connection_succeeds )
{
    MQTT_Connect_fake.return_val = MQTTSuccess;
    EXPECT_EQ( prvMQTTConnect(), MQTTSuccess );
}
/* This test would be ideal, but is not possible to write neatly for the file. */
/* TEST_F(TestMqttAgentTask, MQTT_connect_updates_system_flags_when_creating_a_new_connection) { */
/*     EXPECT_EQ(xEventGroupSetBits_fake.call_count, 0); */
/*     prvMQTTConnect(); */
/*     EXPECT_NE(xEventGroupSetBits_fake.call_count, 0); */
/* } */
TEST_F( TestMqttAgentTaskConnect, MQTT_connect_does_not_set_incorrect_system_flags_when_creating_a_new_connection )
{
    xEventGroupSetBits_fake.custom_fake = expect_mqtt_connected_event_mask;
    prvMQTTConnect();
}

/* Testing prvGetTimeMs */

/* Testing prvGetRandomNumber */

TEST_F( TestMqttAgentTask, Random_generation_calls_random_library_function )
{
    psa_generate_random_fake.return_val = PSA_SUCCESS;
    xTaskGetTickCount_fake.return_val = 3;
    EXPECT_EQ( psa_generate_random_fake.call_count, 0 );
    prvGetRandomNumber();
    EXPECT_NE( psa_generate_random_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Random_generation_does_not_error_if_random_library_function_succeeds )
{
    psa_generate_random_fake.return_val = PSA_SUCCESS;
    xTaskGetTickCount_fake.return_val = 3;
    prvGetRandomNumber();
    expect_no_errors();
}
TEST_F( TestMqttAgentTask, Random_generation_reports_library_failure )
{
    psa_generate_random_fake.return_val = PSA_ERROR_PROGRAMMER_ERROR;
    xTaskGetTickCount_fake.return_val = 3;
    prvGetRandomNumber();
    expect_errors_or_warnings();
}
TEST_F( TestMqttAgentTask, Random_generation_gives_same_output_for_same_value_given_by_random_library )
{
    /* Randomizing outputs should be handled by correct library functions only. */
    xTaskGetTickCount_fake.custom_fake = increment_shared_counter_and_return;
    psa_generate_random_fake.custom_fake = set_random_variable_to_three_and_return_success;
    UBaseType_t expected = prvGetRandomNumber();

    for( int count = 0; count < 5; count++ )
    {
        EXPECT_EQ( expected, prvGetRandomNumber() );
    }
}

/* Testing prvIncomingPublishCallback */

TEST_F( TestMqttAgentTask, Publish_callback_calls_publish_handler )
{
    bool handled = true;

    handleIncomingPublishes_fake.return_val = handled;

    EXPECT_EQ( handleIncomingPublishes_fake.call_count, 0 );
    int dummy = 5;
    MQTTAgentContext_t mqttAgentContext = { nullptr, &dummy };
    uint16_t dummyId = 10;
    MQTTPublishInfo_t xPublishInfo;
    xPublishInfo.qos = MQTTQoS0;
    xPublishInfo.retain = true;
    xPublishInfo.dup = true;
    xPublishInfo.pTopicName = "dummy";
    xPublishInfo.topicNameLength = 6;
    xPublishInfo.pPayload = &dummy;
    xPublishInfo.payloadLength = 1;
    prvIncomingPublishCallback( &mqttAgentContext, dummyId, &xPublishInfo );
    EXPECT_NE( handleIncomingPublishes_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Publish_callback_does_not_error_if_publish_handled )
{
    bool handled = true;

    handleIncomingPublishes_fake.return_val = handled;

    int dummy = 5;
    MQTTAgentContext_t mqttAgentContext = { nullptr, &dummy };
    uint16_t dummyId = 10;
    MQTTPublishInfo_t xPublishInfo;
    xPublishInfo.qos = MQTTQoS0;
    xPublishInfo.retain = true;
    xPublishInfo.dup = true;
    xPublishInfo.pTopicName = "dummy";
    xPublishInfo.topicNameLength = 6;
    xPublishInfo.pPayload = &dummy;
    xPublishInfo.payloadLength = 1;
    prvIncomingPublishCallback( &mqttAgentContext, dummyId, &xPublishInfo );
    expect_no_errors_or_warnings();
}
TEST_F( TestMqttAgentTask, Publish_callback_generates_log_if_handler_fails )
{
    bool handled = false;

    handleIncomingPublishes_fake.return_val = handled;

    int dummy = 5;
    MQTTAgentContext_t mqttAgentContext = { nullptr, &dummy };
    uint16_t dummyId = 10;
    MQTTPublishInfo_t xPublishInfo;
    xPublishInfo.qos = MQTTQoS0;
    xPublishInfo.retain = true;
    xPublishInfo.dup = true;
    xPublishInfo.pTopicName = "dummy";
    xPublishInfo.topicNameLength = 6;
    xPublishInfo.pPayload = &dummy;
    xPublishInfo.payloadLength = 1;
    prvIncomingPublishCallback( &mqttAgentContext, dummyId, &xPublishInfo );
    expect_errors_or_warnings();
}

/* Testing prvReSubscriptionCommandCallback */

TEST_F( TestMqttAgentTask, Command_callback_does_not_error_if_given_MQTT_success )
{
    /* All topic filters are already in subscription list. */
    MQTTAgentCommandContext_t pxCommandContext = {};
    uint8_t subackCodes[] = { 1, 2, 3 };
    MQTTAgentReturnInfo_t returnInfo = { MQTTSuccess, subackCodes };

    prvReSubscriptionCommandCallback( &pxCommandContext, &returnInfo );
    expect_no_errors();
}

/* Testing prvMQTTInit */

TEST_F( TestMqttAgentTask, MQTT_init_tries_to_create_a_command_queue )
{
    QueueDefinition queue = { 10 };

    xQueueCreateStatic_fake.return_val = &queue;
    MQTTAgent_Init_fake.return_val = MQTTSuccess;
    xEventGroupSetBits_fake.return_val = 1;

    EXPECT_EQ( xQueueCreateStatic_fake.call_count, 0 );
    prvMQTTInit();
    EXPECT_NE( xQueueCreateStatic_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, MQTT_init_errors_if_command_queue_creation_returns_nullptr )
{
    /* E.g. happens if out of memory. */
    xQueueCreateStatic_fake.return_val = nullptr;
    MQTTAgent_Init_fake.return_val = MQTTSuccess;
    xEventGroupSetBits_fake.return_val = 1;

    EXPECT_EQ( xQueueCreateStatic_fake.call_count, 0 );
    EXPECT_THROW( prvMQTTInit(), ASSERTION_FAIL );
    EXPECT_NE( xQueueCreateStatic_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, MQTT_init_tries_to_initialise_MQTT_library )
{
    QueueDefinition queue = { 10 };

    xQueueCreateStatic_fake.return_val = &queue;
    MQTTAgent_Init_fake.return_val = MQTTSuccess;
    xEventGroupSetBits_fake.return_val = 1;

    EXPECT_EQ( MQTTAgent_Init_fake.call_count, 0 );
    prvMQTTInit();
    EXPECT_NE( MQTTAgent_Init_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, MQTT_init_errors_if_cannot_initialise_MQTT_library )
{
    QueueDefinition queue = { 10 };

    xQueueCreateStatic_fake.return_val = &queue;
    MQTTAgent_Init_fake.return_val = MQTTBadParameter;
    xEventGroupSetBits_fake.return_val = 1;

    prvMQTTInit();
    expect_errors();
}
TEST_F( TestMqttAgentTask, MQTT_init_returns_success_if_successful )
{
    QueueDefinition queue = { 10 };

    xQueueCreateStatic_fake.return_val = &queue;
    MQTTAgent_Init_fake.return_val = MQTTSuccess;
    xEventGroupSetBits_fake.return_val = 1;

    EXPECT_EQ( prvMQTTInit(), MQTTSuccess );
}
TEST_F( TestMqttAgentTask, MQTT_init_sets_system_MQTT_init_event_flag_if_successful )
{
    QueueDefinition queue = { 10 };

    xQueueCreateStatic_fake.return_val = &queue;
    MQTTAgent_Init_fake.return_val = MQTTSuccess;
    xEventGroupSetBits_fake.return_val = 1;

    EXPECT_EQ( xEventGroupSetBits_fake.call_count, 0 );
    prvMQTTInit();
    EXPECT_NE( xEventGroupSetBits_fake.call_count, 0 );
}

/* Testing prvHandleResubscribe */
TEST_F( TestMqttAgentTask, Resubscribe_returns_success_if_subscription_succeeds )
{
    MQTTAgent_Subscribe_fake.return_val = MQTTSuccess;
    MQTT_Status_strerror_fake.return_val = "dummy";
    EXPECT_EQ( prvHandleResubscribe(), MQTTSuccess );
}

/* Testing prvDisconnectCommandCallback */

TEST_F( TestMqttAgentTask, Callback_for_MQTT_disconnect_tries_to_notify_task_waiting_on_MQTT_disconnection )
{
    xTaskNotify_fake.return_val = pdPASS;

    int dummyTask = 5;
    MQTTAgentCommandContext_t xCommandContext;
    xCommandContext.xTaskToNotify = &dummyTask;
    MQTTAgentReturnInfo_t xReturnInfo;
    xReturnInfo.returnCode = MQTTSuccess;

    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvDisconnectCommandCallback( &xCommandContext, &xReturnInfo );
    EXPECT_NE( xTaskNotify_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Callback_for_MQTT_disconnect_does_not_notify_if_no_tasks_are_waiting )
{
    xTaskNotify_fake.return_val = pdPASS;

    MQTTAgentCommandContext_t xCommandContext;
    xCommandContext.xTaskToNotify = nullptr;
    MQTTAgentReturnInfo_t xReturnInfo;
    xReturnInfo.returnCode = MQTTSuccess;

    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvDisconnectCommandCallback( &xCommandContext, &xReturnInfo );
    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTask, Callback_for_MQTT_disconnect_notifies_tasks_with_correct_MQTT_status_return_code )
{
    xTaskNotify_fake.custom_fake = return_pdpass_and_expect_mqtt_success_return_code;

    int dummyTask = 5;
    MQTTAgentCommandContext_t xCommandContext;
    xCommandContext.xTaskToNotify = &dummyTask;
    MQTTAgentReturnInfo_t xReturnInfo;
    xReturnInfo.returnCode = MQTTBadParameter;

    xTaskNotify_fake.custom_fake = return_pdpass_and_expect_mqtt_bad_parameter_return_code;
    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvDisconnectCommandCallback( &xCommandContext, &xReturnInfo );
    EXPECT_NE( xTaskNotify_fake.call_count, 0 );

    xReturnInfo.returnCode = MQTTBadParameter;
    prvDisconnectCommandCallback( &xCommandContext, &xReturnInfo );
}
TEST_F( TestMqttAgentTask, Callback_for_MQTT_disconnect_notifies_the_correct_task )
{
    xTaskNotify_fake.custom_fake = return_pdpass_and_expect_task_handle_points_to_five;

    int dummyTask = 5;
    MQTTAgentCommandContext_t xCommandContext;
    xCommandContext.xTaskToNotify = &dummyTask;
    MQTTAgentReturnInfo_t xReturnInfo;
    xReturnInfo.returnCode = MQTTSuccess;

    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvDisconnectCommandCallback( &xCommandContext, &xReturnInfo );
    EXPECT_NE( xTaskNotify_fake.call_count, 0 );
}

/* Testing prvMQTTAgentTask */

class TestMqttAgentTaskMainFunction : public TestMqttAgentTask {
public:
    TestMqttAgentTaskMainFunction()
    {
        /* So that MQTT can initialise */
        QueueDefinition queue = { 10 };

        xQueueCreateStatic_fake.return_val = &queue;
        MQTTAgent_Init_fake.return_val = MQTTSuccess;
        xEventGroupSetBits_fake.return_val = 1;
        /* So socket can connect  */
        Transport_Connect_fake.return_val = TRANSPORT_STATUS_SUCCESS;
        /* So random number generation succeeds */
        psa_generate_random_fake.return_val = PSA_SUCCESS;
        xTaskGetTickCount_fake.return_val = 3;
        /* So MQTT connection succeedds */
        MQTTAgent_CancelAll_fake.return_val = MQTTSuccess;
        MQTT_Connect_fake.return_val = MQTTSuccess;
        MQTTAgent_ResumeSession_fake.return_val = MQTTSuccess;
        int dummy = 1;
        xTaskGetCurrentTaskHandle_fake.return_val = &dummy;
        xTaskNotifyWait_fake.return_val = 1;
        xEventGroupClearBits_fake.return_val = 1;
        MQTTAgent_Disconnect_fake.return_val = MQTTSuccess;
        /* Main command loop does not usually terminate, but for these tests to terminate it needs to exit */
        MQTTAgent_CommandLoop_fake.return_val = MQTTSuccess;
        /* For the main function to have defined output. */
        BackoffAlgorithm_GetNextBackoff_fake.return_val = BackoffAlgorithmSuccess;
    }
};

TEST_F( TestMqttAgentTaskMainFunction, Agent_task_waits_for_network_to_start )
{
    EXPECT_EQ( vWaitUntilNetworkIsUp_fake.call_count, 0 );
    prvMQTTAgentTask( nullptr );
    EXPECT_NE( vWaitUntilNetworkIsUp_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTaskMainFunction, Agent_task_initialises_MQTT_library )
{
    EXPECT_EQ( MQTTAgent_Init_fake.call_count, 0 );
    prvMQTTAgentTask( nullptr );
    EXPECT_NE( MQTTAgent_Init_fake.call_count, 0 );
}
TEST_F( TestMqttAgentTaskMainFunction, Agent_task_cannot_continue_if_mqtt_initialisation_fails )
{
    /* MQTT needs a command queue and the agent to be intialised */
    QueueDefinition queue = { 10 };

    xQueueCreateStatic_fake.return_val = &queue;
    MQTTAgent_Init_fake.return_val = MQTTBadParameter;
    xEventGroupSetBits_fake.return_val = 1;

    MQTTAgent_Init_fake.return_val = MQTTBadParameter;

    EXPECT_THROW( prvMQTTAgentTask( nullptr ), ASSERTION_FAIL );
}
TEST_F( TestMqttAgentTaskMainFunction, Agent_task_initialises_MQTT_pool )
{
    EXPECT_EQ( Agent_InitializePool_fake.call_count, 0 );
    prvMQTTAgentTask( nullptr );
    EXPECT_NE( Agent_InitializePool_fake.call_count, 0 );
}
