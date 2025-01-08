# Copyright 2023-2025, Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

target_include_directories(mbedtls-config
    INTERFACE
        .
)

target_link_libraries(mbedtls-config
    INTERFACE
        freertos_kernel
)
