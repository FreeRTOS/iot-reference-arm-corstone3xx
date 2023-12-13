# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(CMAKE_CROSSCOMPILING)
    set(tinycbor_SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/library
        CACHE INTERNAL
        "Path to TinyCBOR source code"
    )

    add_subdirectory(integration)
endif()
