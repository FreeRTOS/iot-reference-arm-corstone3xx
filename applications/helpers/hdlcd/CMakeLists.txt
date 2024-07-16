# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # left empty for future mocks.
else()
    add_library(helpers-hdlcd STATIC)

    target_include_directories(helpers-hdlcd
        PUBLIC
            .
    )

    target_sources(helpers-hdlcd
        PUBLIC
            hdlcd_helper.c
    )

    target_link_libraries(helpers-hdlcd
        PUBLIC
            fri-bsp
    )
endif()
