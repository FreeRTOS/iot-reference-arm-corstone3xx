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

    # The patch 0001-crypto-Define-device-private-key-as-volatile.patch is only
    # needed when Mbed TLS on non-secure side is used as PSA crypto implementation.
    if(PSA_CRYPTO_IMPLEMENTATION STREQUAL "MBEDTLS")
        include(ApplyPatches)

        set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
        set(PATCH_FILES
            "${PATCH_FILES_DIRECTORY}/0001-crypto-Define-device-private-key-as-volatile.patch"
        )
        iot_reference_arm_corstone3xx_apply_patches("${freertos_pkcs11_psa_SOURCE_DIR}" "${PATCH_FILES}")
    endif()

    add_subdirectory(integration)
endif()
