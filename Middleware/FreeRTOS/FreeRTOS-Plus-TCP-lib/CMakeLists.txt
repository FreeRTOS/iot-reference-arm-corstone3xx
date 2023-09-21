# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Currently, the network interface for LAN91C111 ethernet controller in
# FreeRTOS TCP/IP is named `MPS3_AN552` and is based on CMSIS ethernet driver.
# All the MPS3 FVPs emulate LAN91C111 for network connectivity. Therefore,
# `MPS3_AN552` network interface is used by both Corstone-300 and Corstone-310
# FVP targets.
set(FREERTOS_PLUS_TCP_NETWORK_IF "MPS3_AN552" CACHE STRING "FreeRTOS Plus TCP Network Interface selection")

set(FREERTOS_PLUS_TCP_BUFFER_ALLOCATION "2" CACHE STRING "FreeRTOS buffer allocation model number. 1 .. 2.")

add_subdirectory(FreeRTOS-Plus-TCP/source)
add_subdirectory(FreeRTOS-Plus-TCP/tools)

add_library(connectivity-stack ALIAS freertos_plus_tcp)

target_sources( freertos_plus_tcp
  PRIVATE
    src/network_startup.c
    src/transport_mbedtls.c
)

target_include_directories( freertos_plus_tcp SYSTEM
  PUBLIC
    src
)

target_link_libraries(freertos_plus_tcp
    PRIVATE
        app-logging
        freertos_kernel
        fri-bsp
        awsIoT
        mbedtls-helpers
        event-helper
)
