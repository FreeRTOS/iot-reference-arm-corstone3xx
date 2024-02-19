# Copyright 2021-2024 Arm Limited and/or its affiliates
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
    "${PATCH_FILES_DIRECTORY}/0001-resources-Replace-model-wav2letter-with-tiny_wav2let.patch"
    "${PATCH_FILES_DIRECTORY}/0002-Use-CMSIS_device_header-instead-of-RTE_Components.h.patch"
    "${PATCH_FILES_DIRECTORY}/0003-Make-ETHOSU_ARCH-configurable-in-TensorFlow-CMake.patch"
    "${PATCH_FILES_DIRECTORY}/0004-Move-activation_buf_dram-to-.bss.NoInit-region.patch"
)
iot_reference_arm_corstone3xx_apply_patches("${ml_embedded_evaluation_kit_SOURCE_DIR}" "${PATCH_FILES}")

set(TF_PATCH_FILES
    "${PATCH_FILES_DIRECTORY}/0001-tensorflow-kiss_fft_wrapper-include-fix.patch"
)

iot_reference_arm_corstone3xx_apply_patches("${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/tensorflow" "${TF_PATCH_FILES}")
