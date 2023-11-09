# Copyright 2021-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(ml_embedded_evaluation_kit_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to ML Embedded Evaluation Kit source code"
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/integration/patches/0001-resources-Replace-model-wav2letter-with-tiny_wav2let.patch
    WORKING_DIRECTORY ${ml_embedded_evaluation_kit_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/integration/patches/0001-toolchain-Fix-Arm-GNU-Toolchain-13.2-Rel1-compatibil.patch
    WORKING_DIRECTORY ${ml_embedded_evaluation_kit_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/integration/patches/0001-platform-drivers-Remove-RTE_components-header-file.patch
    WORKING_DIRECTORY ${ml_embedded_evaluation_kit_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/integration/patches/0001-asm-syntax-Replace-incorrect-type-specifier.patch
    WORKING_DIRECTORY ${ml_embedded_evaluation_kit_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

add_subdirectory(integration)
