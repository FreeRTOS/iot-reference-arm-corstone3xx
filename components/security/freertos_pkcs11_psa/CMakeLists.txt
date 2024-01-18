# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(freertos_pkcs11_psa_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to FreeRTOS PKCS#11 to PSA shim layer source code"
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/integration/patches/0001-build-Update-mbedtls-version.patch
    WORKING_DIRECTORY ${freertos_pkcs11_psa_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

add_subdirectory(integration)
