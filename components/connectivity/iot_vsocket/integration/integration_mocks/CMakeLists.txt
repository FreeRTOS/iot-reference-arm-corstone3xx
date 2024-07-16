# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(iot-vsocket-integration-mock
    src/transport_interface_api.c
)

target_include_directories(iot-vsocket-integration-mock
    PUBLIC
        inc
)

target_link_libraries(iot-vsocket-integration-mock
    PRIVATE
        fff
)
