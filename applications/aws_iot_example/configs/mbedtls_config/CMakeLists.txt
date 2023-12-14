# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

target_include_directories(mbedtls-config
    INTERFACE
        .
)

target_compile_definitions(mbedtls-config
    INTERFACE
        MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h"
)

target_link_libraries(mbedtls-config
    INTERFACE
        freertos_kernel
)
