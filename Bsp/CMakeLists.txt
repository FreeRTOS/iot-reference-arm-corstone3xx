# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(arm-corstone-platform-bsp)

# BSP serial library

add_library(fri-bsp STATIC)

target_sources(fri-bsp
    PRIVATE
        platform/bsp_serial.c
)

target_include_directories(fri-bsp
    PUBLIC
        platform
)

target_link_libraries(fri-bsp
    PUBLIC
        arm-corstone-platform-bsp
)
