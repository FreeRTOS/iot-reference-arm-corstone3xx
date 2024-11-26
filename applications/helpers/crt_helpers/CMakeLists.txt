# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # Left empty for future mocks.
else()
    add_library(crt-helpers)

    target_include_directories(crt-helpers
        PUBLIC
            inc/)

    target_sources(crt-helpers
        PUBLIC
            src/app_strnlen.c
    )
endif()
