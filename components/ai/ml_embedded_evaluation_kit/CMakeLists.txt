# Copyright 2021-2025 Arm Limited and/or its affiliates
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
    "${PATCH_FILES_DIRECTORY}/0001-Override-EthosU55-NPU-default-IRQ-handler.patch"
)
iot_reference_arm_corstone3xx_apply_patches("${ml_embedded_evaluation_kit_SOURCE_DIR}" "${PATCH_FILES}")
