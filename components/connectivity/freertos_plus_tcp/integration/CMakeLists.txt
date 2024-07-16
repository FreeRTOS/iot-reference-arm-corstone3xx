# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(integration_mocks)
    add_library(connectivity-stack-mock ALIAS freertos-plus-tcp-integration-mock)
else ()
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
        PUBLIC
            # TODO: The CS315 network interface layer inside the FreeRTOS TCP/IP stack
            # https://github.com/FreeRTOS/FreeRTOS-Plus-TCP/blob/main/source/portable/NetworkInterface/MPS4_CS315/Device/Include/SSE315.h#L62
            # is done in such a way that it depends on fri-bsp. This dependency
            # should be removed.
            fri-bsp
        PRIVATE
            coremqtt
            helpers-events
            iot-tls
    )

    add_library(connectivity-stack ALIAS freertos_plus_tcp)
endif()
