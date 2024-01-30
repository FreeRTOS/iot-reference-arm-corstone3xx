# Copyright 2021-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(ml_embedded_evaluation_kit_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to ML Embedded Evaluation Kit source code"
)

include(ApplyPatches)

set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
set(PATCH_FILES
    "${PATCH_FILES_DIRECTORY}/0001-asm-syntax-Replace-incorrect-type-specifier.patch"
    "${PATCH_FILES_DIRECTORY}/0001-platform-drivers-Remove-RTE_components-header-file.patch"
    "${PATCH_FILES_DIRECTORY}/0001-resources-Replace-model-wav2letter-with-tiny_wav2let.patch"
    "${PATCH_FILES_DIRECTORY}/0001-toolchain-Fix-Arm-GNU-Toolchain-13.2-Rel1-compatibil.patch"
)
iot_reference_arm_corstone3xx_apply_patches("${ml_embedded_evaluation_kit_SOURCE_DIR}" "${PATCH_FILES}")

add_subdirectory(integration)
