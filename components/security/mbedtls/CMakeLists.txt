# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(library_mocks)
else ()
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

    include(ApplyPatches)

    set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
    set(PATCH_FILES "${PATCH_FILES_DIRECTORY}/0001-md-Fix-guards-for-functions-and-headers.patch")
    iot_reference_arm_corstone3xx_apply_patches("${mbedtls_SOURCE_DIR}" "${PATCH_FILES}")

    add_subdirectory(library)

    add_subdirectory(integration)
endif()
