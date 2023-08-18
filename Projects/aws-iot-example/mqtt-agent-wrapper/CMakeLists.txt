# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# MQTT agent task library

add_library(mqtt-agent-task STATIC)

target_sources(mqtt-agent-task
    PRIVATE
        ota_agent_task.c
        mqtt_agent_task.c
        subscription_manager.c
        freertos_command_pool.c
        freertos_agent_message.c
)

target_include_directories(mqtt-agent-task
    PUBLIC
        .
)

target_link_libraries(mqtt-agent-task
    PRIVATE
        freertos_kernel
        freertos-ota-pal-psa
        awsIoT
        iot-vsocket
        app-config
        app-logging
        event-helper
)
