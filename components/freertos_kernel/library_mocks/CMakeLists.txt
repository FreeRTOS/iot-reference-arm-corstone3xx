# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-kernel-mock
    src/event_groups.c
    src/semphr.c
    src/tasks.c
    src/queue.c
)

target_include_directories(freertos-kernel-mock
    PUBLIC
        inc
)

target_link_libraries(freertos-kernel-mock
    PRIVATE
        fff
)
