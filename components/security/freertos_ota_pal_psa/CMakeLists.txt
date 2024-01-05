# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(freertos_ota_pal_psa_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to FreeRTOS OTA PAL based on PSA API source code"
)

add_subdirectory(integration)
