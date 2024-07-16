# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-plus-tcp-integration-mock
    src/transport_interface_api.c
)

target_include_directories(freertos-plus-tcp-integration-mock
    PUBLIC
        inc
)

target_link_libraries(freertos-plus-tcp-integration-mock
    PRIVATE
        fff
)
