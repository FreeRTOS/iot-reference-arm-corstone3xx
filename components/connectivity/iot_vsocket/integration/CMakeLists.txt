# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(integration_mocks)
    add_library(connectivity-stack-mock ALIAS iot-vsocket-integration-mock)
else()
    add_library(iot-vsocket
        ${iot_vsocket_SOURCE_DIR}/interface/vsocket/iot_socket.c
        src/network_startup.c
        src/transport_tls_iot_socket.c
    )

    target_include_directories(iot-vsocket
        PUBLIC
            inc
            ${iot_vsocket_SOURCE_DIR}/interface/include
    )

    target_link_libraries(iot-vsocket
        PRIVATE
            coremqtt
            helpers-events
            fri-bsp
            iot-socket-api
            iot-tls
    )
    add_library(connectivity-stack ALIAS iot-vsocket)
endif()
