# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(CMAKE_CROSSCOMPILING)
    set(freertos_ota_pal_psa_SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/library
        CACHE INTERNAL
        "Path to FreeRTOS OTA PAL based on PSA API source code"
    )

    include(ApplyPatches)

    set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
    set(PATCH_FILES
        "${PATCH_FILES_DIRECTORY}/0001-ota-abort-Fix-successful-abortion-check.patch"
        "${PATCH_FILES_DIRECTORY}/0002-Update-FreeRTOS-OTA-PAL-PSA-to-work-with-new-modular.patch"
        "${PATCH_FILES_DIRECTORY}/0003-component-ID-Add-ML-model-file-path.patch"
        "${PATCH_FILES_DIRECTORY}/0004-app-version-Do-not-store-version-in-a-global-var.patch"
        "${PATCH_FILES_DIRECTORY}/0005-PAL-Implement-component-version-getter.patch"
    )
    iot_reference_arm_corstone3xx_apply_patches("${freertos_ota_pal_psa_SOURCE_DIR}" "${PATCH_FILES}")

    add_subdirectory(integration)
else()
    add_subdirectory(library_mocks)
endif()
