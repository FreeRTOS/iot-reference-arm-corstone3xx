# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(mbedtls-mock
    src/psa/crypto.c
)

target_include_directories(mbedtls-mock
    PUBLIC
        inc
)

target_link_libraries(mbedtls-mock
    PRIVATE
        fff
)
