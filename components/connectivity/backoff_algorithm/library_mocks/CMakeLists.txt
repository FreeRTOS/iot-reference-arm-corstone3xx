# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(backoff-algorithm-mock
    src/backoff_algorithm.c
)

target_include_directories(backoff-algorithm-mock
    PUBLIC
        inc
)

target_link_libraries(backoff-algorithm-mock
    PRIVATE
        fff
)
