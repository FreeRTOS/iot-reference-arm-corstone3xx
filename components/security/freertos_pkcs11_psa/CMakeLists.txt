# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # left empty for future mocks.
else ()
    set(freertos_pkcs11_psa_SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/library
        CACHE INTERNAL
        "Path to FreeRTOS PKCS#11 to PSA shim layer source code"
    )

    add_subdirectory(integration)
endif()
