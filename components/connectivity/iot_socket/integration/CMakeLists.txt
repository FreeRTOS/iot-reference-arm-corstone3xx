# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(iot-socket-api INTERFACE)

target_include_directories(iot-socket-api
    INTERFACE
        ${iot_socket_SOURCE_DIR}/include
)
