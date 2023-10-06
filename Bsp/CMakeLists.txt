# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0

set(ARM_CORSTONE_BSP_TARGET_PLATFORM "corstone310" CACHE STRING "Featured Reference Integration target")

set(arm_corstone_platform_bsp_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to Arm Corstone-3xx Platform CMSIS-Driver Based Board Support Package source code"
)

add_subdirectory(library)

target_compile_definitions(arm-corstone-platform-bsp
    INTERFACE
        __DOMAIN_NS=1
)

if(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone300")
    set(BL2_IMAGE_LOAD_ADDRESS 0x00000000 CACHE STRING "Bootload image loading address")
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 CACHE STRING "Secure TF-M firmware loading address")
    set(NS_IMAGE_LOAD_ADDRESS 0x28080000 CACHE STRING "Non-secure user application loading address")
    set(S_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x10022000 CACHE STRING "Secure provisioning bundle loading address")
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x211FF000 CACHE STRING "Non-Secure provisioning bundle loading address")
elseif(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone310")
    set(BL2_IMAGE_LOAD_ADDRESS 0x11000000 CACHE STRING "Bootload image loading address")
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 CACHE STRING "Secure TF-M firmware loading address")
    set(NS_IMAGE_LOAD_ADDRESS 0x28080000 CACHE STRING "Non-secure user application loading address")
    set(S_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x11022000 CACHE STRING "Secure provisioning bundle loading address")
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x213FF000 CACHE STRING "Non-Secure provisioning bundle loading address")
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
