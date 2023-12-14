# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(tinycbor
    ${tinycbor_SOURCE_DIR}/src/cborpretty.c
    ${tinycbor_SOURCE_DIR}/src/cborpretty_stdio.c
    ${tinycbor_SOURCE_DIR}/src/cborencoder.c
    ${tinycbor_SOURCE_DIR}/src/cborencoder_close_container_checked.c
    ${tinycbor_SOURCE_DIR}/src/cborerrorstrings.c
    ${tinycbor_SOURCE_DIR}/src/cborparser.c
    ${tinycbor_SOURCE_DIR}/src/cborparser_dup_string.c
)

target_include_directories(tinycbor
    PUBLIC
        ${tinycbor_SOURCE_DIR}/src
)

add_library(tinycbor-config INTERFACE)

target_link_libraries(tinycbor
    PUBLIC
        tinycbor-config
)
