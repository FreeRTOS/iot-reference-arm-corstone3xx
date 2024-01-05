# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_subdirectory(backoff_algorithm)

set(CONNECTIVITY_STACK "FREERTOS_PLUS_TCP" CACHE STRING "Choose the connectivity stack (FREERTOS_PLUS_TCP|IOT_VSOCKET) to use ().")

if(CONNECTIVITY_STACK STREQUAL "FREERTOS_PLUS_TCP")
    add_subdirectory(freertos_plus_tcp)
elseif(CONNECTIVITY_STACK STREQUAL "IOT_VSOCKET")
    add_subdirectory(iot_socket)
    add_subdirectory(iot_vsocket)
else()
    message(FATAL_ERROR "Invalid connectivity stack choice: ${CONNECTIVITY_STACK}")
endif()
