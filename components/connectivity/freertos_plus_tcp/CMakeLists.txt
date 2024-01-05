# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(freertos_plus_tcp_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to FreeRTOS-Plus-TCP source code"
)

if (${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone300" OR ${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone310")
    set(FREERTOS_PLUS_TCP_NETWORK_IF "MPS3_AN552" CACHE STRING "FreeRTOS Plus TCP Network Interface selection")
endif()

set(FREERTOS_PLUS_TCP_BUFFER_ALLOCATION "2" CACHE STRING "FreeRTOS buffer allocation model number. 1 .. 2.")

add_subdirectory(library/source)
add_subdirectory(library/tools)

add_subdirectory(integration)
