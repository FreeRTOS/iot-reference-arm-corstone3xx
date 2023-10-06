# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(iot_vsocket_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to Arm Virtual Hardware - IoT Virtual Socket source code"
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/integration/patches/0001-Replace-CMSIS-RTX-APIs-with-FreeRTOS-APIs.patch
    WORKING_DIRECTORY ${iot_vsocket_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

add_subdirectory(integration)
