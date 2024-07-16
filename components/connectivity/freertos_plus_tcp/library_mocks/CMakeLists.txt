# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-plus-tcp-mock INTERFACE)

target_include_directories(freertos-plus-tcp-mock
    INTERFACE
        inc
)

target_link_libraries(freertos-plus-tcp-mock
    INTERFACE
        fff
)
