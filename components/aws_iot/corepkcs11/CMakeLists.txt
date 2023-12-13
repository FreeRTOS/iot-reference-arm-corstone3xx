# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(CMAKE_CROSSCOMPILING)
    set(corepkcs11_SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/library
        CACHE INTERNAL
        "Path to corePKCS11 source code"
    )

    include(ApplyPatches)

    set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
    set(PATCH_FILES "${PATCH_FILES_DIRECTORY}/0001-Do-not-define-__PASTE-macro-for-Arm-compiler.patch")
    iot_reference_arm_corstone3xx_apply_patches("${corepkcs11_SOURCE_DIR}" "${PATCH_FILES}")

    add_subdirectory(integration)
endif()
