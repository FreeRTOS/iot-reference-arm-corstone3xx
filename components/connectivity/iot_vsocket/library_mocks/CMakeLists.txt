# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(iot-vsocket-mock INTERFACE
)

target_include_directories(iot-vsocket-mock
    INTERFACE
        inc
)

target_link_libraries(iot-vsocket-mock
    INTERFACE
        fff
)
