# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(coremqtt-mock INTERFACE)

target_include_directories(coremqtt-mock
    INTERFACE
        inc
        ../integration/inc
)

target_link_libraries(coremqtt-mock
    INTERFACE
        fff
)
