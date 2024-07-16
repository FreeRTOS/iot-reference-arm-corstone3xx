# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(helpers-logging-mock
    INTERFACE
)

target_include_directories(helpers-logging-mock
    INTERFACE
        inc
)

target_link_libraries(helpers-logging-mock
    INTERFACE
        fff
)
