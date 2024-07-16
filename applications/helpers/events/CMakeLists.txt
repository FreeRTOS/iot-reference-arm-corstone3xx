# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(mocks)
else()
    add_library(helpers-events
        src/events.c
    )

    target_include_directories(helpers-events
        PUBLIC
            inc
    )

    target_link_libraries(helpers-events
        freertos_kernel
    )
endif()
