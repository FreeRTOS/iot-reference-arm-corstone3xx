# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Add application-specific mocks.
# E.g. app_config.h since it varies by application.
add_subdirectory(config_mocks)

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

# Add helpers for testing mqtt_agent_task.c. Helpers are application-specific.
# E.g. app_config.h is a helper since it varies by application.
add_subdirectory(helpers)

# Test mqq_agent_task.c
add_executable(mqtt-agent-task-test
    test_mqtt_agent_task.cpp
    ../src/mqtt_agent_task.c
)
target_include_directories(mqtt-agent-task-test
    PRIVATE
        .
        ../../library_mocks/inc

        ../inc
)
target_link_libraries(mqtt-agent-task-test
    PRIVATE
        fff
        backoff-algorithm-mock
        coremqtt-agent-integration-mock
        coremqtt-agent-test-config-mocks
        coremqtt-mock
        freertos-kernel-mock
        freertos-plus-tcp-integration-mock
        helpers-events-mock
        helpers-logging-mock
        mbedtls-mock
        trusted-firmware-m-mock
)
iot_reference_arm_corstone3xx_add_test(mqtt-agent-task-test)
