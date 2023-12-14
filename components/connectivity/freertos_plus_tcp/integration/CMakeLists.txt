# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

target_sources(freertos_plus_tcp
    PRIVATE
        src/network_startup.c
        src/transport_mbedtls.c
)

target_include_directories(freertos_plus_tcp SYSTEM
    PUBLIC
        inc
)

target_link_libraries(freertos_plus_tcp
    PRIVATE
        coremqtt
        helpers-events
        iot-tls
)

add_library(connectivity-stack ALIAS freertos_plus_tcp)
