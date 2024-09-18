/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "fff.h"

#include "gtest/gtest.h"

extern "C" {
#include "freertos_agent_message.h"
#include "core_mqtt_agent_message_interface.h"
}

DEFINE_FFF_GLOBALS

class TestFreertosAgentMessage : public ::testing::Test {
public:
    TestFreertosAgentMessage()
    {
        RESET_FAKE( xQueueSendToBack );
        RESET_FAKE( xQueueReceive );
    }
};

TEST_F( TestFreertosAgentMessage, sending_nullptr_message_returns_false )
{
    MQTTAgentCommand_t * command;

    EXPECT_FALSE( Agent_MessageSend( nullptr, &command, 1 ) );
}

TEST_F( TestFreertosAgentMessage, sending_nullptr_command_returns_false )
{
    MQTTAgentMessageContext_t message;

    EXPECT_FALSE( Agent_MessageSend( &message, nullptr, 1 ) );
}

TEST_F( TestFreertosAgentMessage, failing_to_send_a_message_returns_false )
{
    xQueueSendToBack_fake.return_val = pdFAIL;

    MQTTAgentMessageContext_t message;
    MQTTAgentCommand_t * command;
    EXPECT_FALSE( Agent_MessageSend( &message, &command, 1 ) );
}

TEST_F( TestFreertosAgentMessage, successfully_sending_a_message_returns_true )
{
    xQueueSendToBack_fake.return_val = pdPASS;

    MQTTAgentMessageContext_t message;
    MQTTAgentCommand_t * command;
    EXPECT_TRUE( Agent_MessageSend( &message, &command, 1 ) );
}

TEST_F( TestFreertosAgentMessage, request_to_receive_with_nullptr_message_returns_false )
{
    MQTTAgentCommand_t * command;

    EXPECT_FALSE( Agent_MessageReceive( nullptr, &command, 1 ) );
}

TEST_F( TestFreertosAgentMessage, request_to_receive_with_nullptr_command_returns_false )
{
    MQTTAgentMessageContext_t message;

    EXPECT_FALSE( Agent_MessageReceive( &message, nullptr, 1 ) );
}

TEST_F( TestFreertosAgentMessage, failing_to_receive_a_message_returns_false )
{
    xQueueReceive_fake.return_val = pdFAIL;

    MQTTAgentMessageContext_t message;
    MQTTAgentCommand_t * command;
    EXPECT_FALSE( Agent_MessageReceive( &message, &command, 1 ) );
}

TEST_F( TestFreertosAgentMessage, successfully_receiving_a_message_returns_true )
{
    xQueueReceive_fake.return_val = pdPASS;

    MQTTAgentMessageContext_t message;
    MQTTAgentCommand_t * command;
    EXPECT_TRUE( Agent_MessageReceive( &message, &command, 1 ) );
}
