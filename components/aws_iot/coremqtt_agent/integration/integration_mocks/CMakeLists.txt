# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(coremqtt-agent-integration-mock
    src/freertos_agent_message.c
    src/freertos_command_pool.c
    src/mqtt_agent_task.c
    src/subscription_manager.c
)

target_include_directories(coremqtt-agent-integration-mock
    PUBLIC
        inc
)

target_link_libraries(coremqtt-agent-integration-mock
    PRIVATE
        fff
        coremqtt-mock
        coremqtt-agent-mock
        freertos-kernel-mock
)
