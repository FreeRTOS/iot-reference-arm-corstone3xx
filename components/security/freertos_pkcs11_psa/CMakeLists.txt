# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(freertos_pkcs11_psa_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to FreeRTOS PKCS#11 to PSA shim layer source code"
)

include(ApplyPatches)

set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
set(PATCH_FILES
    "${PATCH_FILES_DIRECTORY}/0001-build-Update-mbedtls-version.patch"
)
iot_reference_arm_corstone3xx_apply_patches("${freertos_pkcs11_psa_SOURCE_DIR}" "${PATCH_FILES}")

add_subdirectory(integration)
