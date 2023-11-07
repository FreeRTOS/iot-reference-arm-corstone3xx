# Copyright 2021-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Files generated by TF-M build must be listed as BUILD_BYPRODUCTS
# to inform CMake that they don't exist before build starts. Include
# paths do not need to be listed.
# <BINARY_DIR> is a placeholder keyword in ExternalProject_Add.

set(tfm_ns_interface_generated
    <BINARY_DIR>/install/interface/src/tfm_psa_ns_api.c
    <BINARY_DIR>/install/interface/src/tfm_ps_api.c
    <BINARY_DIR>/install/interface/src/tfm_its_api.c
    <BINARY_DIR>/install/interface/src/tfm_crypto_api.c
    <BINARY_DIR>/install/interface/src/tfm_attest_api.c
    <BINARY_DIR>/install/interface/src/tfm_platform_api.c
    <BINARY_DIR>/install/interface/src/os_wrapper/tfm_ns_interface_rtos.c
)
if(TFM_CMAKE_ARGS MATCHES "TFM_PARTITION_FIRMWARE_UPDATE=ON")
    list(APPEND tfm_ns_interface_generated <BINARY_DIR>/install/interface/src/tfm_fwu_api.c)
endif()

set(s_veneers_generated
    <BINARY_DIR>/install/interface/lib/s_veneers.o
)

include(ExternalProject)

# TF-M can be built with a different toolchain, but the toolchain that
# builds the IoT SDK is guaranteed to be available in the environment.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(tfm_toolchain_file "toolchain_GNUARM.cmake")
elseif(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    set(tfm_toolchain_file "toolchain_ARMCLANG.cmake")
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_SOURCE_DIR}/0001-cs310-tfm-Fix-Ethos-U-git-link.patch
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/trusted-firmware-m"
    OUTPUT_QUIET
    ERROR_QUIET
)

ExternalProject_Add(
    tf-m-build

    # Use code fetched by FetchContent
    DOWNLOAD_COMMAND    ""
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/trusted-firmware-m

    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD     ON

    BUILD_ALWAYS ON

    CMAKE_ARGS
        -D TFM_TOOLCHAIN_FILE=<SOURCE_DIR>/${tfm_toolchain_file}
        -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        ${TFM_CMAKE_ARGS}

    PATCH_COMMAND
        ${TFM_PATCH_COMMAND}

    BUILD_BYPRODUCTS
        ${tfm_ns_interface_generated}
        ${s_veneers_generated}
)

# The path ${BINARY_DIR} is available after ExternalProject_Add.
# Convert <BINARY_DIR> to allow projects to use those files.
ExternalProject_Get_Property(tf-m-build BINARY_DIR)
list(TRANSFORM tfm_ns_interface_generated REPLACE "<BINARY_DIR>" "${BINARY_DIR}")
list(TRANSFORM s_veneers_generated REPLACE "<BINARY_DIR>" "${BINARY_DIR}")
