# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(freertos_ota_pal_psa_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to FreeRTOS OTA PAL based on PSA API source code"
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/integration/patches/0001-ota-abort-Fix-successful-abortion-check.patch
    WORKING_DIRECTORY ${freertos_ota_pal_psa_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

add_subdirectory(integration)
