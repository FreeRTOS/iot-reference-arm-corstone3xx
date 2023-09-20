# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(arm-corstone-platform-bsp)

# Target specific image loading addresses
if(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone300")
    set(BL2_IMAGE_LOAD_ADDRESS 0x00000000 PARENT_SCOPE)
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 PARENT_SCOPE)
    set(NS_IMAGE_LOAD_ADDRESS 0x28080000 PARENT_SCOPE)
    set(S_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x10022000 PARENT_SCOPE)
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x211FF000 PARENT_SCOPE)
elseif(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone310")
    set(BL2_IMAGE_LOAD_ADDRESS 0x11000000 PARENT_SCOPE)
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 PARENT_SCOPE)
    set(NS_IMAGE_LOAD_ADDRESS 0x28080000 PARENT_SCOPE)
    set(S_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x11022000 PARENT_SCOPE)
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x213FF000 PARENT_SCOPE)
endif()

# BSP serial library

add_library(fri-bsp STATIC)

target_sources(fri-bsp
    PRIVATE
        common/bsp_serial.c
)

target_include_directories(fri-bsp
    PUBLIC
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone300>:${CMAKE_CURRENT_LIST_DIR}/corstone300/include>
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone310>:${CMAKE_CURRENT_LIST_DIR}/corstone310/include>
        common
)

target_link_libraries(fri-bsp
    PUBLIC
        arm-corstone-platform-bsp
)
