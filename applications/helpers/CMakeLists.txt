# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_subdirectory(device_advisor)
add_subdirectory(events)
add_subdirectory(hdlcd)
add_subdirectory(logging)
add_subdirectory(provisioning)
# sntp helper library depends on FreeRTOS-Plus-TCP connectivity stack as it
# includes `FreeRTOS_IP.h` header file in one of its source files (sntp_client_task.c),
# thus this library is only added in case of using FREERTOS_PLUS_TCP connectivity stack.
if(CONNECTIVITY_STACK STREQUAL "FREERTOS_PLUS_TCP")
    add_subdirectory(sntp)
endif()
