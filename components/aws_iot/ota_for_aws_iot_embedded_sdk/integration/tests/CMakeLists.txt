# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# application-specific mocks
add_subdirectory(config_mocks)

add_executable(ota-agent-task-test
    test_ota_agent_task.cpp
    ../src/ota_agent_task.c
)
target_include_directories(ota-agent-task-test
    PRIVATE
        .

        ../../library_mocks/inc

        # ota_agent_task.c has no .h file, so we include 'src' not 'inc'
        ../src
)
target_link_libraries(ota-agent-task-test
    PRIVATE
        coremqtt-agent-integration-mock
        coremqtt-agent-mock
        coremqtt-mock
        fff
        freertos-kernel-mock
        freertos-ota-pal-psa-mock
        helpers-events-mock
        helpers-logging-mock
        trusted-firmware-m-mock
        ota-for-aws-iot-embedded-sdk-mock
        ota-for-aws-iot-embedded-sdk-test-specific-mock
)
iot_reference_arm_corstone3xx_add_test(ota-agent-task-test)
