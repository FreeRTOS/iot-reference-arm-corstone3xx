# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(ota_for_aws_iot_embedded_sdk_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to AWS IoT Over-the-air Update source code"
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_SOURCE_DIR}/integration/patches/0001-Replace-strnlen-with-strlen.patch
    WORKING_DIRECTORY ${ota_for_aws_iot_embedded_sdk_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

add_subdirectory(integration)
