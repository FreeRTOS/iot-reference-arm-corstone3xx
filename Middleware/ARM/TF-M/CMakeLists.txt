# Copyright 2021-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

project(tfm-ns-interface)

include(cmake/TF-M-build.cmake)

# TF-M NS interface for the non-secure side

add_library(tfm-ns-interface ${tfm_ns_interface_generated})
add_dependencies(tfm-ns-interface tf-m-build)

target_include_directories(tfm-ns-interface
    PUBLIC
        ${BINARY_DIR}/generated/interface/include
        ${BINARY_DIR}/install/interface/include
)

target_link_libraries(tfm-ns-interface
    PRIVATE
        ${s_veneers_generated}
)

target_compile_definitions(tfm-ns-interface
    PUBLIC
        BL2
        PLATFORM_DEFAULT_CRYPTO_KEYS
        MCUBOOT_IMAGE_NUMBER=2
)

# Reference NS startup files from TF-M
# They are optional - an application can provide its own.

ExternalProject_Get_Property(tf-m-build SOURCE_DIR)

add_library(tfm-ns-startup-an552 OBJECT
${SOURCE_DIR}/platform/ext/target/arm/mps3/an552/device/source/startup_an552.c
)
target_include_directories(tfm-ns-startup-an552
PRIVATE
    ${SOURCE_DIR}/platform/include
    ${SOURCE_DIR}/platform/ext/cmsis
    ${SOURCE_DIR}/platform/ext/target/arm/mps3/an552/device/include
    ${SOURCE_DIR}/platform/ext/target/arm/mps3/an552/partition
)

add_library(tfm-ns-startup-an555 OBJECT
${SOURCE_DIR}/platform/ext/target/arm/mps3/corstone310/common/device/source/startup_corstone310.c
)
target_include_directories(tfm-ns-startup-an555
PRIVATE
    ${SOURCE_DIR}/platform/include
    ${SOURCE_DIR}/platform/ext/cmsis
    ${SOURCE_DIR}/platform/ext/target/arm/mps3/corstone310/fvp/device/config/
    ${SOURCE_DIR}/platform/ext/target/arm/mps3/corstone310/common/device/include/
    ${SOURCE_DIR}/platform/ext/target/arm/mps3/corstone310/common/partition/
)
