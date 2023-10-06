# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${backoff_algorithm_SOURCE_DIR}/backoffAlgorithmFilePaths.cmake)

add_library(backoff-algorithm
    ${BACKOFF_ALGORITHM_SOURCES}
)

target_include_directories(backoff-algorithm
    PUBLIC
        ${BACKOFF_ALGORITHM_INCLUDE_PUBLIC_DIRS}
)

add_library(backoff-algorithm-config INTERFACE)

target_link_libraries(backoff-algorithm
    PUBLIC
        backoff-algorithm-config
)
