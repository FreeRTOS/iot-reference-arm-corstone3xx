# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_SOURCE_DIR}/0001-Replace-CMSIS-RTX-APIs-with-FreeRTOS-APIs.patch
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/AVH"
    OUTPUT_QUIET
    ERROR_QUIET
)

add_library(iot-vsocket STATIC
    AVH/interface/vsocket/iot_socket.c
    network_startup.c
    transport_tls_iot_socket.c
)

add_library(connectivity-stack ALIAS iot-vsocket)

target_include_directories(iot-vsocket
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        AVH/interface/include
)

target_link_libraries(iot-vsocket
    PUBLIC
        iot-socket-api
        event-helper
        freertos_kernel
        fri-bsp
        awsIoT
        mbedtls-helpers
)
