# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(trusted-firmware-m-mock INTERFACE)

target_include_directories(trusted-firmware-m-mock
    INTERFACE
        inc
)

target_link_libraries(trusted-firmware-m-mock
    INTERFACE
        fff
)
