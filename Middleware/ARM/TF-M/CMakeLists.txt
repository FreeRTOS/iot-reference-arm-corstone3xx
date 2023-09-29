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
