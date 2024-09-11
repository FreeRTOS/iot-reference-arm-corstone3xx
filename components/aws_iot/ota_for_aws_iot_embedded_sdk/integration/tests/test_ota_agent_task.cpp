/* Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "fff.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace std;
using namespace std::placeholders;


extern "C" {
/* Header files for types used in this test file */
#include "application_version.h"
#include "core_mqtt_agent.h"
#include "core_mqtt_serializer.h"
#include "FreeRTOS.h"
#include "logging_stack.h"
#include "ota_mqtt_interface.h"
#include "ota_private.h"
#include "ota_config.h"
/* ota.h depends on ota_os_interface.h, which uses the reserved C++ keyword 'delete' */
#define delete    unreserved_delete
#include "ota.h"
#undef delete
#include "portmacro.h"
#include "semphr.h"
#include "subscription_manager.h"
#include "task.h"
#include "events.h"
/* File under test */
/* Functions usually defined in main.c */
DECLARE_FAKE_VOID_FUNC( vOtaActiveHook );
DEFINE_FAKE_VOID_FUNC( vOtaActiveHook );
DECLARE_FAKE_VOID_FUNC( vOtaNotActiveHook );
DEFINE_FAKE_VOID_FUNC( vOtaNotActiveHook );
/* Struct usually defined by the application. */
struct MQTTAgentCommandContext
{
    MQTTStatus_t xReturnStatus;
    TaskHandle_t xTaskToNotify;
    void * pArgs;
};
/* Static functions we are testing */
extern void vStartOtaTask( void );
extern OtaEventData_t * prvOTAEventBufferGet( void );
extern void prvOTAEventBufferFree( OtaEventData_t * const );
extern void prvOTAAgentTask( void * );
extern void vOtaDemoTask( void * );
extern BaseType_t prvRunOTADemo( void );
extern BaseType_t prvResumeOTA( void );
extern BaseType_t prvSuspendOTA( void );
extern void setOtaInterfaces( OtaInterfaces_t * );
extern OtaMqttStatus_t prvMQTTUnsubscribe( const char *,
                                           uint16_t,
                                           uint8_t );
extern OtaMqttStatus_t prvMQTTPublish( const char * const,
                                       uint16_t,
                                       const char *,
                                       uint32_t,
                                       uint8_t );
extern void prvOTAPublishCommandCallback( MQTTAgentCommandContext_t *,
                                          MQTTAgentReturnInfo_t * );
extern OtaMqttStatus_t prvMQTTSubscribe( const char *,
                                         uint16_t,
                                         uint8_t );
extern void prvMQTTUnsubscribeCompleteCallback( MQTTAgentCommandContext_t *,
                                                MQTTAgentReturnInfo_t * );
extern void prvMQTTSubscribeCompleteCallback( MQTTAgentCommandContext_t *,
                                              MQTTAgentReturnInfo_t * );
extern void prvMqttJobCallback( void *,
                                MQTTPublishInfo_t * );
extern void prvMqttDataCallback( void *,
                                 MQTTPublishInfo_t * );
extern void prvMqttDefaultCallback( void *,
                                    MQTTPublishInfo_t * );
extern void prvRegisterOTACallback( const char *,
                                    uint16_t );
extern void prvMqttDataCallback( void *,
                                 MQTTPublishInfo_t * );
extern void otaAppCallback( OtaJobEvent_t,
                            void * );
extern OtaEventData_t * prvOTAEventBufferGet( void );
extern void prvOTAEventBufferFree( OtaEventData_t * const );
/* Mocks of .h files in the same submodule as ota_agent_task.c that are also used by ota_agent_task.c */

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


/* Being under this test class denotes a valid test that needs a corresponding fix, but we do not want clogging the testsuite. */
class SkipTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        GTEST_SKIP() << "Skipping all tests under this suite";
    }
};

#define ASSERTION_FAILURE      1
/* used to indicate a context switch should have happened, but cannot be mocked. */
/* E.g. After Activating the OTA image we do not expect the otaAppCallback test code to continue running. */
#define CONTEXT_SWITCH_FAKE    2


/* Mock for vAssertCalled */
void throw_assertion_failure( const char * pcFile,
                              unsigned long ulLine )
{
    throw( ASSERTION_FAILURE ); /* stop tests running. */
}

/* Mock for SdkLogError */
void fail_and_display_error( const char * msg,
                             va_list args )
{
    char buf[ 1000 ];

    vsnprintf( buf, 1000, msg, args );
    FAIL() << "Failed because of unexpected error message: '" << buf << "' \n";
}
void do_nothing_if_an_error_occurs( const char * msg,
                                    va_list args ) /* do nothing */
{
}

/* Mock for xTaskNotifyWait */
BaseType_t stop_the_rest_of_the_program_running( int unused1,
                                                 int unused2,
                                                 void * unused3,
                                                 TickType_t unused4 )
{
    throw ( CONTEXT_SWITCH_FAKE );
    return pdPASS;
}

/* Mocks for MQTT_MatchTopic */
MQTTStatus_t set_match_true( const char * unused1,
                             const uint16_t unused2,
                             const char * unused3,
                             const uint16_t unused4,
                             bool * pIsMatch )
{
    *pIsMatch = true;
    return MQTTSuccess;
}
MQTTStatus_t set_match_false( const char * unused1,
                              const uint16_t unused2,
                              const char * unused3,
                              const uint16_t unused4,
                              bool * pIsMatch )
{
    *pIsMatch = false;
    return MQTTSuccess;
}

/* We use a global variable because it is not possible to use C++ std::function alongside C function pointers.
 * FFF uses C function pointers.
 * So it not possible to use partial function application via std::bind to pass a parameter to a mock. */
int global_counter = 0;
MQTTStatus_t match_while_global_counter_is_positive( const char * unused1,
                                                     const uint16_t unused2,
                                                     const char * unused3,
                                                     const uint16_t unused4,
                                                     bool * pIsMatch )
{
    if( global_counter > 0 )
    {
        *pIsMatch = true;
        global_counter = global_counter - 1;
    }
    else
    {
        *pIsMatch = false;
    }

    return MQTTSuccess;
}

/* Mock for OTA_SignalEvent */
bool expect_block_received_signal( const OtaEventMsg_t * const pEventMsg )
{
    EXPECT_EQ( pEventMsg->eventId, OtaAgentEventReceivedFileBlock );
    return true;
}

void expect_errors()
{
    EXPECT_NE( SdkLogError_fake.call_count, 0 );
}

/* Mock for MQTTAgent_Publish */
MQTTStatus_t set_return_status_to_mqtt_success( const MQTTAgentContext_t * pMqttAgentContext,
                                                MQTTPublishInfo_t * pPublishInfo,
                                                const MQTTAgentCommandInfo_t * pCommandInfo )
{
    MQTTAgentCommandContext_t * ctxt = ( MQTTAgentCommandContext_t * ) pCommandInfo->pCmdCompleteCallbackContext;

    ctxt->xReturnStatus = MQTTSuccess;
    return MQTTSuccess;
}
MQTTStatus_t set_return_status_to_mqtt_bad_parameter( const MQTTAgentContext_t * pMqttAgentContext,
                                                      MQTTPublishInfo_t * pPublishInfo,
                                                      const MQTTAgentCommandInfo_t * pCommandInfo )
{
    MQTTAgentCommandContext_t * ctxt = ( MQTTAgentCommandContext_t * ) pCommandInfo->pCmdCompleteCallbackContext;

    ctxt->xReturnStatus = MQTTBadParameter; /* the command returns failure */
    return MQTTSuccess;                     /* the publish itself succeeded. */
}

/* Mock for xTaskNotify */
BaseType_t expect_task_to_notify_is_five( TaskHandle_t handle,
                                          uint32_t unused1,
                                          eNotifyAction unused2 )
{
    EXPECT_EQ( *handle, 5 );
    return pdPASS;
}

/* Mock for MQTTAgent_Subscribe and MQTTAgent_Unsubscribe */
MQTTStatus_t fake_successful_subscription( const MQTTAgentContext_t * pMqttAgentContext,
                                           MQTTAgentSubscribeArgs_t * pSubscriptionArgs,
                                           const MQTTAgentCommandInfo_t * pCommandInfo )
{
    MQTTAgentCommandContext_t * ctxt = ( MQTTAgentCommandContext_t * ) pCommandInfo->pCmdCompleteCallbackContext;

    ctxt->xReturnStatus = MQTTSuccess;
    return MQTTSuccess;
}
MQTTStatus_t fake_bad_subscribe_parameter( const MQTTAgentContext_t * pMqttAgentContext,
                                           MQTTAgentSubscribeArgs_t * pSubscriptionArgs,
                                           const MQTTAgentCommandInfo_t * pCommandInfo )
{
    MQTTAgentCommandContext_t * ctxt = ( MQTTAgentCommandContext_t * ) pCommandInfo->pCmdCompleteCallbackContext;

    ctxt->xReturnStatus = MQTTBadParameter;
    return MQTTBadParameter;
}
MQTTStatus_t fake_subscribe_command_failure_only( const MQTTAgentContext_t * pMqttAgentContext,
                                                  MQTTAgentSubscribeArgs_t * pSubscriptionArgs,
                                                  const MQTTAgentCommandInfo_t * pCommandInfo )
{
    MQTTAgentCommandContext_t * ctxt = ( MQTTAgentCommandContext_t * ) pCommandInfo->pCmdCompleteCallbackContext;

    ctxt->xReturnStatus = MQTTBadParameter;
    return MQTTSuccess;
}

/* Mock for MQTTAgent_Publish */
MQTTStatus_t fake_successful_publish( const MQTTAgentContext_t * pMqttAgentContext,
                                      MQTTPublishInfo_t * pPublishInfo,
                                      const MQTTAgentCommandInfo_t * pCommandInfo )
{
    MQTTAgentCommandContext_t * ctxt = ( MQTTAgentCommandContext_t * ) pCommandInfo->pCmdCompleteCallbackContext;

    ctxt->xReturnStatus = MQTTSuccess;
    return MQTTSuccess;
}
MQTTStatus_t fake_bad_publish_parameter( const MQTTAgentContext_t * pMqttAgentContext,
                                         MQTTPublishInfo_t * pPublishInfo,
                                         const MQTTAgentCommandInfo_t * pCommandInfo )
{
    MQTTAgentCommandContext_t * ctxt = ( MQTTAgentCommandContext_t * ) pCommandInfo->pCmdCompleteCallbackContext;

    ctxt->xReturnStatus = MQTTBadParameter;
    return MQTTBadParameter;
}

/* Mock for OTA_Suspend */
OtaErr_t fake_successful_ota_suspend( void )
{
    OTA_GetState_fake.return_val = OtaAgentStateSuspended;
    return OtaErrNone;
}
OtaErr_t fake_unsuccessful_ota_suspend( void )
{
    OTA_GetState_fake.return_val = OtaAgentStateWaitingForFileBlock;
    return OtaErrUninitialized;
}
OtaErr_t fake_successful_ota_suspend_but_state_is_incorrect( void )
{
    OTA_GetState_fake.return_val = OtaAgentStateShuttingDown;
    return OtaErrNone;
}

/* Mock for OTA_Resume */
OtaErr_t fake_successful_ota_resume( void )
{
    OTA_GetState_fake.return_val = OtaAgentStateWaitingForFileBlock;
    return OtaErrNone;
}
OtaErr_t fake_unsuccessful_ota_resume( void )
{
    OTA_GetState_fake.return_val = OtaAgentStateSuspended;
    return OtaErrUninitialized;
}
OtaErr_t fake_successful_ota_resume_but_state_is_still_suspended( void )
{
    OTA_GetState_fake.return_val = OtaAgentStateSuspended;
    return OtaErrNone;
}

class TestOtaAgentTask : public ::testing::Test {
public:
    TestOtaAgentTask()
    {
        RESET_FAKE( addSubscription );
        RESET_FAKE( GetImageVersionPSA );
        RESET_FAKE( MQTTAgent_Subscribe );
        RESET_FAKE( MQTTAgent_Publish );
        RESET_FAKE( MQTTAgent_Unsubscribe );
        RESET_FAKE( MQTTAgent_Subscribe );
        RESET_FAKE( MQTT_MatchTopic );
        RESET_FAKE( OTA_ActivateNewImage );
        RESET_FAKE( OTA_EventProcessingTask );
        RESET_FAKE( OTA_GetState );
        RESET_FAKE( OTA_Init );
        RESET_FAKE( OTA_Resume );
        RESET_FAKE( OTA_SetImageState );
        RESET_FAKE( OTA_Shutdown );
        RESET_FAKE( OTA_SignalEvent );
        RESET_FAKE( OTA_Suspend );
        RESET_FAKE( removeSubscription );
        RESET_FAKE( SdkLogError );
        RESET_FAKE( SdkLogWarn );
        RESET_FAKE( SdkLogDebug );
        RESET_FAKE( vAssertCalled );
        RESET_FAKE( vOtaActiveHook );
        RESET_FAKE( vOtaNotActiveHook );
        RESET_FAKE( vTaskDelay );
        RESET_FAKE( vSemaphoreDelete );
        RESET_FAKE( vWaitUntilMQTTAgentConnected );
        RESET_FAKE( vWaitUntilMQTTAgentReady );
        RESET_FAKE( xIsMqttAgentConnected );
        RESET_FAKE( xSemaphoreCreateMutex );
        RESET_FAKE( xSemaphoreTake );
        RESET_FAKE( xSemaphoreGive );
        RESET_FAKE( xTaskGetCurrentTaskHandle );
        RESET_FAKE( xTaskNotify )
        RESET_FAKE( xTaskNotifyStateClear );
        RESET_FAKE( xTaskNotifyWait );
        RESET_FAKE( xTaskCreate );

        /* Default values.
         * Do not overwrite these here, instead use inheritance or
         * overwrite the mock return values in individual tests. */
        addSubscription_fake.return_val = true; /* success. */
        MQTTAgent_Subscribe_fake.custom_fake = fake_successful_subscription;
        MQTTAgent_Publish_fake.custom_fake = fake_successful_publish;
        MQTTAgent_Unsubscribe_fake.return_val = MQTTSuccess;
        MQTTAgent_Subscribe_fake.return_val = MQTTSuccess;
        MQTT_MatchTopic_fake.return_val = MQTTSuccess;
        OTA_ActivateNewImage_fake.return_val = OtaErrNone;
        OTA_GetState_fake.return_val = OtaAgentStateReady;
        OTA_Init_fake.return_val = OtaErrNone;
        OTA_SetImageState_fake.return_val = OtaErrNone;
        OTA_Shutdown_fake.return_val = OtaAgentStateShuttingDown;
        OTA_SignalEvent_fake.return_val = true;
        OTA_Suspend_fake.custom_fake = fake_successful_ota_suspend;
        OTA_Resume_fake.custom_fake = fake_successful_ota_resume;
        xIsMqttAgentConnected_fake.return_val = true;
        xTaskGetCurrentTaskHandle_fake.return_val = nullptr;
        xTaskNotify_fake.return_val = pdPASS;
        xTaskNotifyStateClear_fake.return_val = pdTRUE;
        xTaskNotifyWait_fake.return_val = pdPASS;

        /*
         * These default semaphore values are relied on by nearly all tests.
         * Do not change them here.
         */
        xSemaphoreTake_fake.return_val = pdTRUE;
        xSemaphoreGive_fake.return_val = pdTRUE;
        xTaskCreate_fake.return_val = pdPASS;

        /*
         * Wrap functions expected to fail an assertion in EXPECT_THROW from GoogleTest.
         * Overwrite these in the test if you expect a test's inputs to cause an error.
         * Note that overwriting a custom fake requires another custom fake, setting return_val
         * is not sufficient.
         */
        vAssertCalled_fake.custom_fake = throw_assertion_failure;
        /* assume every unexpected error log is a problem. */
        SdkLogError_fake.custom_fake = fail_and_display_error;
    }
};

/* Mock for OTA_SetImageState */
OtaErr_t should_set_image_to_accepted_state( OtaImageState_t state )
{
    /* Assume the image is valid. */
    EXPECT_EQ( state, OtaImageStateAccepted );
    return OtaErrNone;
}

/* The  file under test contains static functions which the tests in this file assume are made visible
 * by conditional compiling macros. This test verifies these macros are defined. */
TEST_F( TestOtaAgentTask, can_test_static_functions )
{
    #ifndef UNIT_TESTING
        FAIL() << "The macro UNIT_TESTING is not defined, please add this to your CMake compile definitions.";
    #endif /* UNIT_TESTING */
}

/* Test prvOTAEventBufferFree */
TEST_F( TestOtaAgentTask, freeing_ota_event_buffer_frees_buffer )
{
    OtaEventData_t pxBuffer = { 0 };

    pxBuffer.bufferUsed = true;
    prvOTAEventBufferFree( &pxBuffer );
    EXPECT_FALSE( pxBuffer.bufferUsed );
}

/* Test prvOTAEventBufferGet */
TEST_F( TestOtaAgentTask, buffer_size_is_sufficient_for_these_tests_to_run )
{
    /* must have at least 2 buffers to for these tests to run. */
    EXPECT_GE( otaconfigMAX_NUM_OTA_DATA_BUFFERS, 2 );
}

TEST_F( TestOtaAgentTask, getting_a_single_buffer_does_not_return_nullpointer )
{
    OtaEventData_t * buffer = prvOTAEventBufferGet();

    EXPECT_NE( buffer, nullptr );
}

TEST_F( TestOtaAgentTask, the_agent_eventually_runs_out_of_data_buffers )
{
    OtaEventData_t * buffer;

    for( int i = 0; i < otaconfigMAX_NUM_OTA_DATA_BUFFERS + 10; i++ )
    {
        buffer = prvOTAEventBufferGet();
    }

    EXPECT_EQ( buffer, nullptr );
}

TEST_F( TestOtaAgentTask, tests_do_not_interfere_with_each_other )
{
    OtaEventData_t * buffer = prvOTAEventBufferGet();

    EXPECT_NE( buffer, nullptr );
}

TEST_F( TestOtaAgentTask, get_uses_all_available_buffers )
{
    /* This test failing will mean some subsequent tests fail. */
    OtaEventData_t * buffer;

    for( int i = 0; i < otaconfigMAX_NUM_OTA_DATA_BUFFERS - 1; i++ )
    {
        buffer = prvOTAEventBufferGet();
    }

    EXPECT_NE( buffer, nullptr );
    buffer = prvOTAEventBufferGet();
    EXPECT_NE( buffer, nullptr );
    buffer = prvOTAEventBufferGet();
    EXPECT_EQ( buffer, nullptr );
}

TEST_F( TestOtaAgentTask, getting_buffer_marks_it_as_in_use )
{
    OtaEventData_t * buffer = prvOTAEventBufferGet();

    EXPECT_TRUE( buffer->bufferUsed );
}

TEST_F( TestOtaAgentTask, freeing_a_buffer_marks_it_as_out_of_use )
{
    OtaEventData_t * buffer = prvOTAEventBufferGet();

    EXPECT_TRUE( buffer->bufferUsed );
    prvOTAEventBufferFree( buffer );
    EXPECT_FALSE( buffer->bufferUsed );
}

TEST_F( TestOtaAgentTask, buffers_are_reused )
{
    /* Freeing then getting all buffers marks the initially freed buffer as in use */
    OtaEventData_t * buffer = prvOTAEventBufferGet();

    EXPECT_TRUE( buffer->bufferUsed );
    prvOTAEventBufferFree( buffer );
    EXPECT_FALSE( buffer->bufferUsed );

    for( int i = 0; i < otaconfigMAX_NUM_OTA_DATA_BUFFERS; i++ )
    {
        OtaEventData_t * unused = prvOTAEventBufferGet();
    }

    EXPECT_TRUE( buffer->bufferUsed );
}

TEST_F( TestOtaAgentTask, freeing_a_buffer_causes_previously_failing_buffer_get_to_succeed )
{
    OtaEventData_t * buffer_1;

    for( int i = 0; i < otaconfigMAX_NUM_OTA_DATA_BUFFERS; i++ )
    {
        buffer_1 = prvOTAEventBufferGet();
        EXPECT_NE( buffer_1, nullptr );
    }

    OtaEventData_t * buffer_2 = prvOTAEventBufferGet();
    EXPECT_EQ( buffer_2, nullptr );
    /* Free the last buffer that was claimed. */
    prvOTAEventBufferFree( buffer_1 );
    /* Try to claim a buffer again. Expect to succeed this time. */
    buffer_2 = prvOTAEventBufferGet();
    EXPECT_NE( buffer_2, nullptr ) << "Expected to a buffer to become available after freeing";
}

/* Mock for OTA_ActivateImage */
OtaErr_t stop_running_test_if_image_activated( void )
{
    throw CONTEXT_SWITCH_FAKE;
    return OtaErrNone;
}

/* otaAppCallback */
TEST_F( TestOtaAgentTask, agent_activates_image_if_image_is_authenticated_and_ready_to_activate )
{
    OTA_ActivateNewImage_fake.custom_fake = stop_running_test_if_image_activated;
    OTA_Shutdown_fake.return_val = OtaAgentStateShuttingDown;
    OTA_SetImageState_fake.return_val = OtaErrNone;
    OTA_GetState_fake.return_val = OtaAgentStateReady;
    EXPECT_EQ( OTA_ActivateNewImage_fake.call_count, 0 );
    try {
        otaAppCallback( OtaJobEventActivate, nullptr );
    }
    catch( int num ) {
        if( num != CONTEXT_SWITCH_FAKE )
        {
            throw num;
        }
    }
    EXPECT_NE( OTA_ActivateNewImage_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, agent_does_not_try_to_activate_image_if_update_rejected )
{
    OTA_ActivateNewImage_fake.return_val = OtaErrNone;
    OTA_Shutdown_fake.return_val = OtaAgentStateShuttingDown;
    OTA_SetImageState_fake.return_val = OtaErrNone;
    OTA_GetState_fake.return_val = OtaAgentStateReady;
    EXPECT_EQ( OTA_ActivateNewImage_fake.call_count, 0 );
    otaAppCallback( OtaJobEventFail, nullptr );
    EXPECT_EQ( OTA_ActivateNewImage_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, agent_frees_event_buffer_once_event_is_processed )
{
    OtaEventData_t * buffer = prvOTAEventBufferGet();

    EXPECT_TRUE( buffer->bufferUsed );
    otaAppCallback( OtaJobEventProcessed, buffer );
    EXPECT_FALSE( buffer->bufferUsed );
}
TEST_F( TestOtaAgentTask, agent_calls_not_active_hook_when_job_received )
{
    EXPECT_EQ( vOtaActiveHook_fake.call_count, 0 );
    otaAppCallback( OtaJobEventReceivedJob, nullptr );
    EXPECT_NE( vOtaActiveHook_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, agent_calls_active_hook_when_no_job_is_received )
{
    EXPECT_EQ( vOtaNotActiveHook_fake.call_count, 0 );
    otaAppCallback( OtaJobEventNoActiveJob, nullptr );
    EXPECT_NE( vOtaNotActiveHook_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, agent_tries_to_set_image_state_when_starting_test )
{
    EXPECT_EQ( OTA_SetImageState_fake.call_count, 0 );
    otaAppCallback( OtaJobEventStartTest, nullptr );
    EXPECT_NE( OTA_SetImageState_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, agent_tries_to_set_image_state_as_accepted_when_starting_test )
{
    /* Agent marks valid image as accepted */
    EXPECT_EQ( OTA_SetImageState_fake.call_count, 0 );
    otaAppCallback( OtaJobEventStartTest, nullptr );
    OTA_SetImageState_fake.custom_fake = should_set_image_to_accepted_state;
}
TEST_F( TestOtaAgentTask, agent_shuts_down_ota_with_error_message_if_self_test_is_failed )
{
    /* 'Self test' refers to validating the image using signatures, and checking */
    /* that the version number has increased. */
    /* For more information, see AWS OTA documentation. */
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    EXPECT_EQ( SdkLogError_fake.call_count, 0 );
    EXPECT_EQ( OTA_Shutdown_fake.call_count, 0 );
    otaAppCallback( OtaJobEventSelfTestFailed, nullptr );
    EXPECT_NE( OTA_Shutdown_fake.call_count, 0 );
    EXPECT_NE( SdkLogError_fake.call_count, 0 );
}

/* prvMqttJobCallback */
TEST_F( TestOtaAgentTask, job_callback_does_not_error_for_a_normal_call )
{
    int payload = 15;
    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, 1 };

    prvMqttJobCallback( nullptr, &pubInfo );
}
TEST_F( TestOtaAgentTask, job_callback_tries_to_send_event_for_valid_data )
{
    /* Tries to send job document received event when job is done */
    EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
    int payload = 15;
    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, 1 };
    prvMqttJobCallback( nullptr, &pubInfo );
    EXPECT_NE( OTA_SignalEvent_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, job_callback_does_not_try_to_send_event_for_too_large_a_mqtt_payload )
{
    /* This needs to be sanitised because the user controls the payload. */
    /* This test tries to get the job callback to seg fault. E.g. by bad use of memcpy. */
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    try {
        EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
        uint32_t * largePayload[ 1000 ];
        MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, ( void * ) largePayload, 1000 };
        prvMqttJobCallback( nullptr, &pubInfo );
        EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
    }
    catch( int num ) {
        if( num == ASSERTION_FAILURE )
        {
            EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
        }
        else
        {
            throw ( num );
        }
    }
}
TEST_F( TestOtaAgentTask, job_callback_does_not_send_event_signal_again_if_signal_was_received_correctly )
{
    OTA_SignalEvent_fake.return_val = true; /* success */
    EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
    int payload = 15;
    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, 1 };
    prvMqttJobCallback( nullptr, &pubInfo );
    EXPECT_EQ( OTA_SignalEvent_fake.call_count, 1 );
}

/* prvMqttDefaultCallback */

TEST_F( TestOtaAgentTask, mqtt_default_handler_redirects_publishes_to_handlers_based_on_topics )
{
    int payload = 15;
    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, 1 };

    EXPECT_EQ( MQTT_MatchTopic_fake.call_count, 0 );
    prvMqttDefaultCallback( nullptr, &pubInfo );
    EXPECT_NE( MQTT_MatchTopic_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, mqtt_default_handler_handles_very_large_data_packet )
{
    /* We are trying to break bad use of memcpy in this test. */
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    size_t tooLargeForBuffer = 1000; /* probably too big for the mqtt buffer. */
    int payload[ tooLargeForBuffer ];

    for( int i = 0; i < tooLargeForBuffer; i++ )
    {
        payload[ i ] = i;
    }

    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, tooLargeForBuffer };
    MQTT_MatchTopic_fake.custom_fake = set_match_true;
    prvMqttDefaultCallback( nullptr, &pubInfo );
    MQTT_MatchTopic_fake.custom_fake = set_match_false;
    prvMqttDefaultCallback( nullptr, &pubInfo );
    /* Do not SegFault either way. */
}

/* prvMqttDataCallback */

TEST_F( TestOtaAgentTask, mqtt_data_handler_signals_if_successful )
{
    EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
    int payload = 12;
    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, 1 };
    prvMqttDataCallback( nullptr, &pubInfo );
    EXPECT_NE( OTA_SignalEvent_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, mqtt_data_handler_does_not_segfault_on_large_data_packets )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    size_t tooLargeForBuffer = 1000; /* probably too big for the mqtt buffer. */
    int payload[ tooLargeForBuffer ];

    for( int i = 0; i < tooLargeForBuffer; i++ )
    {
        payload[ i ] = i;
    }

    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, tooLargeForBuffer };
    prvMqttDataCallback( nullptr, &pubInfo );

    if( SdkLogError_fake.call_count > 0 )
    {
        /* then expect no event signal to have been sent. */
        EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
    }
}
TEST_F( TestOtaAgentTask, mqtt_data_handler_signals_file_block_received_if_successful )
{
    int payload = 12;
    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, 1 };

    prvMqttDataCallback( nullptr, &pubInfo );
    OTA_SignalEvent_fake.custom_fake = expect_block_received_signal;
}
TEST_F( TestOtaAgentTask, mqtt_data_handler_errors_if_out_of_ota_data_buffers )
{
    for( int i = 0; i < otaconfigMAX_NUM_OTA_DATA_BUFFERS; i++ )
    {
        OtaEventData_t * buffer = prvOTAEventBufferGet();
        EXPECT_NE( buffer, nullptr );
    }

    OtaEventData_t * buffer = prvOTAEventBufferGet();
    EXPECT_EQ( buffer, nullptr );
    /* Out of data buffers */
    int payload = 15;
    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, 1 };
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    prvMqttDataCallback( nullptr, &pubInfo );
    expect_errors();
    EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, mqtt_data_handler_does_not_send_signal_if_out_of_ota_data_buffers )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs; /* being out of buffers will cause errors. */
    OtaEventData_t * buffer;

    for( int i = 0; i < otaconfigMAX_NUM_OTA_DATA_BUFFERS + 10; i++ )
    {
        buffer = prvOTAEventBufferGet();
    }

    EXPECT_EQ( buffer, nullptr );
    int payload = 15;
    MQTTPublishInfo_t pubInfo = { MQTTQoS0, true, true, "topic name", 11, &payload, 1 };
    prvMqttDataCallback( nullptr, &pubInfo );
    EXPECT_EQ( OTA_SignalEvent_fake.call_count, 0 );
}

/* prvRegisterOTACallback
 * Is a callback for receiving messages intended for the OTA agent, from the broker.
 */

TEST_F( TestOtaAgentTask, the_ota_callback_does_not_add_a_subscription_if_no_topics_match )
{
    MQTT_MatchTopic_fake.custom_fake = set_match_false;
    prvRegisterOTACallback( "dummy", 6 );
    EXPECT_EQ( addSubscription_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, the_ota_callback_adds_a_subscription_if_topics_match )
{
    MQTT_MatchTopic_fake.custom_fake = set_match_true;
    prvRegisterOTACallback( "dummy", 6 );
    EXPECT_NE( addSubscription_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, the_ota_callback_adds_a_subscription_per_matching_topic )
{
    /* We also assume there are at least 2 matching topics for this test. */
    global_counter = 2;
    int initial_counter_value = global_counter;
    MQTT_MatchTopic_fake.custom_fake = match_while_global_counter_is_positive;
    prvRegisterOTACallback( "dummy", 6 );
    EXPECT_EQ( addSubscription_fake.call_count, initial_counter_value );
}

/* prvMQTTSubscribeCompleteCallback */

TEST_F( TestOtaAgentTask, command_return_code_is_stored_in_mqtt_return_info )
{
    MQTTAgentReturnInfo_t returnInfo;
    MQTTSubscribeInfo_t info;

    info.pTopicFilter = "dummy";
    info.topicFilterLength = 6;
    info.qos = MQTTQoS0;
    MQTTAgentSubscribeArgs_t subscribeArgs;
    subscribeArgs.pSubscribeInfo = &info;
    MQTTAgentCommandContext_t cmndCtxt;
    cmndCtxt.pArgs = &subscribeArgs;
    returnInfo.returnCode = MQTTSuccess;
    prvMQTTSubscribeCompleteCallback( &cmndCtxt, &returnInfo );
    EXPECT_EQ( cmndCtxt.xReturnStatus, returnInfo.returnCode );
    returnInfo.returnCode = MQTTBadParameter;
    prvMQTTSubscribeCompleteCallback( &cmndCtxt, &returnInfo );
    EXPECT_EQ( cmndCtxt.xReturnStatus, returnInfo.returnCode );
}
TEST_F( TestOtaAgentTask, if_there_is_a_task_to_notify_a_task_is_notified )
{
    int task = 5;
    MQTTAgentReturnInfo_t returnInfo;
    MQTTSubscribeInfo_t info;

    info.pTopicFilter = "dummy";
    info.topicFilterLength = 6;
    info.qos = MQTTQoS0;
    MQTTAgentSubscribeArgs_t subscribeArgs;
    subscribeArgs.pSubscribeInfo = &info;
    MQTTAgentCommandContext_t cmndCtxt;
    cmndCtxt.pArgs = &subscribeArgs;
    cmndCtxt.xTaskToNotify = &task;
    returnInfo.returnCode = MQTTSuccess;
    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvMQTTSubscribeCompleteCallback( &cmndCtxt, &returnInfo );
    EXPECT_NE( xTaskNotify_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, if_there_is_a_task_to_notify_the_correct_task_is_notified )
{
    int task = 5; /* do not change this without modifying the fake for xTaskNotify */
    MQTTAgentReturnInfo_t returnInfo;
    MQTTSubscribeInfo_t info;

    info.pTopicFilter = "dummy";
    info.topicFilterLength = 6;
    info.qos = MQTTQoS0;
    MQTTAgentSubscribeArgs_t subscribeArgs;
    subscribeArgs.pSubscribeInfo = &info;
    MQTTAgentCommandContext_t cmndCtxt;
    cmndCtxt.pArgs = &subscribeArgs;
    cmndCtxt.xTaskToNotify = &task;
    returnInfo.returnCode = MQTTSuccess;
    xTaskNotify_fake.custom_fake = expect_task_to_notify_is_five;
    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvMQTTSubscribeCompleteCallback( &cmndCtxt, &returnInfo );
    EXPECT_NE( xTaskNotify_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, if_there_is_no_task_then_task_notify_is_not_called )
{
    MQTTAgentReturnInfo_t returnInfo;
    MQTTSubscribeInfo_t info;

    info.pTopicFilter = "dummy";
    info.topicFilterLength = 6;
    info.qos = MQTTQoS0;
    MQTTAgentSubscribeArgs_t subscribeArgs;
    subscribeArgs.pSubscribeInfo = &info;
    MQTTAgentCommandContext_t cmndCtxt;
    cmndCtxt.pArgs = &subscribeArgs;
    cmndCtxt.xTaskToNotify = nullptr;
    returnInfo.returnCode = MQTTSuccess;
    prvMQTTSubscribeCompleteCallback( &cmndCtxt, &returnInfo );
    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
}

/* prvMQTTUnsubscribeCompleteCallback */

/* These tests duplicate code because it is poor practice to share variables between unit tests. */
TEST_F( TestOtaAgentTask, command_return_code_is_stored_in_mqtt_return_info_2 )
{
    MQTTAgentReturnInfo_t returnInfo;
    MQTTSubscribeInfo_t info;

    info.pTopicFilter = "dummy";
    info.topicFilterLength = 6;
    info.qos = MQTTQoS0;
    MQTTAgentSubscribeArgs_t subscribeArgs;
    subscribeArgs.pSubscribeInfo = &info;
    MQTTAgentCommandContext_t cmndCtxt;
    cmndCtxt.pArgs = &subscribeArgs;
    returnInfo.returnCode = MQTTSuccess;
    prvMQTTUnsubscribeCompleteCallback( &cmndCtxt, &returnInfo );
    EXPECT_EQ( cmndCtxt.xReturnStatus, returnInfo.returnCode );
    returnInfo.returnCode = MQTTBadParameter;
    prvMQTTUnsubscribeCompleteCallback( &cmndCtxt, &returnInfo );
    EXPECT_EQ( cmndCtxt.xReturnStatus, returnInfo.returnCode );
}
TEST_F( TestOtaAgentTask, if_there_is_a_task_to_notify_this_task_is_notified_2 )
{
    int task = 5;
    MQTTAgentReturnInfo_t returnInfo;
    MQTTSubscribeInfo_t info;

    info.pTopicFilter = "dummy";
    info.topicFilterLength = 6;
    info.qos = MQTTQoS0;
    MQTTAgentSubscribeArgs_t subscribeArgs;
    subscribeArgs.pSubscribeInfo = &info;
    MQTTAgentCommandContext_t cmndCtxt;
    cmndCtxt.pArgs = &subscribeArgs;
    cmndCtxt.xTaskToNotify = &task;
    returnInfo.returnCode = MQTTSuccess;
    xTaskNotify_fake.custom_fake = expect_task_to_notify_is_five;
    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvMQTTUnsubscribeCompleteCallback( &cmndCtxt, &returnInfo );
    EXPECT_NE( xTaskNotify_fake.call_count, 0 );
}

/* prvMQTTSubscribe */

TEST_F( TestOtaAgentTask, mqtt_subscribe_subscribes_to_a_topic )
{
    EXPECT_EQ( MQTTAgent_Subscribe_fake.call_count, 0 );
    prvMQTTSubscribe( "dummy", 6, 0 );
    EXPECT_NE( MQTTAgent_Subscribe_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, mqtt_subscribe_asks_for_the_current_task_to_be_notified )
{
    int this_task = 5;

    xTaskGetCurrentTaskHandle_fake.return_val = &this_task;
    xTaskNotify_fake.custom_fake = expect_task_to_notify_is_five;
    EXPECT_EQ( xTaskGetCurrentTaskHandle_fake.call_count, 0 );
    prvMQTTSubscribe( "dummy", 6, 0 );
    EXPECT_NE( xTaskGetCurrentTaskHandle_fake.call_count, 0 );
}
/* We do not test this function for null pointer inputs */
/* because the topic filter not being null could be taken as a */
/* pre-condition. */
TEST_F( TestOtaAgentTask, mqtt_subscribe_returns_success_on_successful_subscription )
{
    int this_task = 8;

    xTaskGetCurrentTaskHandle_fake.return_val = &this_task;
    MQTTAgent_Subscribe_fake.custom_fake = fake_successful_subscription;
    EXPECT_EQ( prvMQTTSubscribe( "dummy", 6, 0 ), OtaMqttSuccess );
}
TEST_F( TestOtaAgentTask, mqtt_subscribe_returns_failure_on_unsuccessful_subscription )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    int this_task = 8;
    xTaskGetCurrentTaskHandle_fake.return_val = &this_task;
    MQTTAgent_Subscribe_fake.custom_fake = fake_bad_subscribe_parameter;
    EXPECT_NE( prvMQTTSubscribe( "dummy", 6, 0 ), OtaMqttSuccess );
}

/* prvOTAPublishCommandCallback */
TEST_F( TestOtaAgentTask, publish_callback_sets_return_code )
{
    MQTTStatus_t expectedReturnStatus = MQTTSuccess;
    MQTTAgentCommandContext_t cmndCtxt = { MQTTBadParameter };
    MQTTAgentReturnInfo_t retInfo = { expectedReturnStatus };

    EXPECT_NE( cmndCtxt.xReturnStatus, expectedReturnStatus );
    prvOTAPublishCommandCallback( &cmndCtxt, &retInfo );
    EXPECT_EQ( cmndCtxt.xReturnStatus, expectedReturnStatus );
    /* Sasme functionality tested, different arguments. */
    expectedReturnStatus = MQTTRecvFailed;
    retInfo = { expectedReturnStatus };
    prvOTAPublishCommandCallback( &cmndCtxt, &retInfo );
    EXPECT_EQ( cmndCtxt.xReturnStatus, expectedReturnStatus );
}
TEST_F( TestOtaAgentTask, publish_callback_notifies_task_if_task_to_notify_is_set )
{
    int task = 5;

    xTaskNotify_fake.return_val = pdPASS;
    MQTTAgentCommandContext_t cmndCtxt = { MQTTBadParameter, &task };
    MQTTAgentReturnInfo_t retInfo = { MQTTSuccess };
    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvOTAPublishCommandCallback( &cmndCtxt, &retInfo );
    EXPECT_NE( xTaskNotify_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, publish_callback_does_not_notify_task_if_task_to_notify_is_not_set )
{
    MQTTAgentCommandContext_t cmndCtxt = { MQTTBadParameter, nullptr };
    MQTTAgentReturnInfo_t retInfo = { MQTTSuccess };

    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
    prvOTAPublishCommandCallback( &cmndCtxt, &retInfo );
    EXPECT_EQ( xTaskNotify_fake.call_count, 0 );
}

/* prvMQTTPublish
 * Handles sending a publish via MQTT
 */

TEST_F( TestOtaAgentTask, publishing_calls_mqtt_publish_api )
{
    EXPECT_EQ( MQTTAgent_Publish_fake.call_count, 0 );
    prvMQTTPublish( "dummy1", 7, "dummymsg", 9, MQTTQoS0 );
    EXPECT_NE( MQTTAgent_Publish_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, publishing_waits_on_mqtt_publish_to_finish_before_it_can_return_result_of_the_publish )
{
    EXPECT_EQ( xTaskNotifyWait_fake.call_count, 0 );
    prvMQTTPublish( "dummy1", 7, "dummymsg", 9, MQTTQoS1 );
    EXPECT_NE( xTaskNotifyWait_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, publishing_returns_failure_if_mqtt_publish_fails )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    MQTTAgent_Publish_fake.custom_fake = fake_bad_publish_parameter;
    EXPECT_EQ( prvMQTTPublish( "dummy1", 7, "dummymsg", 9, MQTTQoS1 ), OtaMqttPublishFailed );
}
TEST_F( TestOtaAgentTask, publishing_returns_failure_if_waiting_on_the_publish_command_fails )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    /* i.e. xTaskNotifyWait returns failure, so notification has not happened. */
    xTaskNotifyWait_fake.return_val = pdFAIL;
    EXPECT_EQ( prvMQTTPublish( "dummy1", 7, "dummymsg", 9, MQTTQoS1 ), OtaMqttPublishFailed );
}
TEST_F( TestOtaAgentTask, publishing_returns_success_if_mqtt_publish_succeeds )
{
    MQTTAgent_Publish_fake.custom_fake = set_return_status_to_mqtt_success;
    EXPECT_EQ( prvMQTTPublish( "dummy1", 7, "dummymsg", 9, MQTTQoS1 ), OtaMqttSuccess );
}
TEST_F( TestOtaAgentTask, publishing_returns_failure_if_scheduling_command_via_mqtt_publish_succeeds_but_the_command_fails )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    MQTTAgent_Publish_fake.custom_fake = set_return_status_to_mqtt_bad_parameter;
    EXPECT_EQ( prvMQTTPublish( "dummy1", 7, "dummymsg", 9, MQTTQoS1 ), OtaMqttPublishFailed );
}

/* prvMQTTUnsubscribe */

TEST_F( TestOtaAgentTask, unsubscribe_calls_mqtt_unsubscribe )
{
    EXPECT_EQ( MQTTAgent_Unsubscribe_fake.call_count, 0 );
    prvMQTTUnsubscribe( "dummy", 6, 0 );
    EXPECT_NE( MQTTAgent_Unsubscribe_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, unsubscribe_gets_current_task_handle )
{
    /* presumably to notify the current task once a block arrives that matches a subscription. */
    EXPECT_EQ( xTaskGetCurrentTaskHandle_fake.call_count, 0 );
    prvMQTTUnsubscribe( "dummy", 6, 0 );
    EXPECT_NE( xTaskGetCurrentTaskHandle_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, unsubcribe_tries_to_wait_on_something )
{
    /* This test is a pre-requisite for verifying that unsubscirbe waits on the MQTTAgent_Unsubscribe command */
    EXPECT_EQ( xTaskNotifyWait_fake.call_count, 0 );
    prvMQTTUnsubscribe( "dummy", 6, 0 );
    EXPECT_NE( xTaskNotifyWait_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, unsubcribe_waits_while_unsubscription_command_completes )
{
    /* This behaviour is needed because otherwise we cannot return failure if the command fails.
     * Nor can we cleanup resources allocated for the unsubscribe call.
     * I.e. must call unsubscribe first, and then call wait */
    xTaskNotifyWait_fake.custom_fake = stop_the_rest_of_the_program_running;
    EXPECT_EQ( xTaskNotifyWait_fake.call_count, 0 );
    try {
        prvMQTTUnsubscribe( "dummy", 6, 0 );
    }
    catch( int num ) {
        if( num != CONTEXT_SWITCH_FAKE )
        {
            throw ( num );
        }
    }
    /* I.e. Unsubscribe must be called before waiting. */
    EXPECT_NE( MQTTAgent_Unsubscribe_fake.call_count, 0 );
    EXPECT_NE( xTaskNotifyWait_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, unsubscribe_returns_failure_if_mqtt_unsubscribe_fails )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    MQTTAgent_Unsubscribe_fake.custom_fake = fake_bad_subscribe_parameter;
    EXPECT_NE( prvMQTTUnsubscribe( "dummy", 6, 0 ), OtaMqttSuccess );
}
TEST_F( TestOtaAgentTask, unsubscribe_returns_failure_if_mqtt_unsubscribe_succeeds_but_command_fails )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    MQTTAgent_Unsubscribe_fake.custom_fake = fake_subscribe_command_failure_only;
    EXPECT_NE( prvMQTTUnsubscribe( "dummy", 6, 0 ), OtaMqttSuccess );
}
TEST_F( TestOtaAgentTask, unsubscribe_returns_success_if_mqtt_unsubscribe_succeeds_and_command_succeeds )
{
    MQTTAgent_Unsubscribe_fake.custom_fake = fake_successful_subscription;
    EXPECT_EQ( prvMQTTUnsubscribe( "dummy", 6, 0 ), OtaMqttSuccess );
}

/* setOtaInterfaces */
TEST_F( TestOtaAgentTask, setting_ota_interfaces_changes_the_passed_interface_values )
{
    /* Just check a few fields */
    OtaInterfaces_t * interface = ( OtaInterfaces_t * ) calloc( 1, sizeof( OtaInterfaces_t ) );

    EXPECT_NE( interface, nullptr );
    OtaFree_t original_free = interface->os.mem.free;
    OtaMalloc_t original_malloc = interface->os.mem.malloc;
    OtaMqttPublish_t original_publish = interface->mqtt.publish;
    OtaPalWriteBlock_t original_writeBlock = interface->pal.writeBlock;
    setOtaInterfaces( interface );
    EXPECT_NE( interface->os.mem.free, original_free );
    EXPECT_NE( interface->os.mem.malloc, original_malloc );
    EXPECT_NE( interface->mqtt.publish, original_publish );
    EXPECT_NE( interface->pal.writeBlock, original_writeBlock );
    free( interface );
}

/* prvOTAAgentTask */
TEST_F( TestOtaAgentTask, ota_agent_task_gets_called )
{
    EXPECT_EQ( OTA_EventProcessingTask_fake.call_count, 0 );
    prvOTAAgentTask( nullptr );
    EXPECT_NE( OTA_EventProcessingTask_fake.call_count, 0 );
}

/* prvSuspendOTA */

TEST_F( TestOtaAgentTask, suspending_ota_calls_ota_suspend )
{
    OTA_Suspend_fake.custom_fake = fake_successful_ota_suspend;
    EXPECT_EQ( OTA_Suspend_fake.call_count, 0 );
    prvSuspendOTA();
    EXPECT_NE( OTA_Suspend_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, suspend_returns_success_if_ota_suspend_call_does_not_error )
{
    OTA_Suspend_fake.custom_fake = fake_successful_ota_suspend;
    EXPECT_EQ( prvSuspendOTA(), pdPASS );
}
TEST_F( TestOtaAgentTask, suspend_ota_returns_failure_if_ota_library_suspend_returns_failure )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    OTA_Suspend_fake.custom_fake = fake_unsuccessful_ota_suspend;
    OTA_Suspend_fake.return_val = OtaErrUninitialized;
    OTA_GetState_fake.return_val = OtaAgentStateShuttingDown;
    EXPECT_EQ( prvSuspendOTA(), pdFAIL );
}
TEST_F( TestOtaAgentTask, suspend_ota_returns_failure_if_ota_suspend_reports_success_but_the_current_ota_state_is_not_suspended )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    OTA_Suspend_fake.custom_fake = fake_successful_ota_suspend_but_state_is_incorrect;
    EXPECT_EQ( prvSuspendOTA(), pdFAIL );
}
/* We do not test what happens if ota suspend returns an unusual error code because */
/* There are many possible correct behaviours. */

/* prvResumeOTA */

TEST_F( TestOtaAgentTask, resuming_ota_calls_ota_resume )
{
    OTA_Resume_fake.return_val = OtaErrNone;
    OTA_GetState_fake.return_val = OtaAgentStateWaitingForFileBlock;
    EXPECT_EQ( OTA_Resume_fake.call_count, 0 );
    prvResumeOTA();
    EXPECT_NE( OTA_Resume_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, resume_returns_success_if_ota_resume_call_does_not_error )
{
    OTA_Resume_fake.return_val = OtaErrNone;
    OTA_GetState_fake.return_val = OtaAgentStateWaitingForFileBlock;
    EXPECT_EQ( prvResumeOTA(), pdPASS );
}
TEST_F( TestOtaAgentTask, resume_ota_returns_failure_if_resuming_fails )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    OTA_Resume_fake.custom_fake = fake_unsuccessful_ota_resume;
    EXPECT_EQ( prvResumeOTA(), pdFAIL );
}
TEST_F( TestOtaAgentTask, resume_ota_returns_failure_if_ota_resume_reports_success_but_the_current_ota_state_is_not_resumed )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    OTA_Resume_fake.custom_fake = fake_successful_ota_resume_but_state_is_still_suspended;
    EXPECT_EQ( prvResumeOTA(), pdFAIL );
}

/* Test prvRunOTADemo */

TEST_F( TestOtaAgentTask, demo_waits_for_mqtt_agent )
{
    OTA_GetState_fake.return_val = OtaAgentStateStopped;
    EXPECT_EQ( vWaitUntilMQTTAgentReady_fake.call_count, 0 );
    EXPECT_EQ( vWaitUntilMQTTAgentConnected_fake.call_count, 0 );
    prvRunOTADemo();
    EXPECT_NE( vWaitUntilMQTTAgentReady_fake.call_count, 0 );
    EXPECT_NE( vWaitUntilMQTTAgentConnected_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, demo_tries_to_initialise_ota_library )
{
    OTA_GetState_fake.return_val = OtaAgentStateStopped;
    EXPECT_EQ( OTA_Init_fake.call_count, 0 );
    prvRunOTADemo();
    EXPECT_NE( OTA_Init_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, demo_returns_failure_if_cannot_initialise_ota_library )
{
    SdkLogError_fake.custom_fake = do_nothing_if_an_error_occurs;
    OTA_GetState_fake.return_val = OtaAgentStateStopped;
    OTA_Init_fake.return_val = OtaErrUninitialized;
    EXPECT_EQ( prvRunOTADemo(), pdFAIL );
}
TEST_F( TestOtaAgentTask, demo_tries_to_create_ota_agent_task )
{
    OTA_GetState_fake.return_val = OtaAgentStateStopped;
    EXPECT_EQ( xTaskCreate_fake.call_count, 0 );
    prvRunOTADemo();
    EXPECT_NE( xTaskCreate_fake.call_count, 0 );
}
TEST_F( TestOtaAgentTask, demo_removes_subscription_to_broker_once_finished )
{
    OTA_GetState_fake.return_val = OtaAgentStateStopped;
    EXPECT_EQ( removeSubscription_fake.call_count, 0 );
    prvRunOTADemo();
    EXPECT_NE( removeSubscription_fake.call_count, 0 );
}

/*
 * We do not test this function further because it may use infinite loops,
 * and testing these loops will cause the tests to be brittle.
 */

/* vOtaDemoTask */

TEST_F( TestOtaAgentTask, demo_task_tries_to_read_image_version )
{
    OTA_GetState_fake.return_val = OtaAgentStateStopped;
    xSemaphoreCreateMutex_fake.return_val = 100;
    GetImageVersionPSA_fake.return_val = 0;
    EXPECT_EQ( GetImageVersionPSA_fake.call_count, 0 );
    vOtaDemoTask( nullptr );
    EXPECT_NE( GetImageVersionPSA_fake.call_count, 0 );
}

/* Test vStartOtaTask */
TEST_F( TestOtaAgentTask, starting_ota_task_creates_task )
{
    EXPECT_EQ( xTaskCreate_fake.call_count, 0 );
    vStartOtaTask();
    EXPECT_NE( xTaskCreate_fake.call_count, 0 );
}
