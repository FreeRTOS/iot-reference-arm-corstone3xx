# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(trusted_firmware-m_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to Trusted Firmware-M source code"
)

include(ApplyPatches)

set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
set(PATCH_FILES "${PATCH_FILES_DIRECTORY}/0001-corstone300-Modify-ISRAM0-to-be-non-secure-partition.patch")
iot_reference_arm_corstone3xx_apply_patches("${trusted_firmware-m_SOURCE_DIR}" "${PATCH_FILES}")

add_subdirectory(integration)
