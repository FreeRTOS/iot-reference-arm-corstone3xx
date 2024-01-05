# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(freertos_kernel_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to FreeRTOS-Kernel source code"
)

set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

if(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone300")
    set(FREERTOS_PORT GCC_ARM_CM55_TFM)
elseif(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone310")
    set(FREERTOS_PORT GCC_ARM_CM85_TFM)
endif()

# FreeRTOS requires the freertos_config library to exist first
add_library(freertos_config INTERFACE)
add_subdirectory(library)
