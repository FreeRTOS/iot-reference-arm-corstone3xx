# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_subdirectory(corejson)
add_subdirectory(coremqtt)
add_subdirectory(coremqtt_agent)
add_subdirectory(corepkcs11)
# sntp helper library depends on FreeRTOS-Plus-TCP connectivity stack as it
# includes `FreeRTOS_IP.h` header file in one of its source files (sntp_client_task.c),
# thus the coresntp library is only added in case of using FREERTOS_PLUS_TCP connectivity stack.
if(CONNECTIVITY_STACK STREQUAL "FREERTOS_PLUS_TCP")
    add_subdirectory(coresntp)
endif()
add_subdirectory(ota_for_aws_iot_embedded_sdk)
add_subdirectory(tinycbor)
