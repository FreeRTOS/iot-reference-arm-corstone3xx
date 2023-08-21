# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_subdirectory(kernel)

if (DEFINED CONNECTIVITY_STACK)
    if (${CONNECTIVITY_STACK} STREQUAL "FREERTOS_TCP_IP")
        add_subdirectory(FreeRTOS-Plus-TCP-lib)
    endif()
endif()
