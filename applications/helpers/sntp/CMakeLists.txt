# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(helpers-sntp
    src/sntp_client_task.c
)

target_include_directories(helpers-sntp
    PUBLIC
        inc
)

target_include_directories(coresntp-config
    INTERFACE
        inc
)

target_link_libraries(helpers-sntp
    backoff-algorithm
    connectivity-stack
    corepkcs11
    coresntp
    helpers-logging
    mbedtls
)
