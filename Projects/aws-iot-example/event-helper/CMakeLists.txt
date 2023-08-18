# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# System event helper library

add_library(event-helper STATIC)

target_sources(event-helper
    PRIVATE
        event_helper.c
)

target_include_directories(event-helper
    PUBLIC
        .
)

target_link_libraries(event-helper
    PRIVATE
        freertos_kernel
)
