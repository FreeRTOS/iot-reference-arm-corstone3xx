# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(corepkcs11_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to corePKCS11 source code"
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_SOURCE_DIR}/integration/patches/0001-Do-not-define-__PASTE-macro-for-Arm-compiler.patch
    WORKING_DIRECTORY ${corepkcs11_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

add_subdirectory(integration)
