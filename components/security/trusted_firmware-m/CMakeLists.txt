# Copyright 2023-2025, Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT


set(trusted_firmware-m_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to Trusted Firmware-M source code"
)

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(library_mocks)
else()
    # This patch is only applied in case of building keyword_detection application with GNU toolchain
    # as it is currently the only application that utilises the ML Model component OTA update feature
    # where this patch is needed.
    if((${APPLICATION_PATH} MATCHES ".*keyword_detection") AND (${CMAKE_C_COMPILER_ID} STREQUAL "GNU"))
        include(ApplyPatches)

        set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
        set(PATCH_FILES
            "${PATCH_FILES_DIRECTORY}/0001-corstone300-Add-ML-model-component.patch"
            "${PATCH_FILES_DIRECTORY}/0002-corstone310-Add-ML-model-component.patch"
            "${PATCH_FILES_DIRECTORY}/0003-mps4-Add-ML-model-component.patch"
        )
        iot_reference_arm_corstone3xx_apply_patches("${trusted_firmware-m_SOURCE_DIR}" "${PATCH_FILES}")
    endif()
    add_subdirectory(integration)
endif()
