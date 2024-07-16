# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(library_mocks)
else()
    set(freertos_kernel_SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/library
        CACHE INTERNAL
        "Path to FreeRTOS-Kernel source code"
    )

    set(FREERTOS_HEAP "${CMAKE_CURRENT_LIST_DIR}/integration/src/heap_management.c" CACHE STRING "" FORCE)

    if(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone300")
        set(FREERTOS_PORT GCC_ARM_CM55_TFM)
    elseif(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone310" OR ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone315")
        set(FREERTOS_PORT GCC_ARM_CM85_TFM)
    endif()

    # FreeRTOS requires the freertos_config library to exist first
    add_library(freertos_config INTERFACE)
    target_include_directories(freertos_config
        INTERFACE
            $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone300>:${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone300/include>
            $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone310>:${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone310/include>
            $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone315>:${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone315/include>
    )
    add_subdirectory(library)
endif()
