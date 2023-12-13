# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_executable(freertos-agent-message-test
    test_freertos_agent_message.cpp
    ../src/freertos_agent_message.c
)

target_include_directories(freertos-agent-message-test
    PRIVATE
        ../inc
)

target_link_libraries(freertos-agent-message-test
    PRIVATE
        fff
        coremqtt-agent-mock
        freertos-kernel-mock
)

iot_reference_arm_corstone3xx_add_test(freertos-agent-message-test)
