# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(coremqtt-agent-mock INTERFACE)

target_include_directories(coremqtt-agent-mock
    INTERFACE
        inc
)

target_link_libraries(coremqtt-agent-mock
    INTERFACE
        fff
        coremqtt-mock
)
