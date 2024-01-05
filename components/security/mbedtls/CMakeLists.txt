# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(mbedtls_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to Mbed TLS source code"
)

# MbedTLS library configuration
# Programs and testing builds are not required
# Note: Mbed TLS specifies an old version of CMake which does not fully support
# overriding a CACHE variable with a regular one, so we need to use the CACHE
# keyword here.
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory(library)

add_subdirectory(integration)
