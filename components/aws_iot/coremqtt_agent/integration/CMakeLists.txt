# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${coremqtt_agent_SOURCE_DIR}/mqttAgentFilePaths.cmake)

add_library(coremqtt-agent
    ${MQTT_AGENT_SOURCES}
    src/mqtt_agent_task.c
    src/subscription_manager.c
    src/freertos_command_pool.c
    src/freertos_agent_message.c
)

target_include_directories(coremqtt-agent
    PUBLIC
        ${MQTT_AGENT_INCLUDE_PUBLIC_DIRS}
        inc
)

add_library(coremqtt-agent-config INTERFACE)

target_link_libraries(coremqtt-agent
    PUBLIC
        coremqtt-agent-config
    PRIVATE
        backoff-algorithm
        connectivity-stack
        coremqtt
        freertos_kernel
        helpers-events
        helpers-logging
)
