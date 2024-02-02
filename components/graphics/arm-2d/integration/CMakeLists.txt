# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(arm-2d STATIC)

target_include_directories(arm-2d
    PUBLIC
        ${arm-2d_SOURCE_DIR}/Library/Include
        ${arm-2d_SOURCE_DIR}/Helper/Include
)

target_sources(arm-2d
    PUBLIC
        ${arm-2d_SOURCE_DIR}/Library/Source/arm_2d.c
        ${arm-2d_SOURCE_DIR}/Library/Source/__arm_2d_tile.c
        ${arm-2d_SOURCE_DIR}/Library/Source/arm_2d_draw.c
        ${arm-2d_SOURCE_DIR}/Library/Source/arm_2d_alpha_blending.c
        ${arm-2d_SOURCE_DIR}/Library/Source/arm_2d_helium.c
)

target_link_libraries(arm-2d
    PUBLIC
        arm_2d-config
        cmsis-dsp
)
