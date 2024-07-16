# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(helpers-events-mock
    src/events.c
)

target_include_directories(helpers-events-mock
    PUBLIC
        inc
)

target_link_libraries(helpers-events-mock
    PRIVATE
        fff
)
