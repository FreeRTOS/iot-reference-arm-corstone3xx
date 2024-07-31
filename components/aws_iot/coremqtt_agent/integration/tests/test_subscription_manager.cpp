/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "fff.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace std;

extern "C" {
#include "logging_stack.h"
#include "subscription_manager.h"
/* Functions usually defined by main.c */
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

void debugErr( const char * msg,
               va_list args )
{
    char buf[ 1000 ];

    vsnprintf( buf, 1000, msg, args );
    cout << "Program errored with:" << buf << "\n";
}
void debugWarn( const char * msg,
                va_list args )
{
    char buf[ 1000 ];

    vsnprintf( buf, 1000, msg, args );
    cout << "Program warned with: " << buf << "\n";
}


DECLARE_FAKE_VOID_FUNC( dummyCallback, void *, MQTTPublishInfo_t * );
DEFINE_FAKE_VOID_FUNC( dummyCallback, void *, MQTTPublishInfo_t * );
DECLARE_FAKE_VOID_FUNC( dummyCallback2, void *, MQTTPublishInfo_t * );
DEFINE_FAKE_VOID_FUNC( dummyCallback2, void *, MQTTPublishInfo_t * );

class TestSubscriptionManager : public ::testing::Test {
public:
    TestSubscriptionManager()
    {
        RESET_FAKE( dummyCallback );
        RESET_FAKE( dummyCallback2 );
        RESET_FAKE( MQTT_MatchTopic );
        RESET_FAKE( SdkLogError );
        RESET_FAKE( SdkLogWarn );
        RESET_FAKE( SdkLogInfo );
        MQTT_MatchTopic_fake.return_val = MQTTSuccess;
        SdkLogError_fake.custom_fake = debugErr;
        SdkLogWarn_fake.custom_fake = debugWarn;
    }
};

int context[ 10 ]; /* should not write to this if using it during a test. */

void expect_no_errors( void )
{
    ASSERT_EQ( SdkLogError_fake.call_count, 0 );
}

void expect_warnings( void )
{
    ASSERT_NE( SdkLogWarn_fake.call_count, 0 );
}


void expect_no_warnings_or_errors( void )
{
    ASSERT_EQ( SdkLogWarn_fake.call_count, 0 );
    ASSERT_EQ( SdkLogError_fake.call_count, 0 ) << "Expected no warnings or errors.";
}

/*
 * bool addSubscription( const char * pcTopicFilterString,
 *                    uint16_t usTopicFilterLength,
 *                    IncomingPubCallback_t pxIncomingPublishCallback,
 *                    void * pvIncomingPublishCallbackContext );
 *
 * This function has no function dependencies except the C standard library.
 */

TEST_F( TestSubscriptionManager, adding_a_nullptr_callback_function_does_not_segfault )
{
    addSubscription( "dummy", 5, nullptr, context );
}

TEST_F( TestSubscriptionManager, adding_a_nullptr_topic_name_does_not_segfault )
{
    addSubscription( nullptr, 5, dummyCallback, context );
}

TEST_F( TestSubscriptionManager, adding_a_nullptr_callback_context_does_not_segfault )
{
    addSubscription( "dummy", 5, dummyCallback, nullptr );
}

TEST_F( TestSubscriptionManager, adding_a_valid_subscription_does_not_error )
{
    addSubscription( "dummy", 5, dummyCallback, context );
    expect_no_errors();
}

TEST_F( TestSubscriptionManager, adding_a_valid_subscription_returns_true )
{
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback, context ) );
    expect_no_warnings_or_errors();
}

TEST_F( TestSubscriptionManager, adding_two_subscriptions_with_different_callbacks_returns_true )
{
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback, context ) );
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback2, context ) );
    expect_no_warnings_or_errors();
}

TEST_F( TestSubscriptionManager, adding_two_subscriptions_which_differ_only_by_topic_should_succeed )
{
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback, context ) );
    EXPECT_TRUE( addSubscription( "dummy2", 6, dummyCallback, context ) );
    expect_no_warnings_or_errors();
}

TEST_F( TestSubscriptionManager, can_handle_empty_topic_name )
{
    std::string str = "";
    const char * veryShortTopicName = str.c_str();

    EXPECT_FALSE( addSubscription( veryShortTopicName, 0, dummyCallback, context ) );
}

TEST_F( TestSubscriptionManager, can_handle_very_long_topic_names )
{
    std::string str = "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string"
                      "this is a very large string this is a very large string";
    const char * veryLongTopicName = str.c_str();

    EXPECT_TRUE( addSubscription( veryLongTopicName, 551, dummyCallback, context ) );
    expect_no_warnings_or_errors();
}

/* This test also verifies it has been added correctly. */
TEST_F( TestSubscriptionManager, adding_the_same_subscription_twice_causes_warnings )
{
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback, context ) );
    expect_no_warnings_or_errors();
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback, context ) );
    expect_warnings();
}

TEST_F( TestSubscriptionManager, adding_duplicate_subscriptions_returns_true )
{
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback, context ) );
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback, context ) );
    EXPECT_TRUE( addSubscription( "dummy", 5, dummyCallback, context ) );
}

TEST_F( TestSubscriptionManager, subscription_manager_eventually_cannot_add_subscriptions )
{
    /* This test assumes you cannot add more than the maximum number of subscriptions. */
    bool outOfMemory = false;
    uint32_t excessSubscriptions = 10;
    vector<string> topics( SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS + excessSubscriptions + 1 );

    for( uint32_t i = 0; i < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS + excessSubscriptions; i++ )
    {
        topics[ i ] = std::to_string( i );
        outOfMemory = !addSubscription( topics[ i ].c_str(), topics[ i ].length(), dummyCallback, context );
    }

    EXPECT_TRUE( outOfMemory ) << "Appeared to add more subscriptions than the maximum limit.";
    EXPECT_FALSE( addSubscription( "uniqueName", 5, dummyCallback, context ) ) << "Can still add subscriptions. Should be full.";
}

TEST_F( TestSubscriptionManager, subscription_manager_has_the_correct_maximum_subscription_amount )
{
    int ctxt[ 10 ];
    bool outOfMemory = false;
    int outOfMemoryCount = 0;
    uint32_t excessSubscriptions = 10;
    vector<string> topics( SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS + excessSubscriptions + 1 );

    for( uint32_t i = 0; i < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS + excessSubscriptions; i++ )
    {
        topics[ i ] = std::to_string( i );
        outOfMemory = !addSubscription( topics[ i ].c_str(), topics[ i ].length(), dummyCallback, ctxt );

        if( outOfMemory )
        {
            outOfMemoryCount++;
        }
    }

    EXPECT_EQ( outOfMemoryCount, excessSubscriptions ) << "Should have detected " << excessSubscriptions << " subscriptions over the limit.";
}

TEST_F( TestSubscriptionManager, subscription_manager_does_not_overwrite_undeleted_values_if_full )
{
    bool outOfMemory = false;
    uint32_t excessSubscriptions = 10;
    vector<string> topics( SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS + excessSubscriptions + 1 );

    for( uint32_t i = 0; i < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS + excessSubscriptions; i++ )
    {
        topics[ i ] = std::to_string( i );
        outOfMemory = !addSubscription( topics[ i ].c_str(), topics[ i ].length(), dummyCallback, context );
    }

    /* Should be out of memory. */
    EXPECT_TRUE( addSubscription( "0", 1, dummyCallback, context ) );
}

/*
 * bool removeSubscription( const char * pcTopicFilterString,
 *                       uint16_t usTopicFilterLength );
 */


TEST_F( TestSubscriptionManager, trying_to_remove_a_subscription_that_does_not_exist_does_not_error )
{
    removeSubscription( "doesNotExist", 13 );
    expect_no_errors();
}

TEST_F( TestSubscriptionManager, adding_then_removing_a_subscription_does_not_error )
{
    EXPECT_TRUE( addSubscription( "0", 1, dummyCallback, context ) );
    removeSubscription( "0", 1 );
    expect_no_errors();
}

TEST_F( TestSubscriptionManager, removing_a_subscription_successfully_returns_true )
{
    EXPECT_TRUE( addSubscription( "0", 1, dummyCallback, context ) );
    EXPECT_TRUE( removeSubscription( "0", 1 ) );
    expect_no_warnings_or_errors();
}

TEST_F( TestSubscriptionManager, adding_then_removing_a_subscription_actually_removes_it )
{
    EXPECT_TRUE( addSubscription( "0", 1, dummyCallback, context ) );
    removeSubscription( "0", 1 );
    expect_no_warnings_or_errors();
    /* try to add the same subscription back. Should not get a warning that it already exists. */
    EXPECT_TRUE( addSubscription( "0", 1, dummyCallback, context ) );
    expect_no_warnings_or_errors();
}

TEST_F( TestSubscriptionManager, failure_to_remove_a_subscription_returns_false )
{
    EXPECT_FALSE( removeSubscription( "0", 1 ) );
}


TEST_F( TestSubscriptionManager, get_no_failures_when_adding_then_removing_a_lot_of_subscriptions )
{
    bool outOfMemory = false;
    uint32_t excessSubscriptions = 10;
    vector<string> topics( SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS );

    for( uint32_t i = 0; i < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS; i++ )
    {
        topics[ i ] = std::to_string( i );
        outOfMemory = !addSubscription( topics[ i ].c_str(), topics[ i ].length(), dummyCallback, context );
    }

    /* Should be out of memory. */
    expect_no_errors();

    for( uint32_t i = 0; i < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS; i++ )
    {
        removeSubscription( topics[ i ].c_str(), topics[ i ].length() );
    }

    expect_no_warnings_or_errors();
}

TEST_F( TestSubscriptionManager, removing_from_a_full_subscription_list_creates_space )
{
    bool outOfMemory = false;
    uint32_t excessSubscriptions = 10;
    vector<string> topics( SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS + excessSubscriptions + 1 );

    for( uint32_t i = 0; i < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS + excessSubscriptions; i++ )
    {
        topics[ i ] = std::to_string( i );
        outOfMemory = !addSubscription( topics[ i ].c_str(), topics[ i ].length(), dummyCallback, context );
    }

    expect_no_warnings_or_errors();
    EXPECT_TRUE( outOfMemory );
    removeSubscription( "0", 1 );
    EXPECT_TRUE( addSubscription( "50", 2, dummyCallback, context ) );
}

/*
 * bool handleIncomingPublishes( MQTTPublishInfo_t * pxPublishInfo );
 */

TEST_F( TestSubscriptionManager, callback_handler_does_not_error_if_passed_read_only_topic_name )
{
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "readonly",
        9,
        nullptr,
        0
    };

    handleIncomingPublishes( &info );
    expect_no_errors();
}

MQTTStatus_t set_match_false( const char * pTopicName,
                              const uint16_t topicNameLength,
                              const char * pTopicFilter,
                              const uint16_t topicFilterLength,
                              bool * pIsMatch )
{
    *pIsMatch = false;
    return MQTTSuccess;
}

MQTTStatus_t set_match_true_if_valid( const char * pTopicName,
                                      const uint16_t topicNameLength,
                                      const char * pTopicFilter,
                                      const uint16_t topicFilterLength,
                                      bool * pIsMatch )
{
    if( ( topicNameLength > 0 ) && ( topicFilterLength > 0 ) )
    {
        *pIsMatch = true;
    }

    return MQTTSuccess;
}

TEST_F( TestSubscriptionManager, callback_handler_validates_handler_for_subscription )
{
    addSubscription( "dummy/#", 8, dummyCallback, context );
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "dummy",
        9,
        nullptr,
        0
    };
    handleIncomingPublishes( &info );
    EXPECT_NE( MQTT_MatchTopic_fake.call_count, 0 );
}

TEST_F( TestSubscriptionManager, callback_handler_does_not_error_if_no_subscription_exists_for_a_topic )
{
    MQTT_MatchTopic_fake.custom_fake = set_match_false;
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "dummy",
        9,
        nullptr,
        0
    };
    handleIncomingPublishes( &info );
    expect_no_errors();
}

TEST_F( TestSubscriptionManager, callback_handler_does_not_call_handler_if_topic_does_not_match_any_subscriptions )
{
    addSubscription( "dummy/#", 8, dummyCallback, context );
    MQTT_MatchTopic_fake.custom_fake = set_match_false; /* topic does not match. */
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "dummy/test",
        9,
        nullptr,
        0
    };
    handleIncomingPublishes( &info );
    expect_no_errors();
    EXPECT_EQ( dummyCallback2_fake.call_count, 0 );
}

TEST_F( TestSubscriptionManager, callback_handler_calls_handler_if_topic_matches_subscription )
{
    addSubscription( "dummy/#", 8, dummyCallback, context );
    MQTT_MatchTopic_fake.custom_fake = set_match_true_if_valid; /* topic always matches (if not empty subscriptionc) */
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "dummy/test",
        9,
        nullptr,
        0
    };
    handleIncomingPublishes( &info );
    expect_no_errors();
    EXPECT_NE( dummyCallback_fake.call_count, 0 );
}

TEST_F( TestSubscriptionManager, callback_handler_returns_false_if_no_subscription_found_for_topic_given )
{
    MQTT_MatchTopic_fake.custom_fake = set_match_false; /* topic does not match. */
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "dummy/test",
        9,
        nullptr,
        0
    };
    EXPECT_FALSE( handleIncomingPublishes( &info ) );
    expect_no_errors();
}

TEST_F( TestSubscriptionManager, callback_handler_returns_true_if_matching_subscription_exists_for_topic )
{
    addSubscription( "dummy/#", 8, dummyCallback, context );
    MQTT_MatchTopic_fake.custom_fake = set_match_true_if_valid; /* topic always matches (if not empty subscriptionc) */
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "dummy/test",
        9,
        nullptr,
        0
    };
    EXPECT_TRUE( handleIncomingPublishes( &info ) );
    expect_no_errors();
}

TEST_F( TestSubscriptionManager, callback_handler_does_not_segfault_if_null_subscription_info_passed )
{
    handleIncomingPublishes( nullptr );
}

TEST_F( TestSubscriptionManager, callback_handler_calls_callback_function_a_single_time )
{
    addSubscription( "dummy/#", 8, dummyCallback, context );
    MQTT_MatchTopic_fake.custom_fake = set_match_true_if_valid; /* topic always matches (if not empty subscriptionc) */
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "dummy/test",
        9,
        nullptr,
        0
    };
    handleIncomingPublishes( &info );
    expect_no_errors();
    EXPECT_EQ( dummyCallback_fake.call_count, 1 );
}

TEST_F( TestSubscriptionManager, callback_handler_calls_callback_function_for_every_matching_subscription )
{
    addSubscription( "dummy/#", 8, dummyCallback, context );
    addSubscription( "dummy/", 8, dummyCallback2, context );
    addSubscription( "dummy/##", 8, dummyCallback, context );
    MQTT_MatchTopic_fake.custom_fake = set_match_true_if_valid; /* topic always matches (if not empty subscriptionc) */
    MQTTPublishInfo_t info = {
        MQTTQoS0,
        true,
        true,
        "dummy/test",
        9,
        nullptr,
        0
    };
    handleIncomingPublishes( &info );
    expect_no_errors();
    EXPECT_EQ( dummyCallback_fake.call_count, 2 );
    EXPECT_EQ( dummyCallback2_fake.call_count, 1 );
}
