# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(unity_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to Unity Test source code"
)

set(UNITY_EXTENSION_FIXTURE ON CACHE BOOL "")

add_subdirectory(library)
