# Copyright 2021-2024, Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

project(tfm-ns-interface)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(BuildTfm)

# TF-M NS interface for the non-secure side

add_library(tfm-ns-interface ${tfm_ns_interface_generated})
add_dependencies(tfm-ns-interface trusted_firmware-m-build)

target_include_directories(tfm-ns-interface
    PUBLIC
        ${BINARY_DIR}/api_ns/interface/include
)

target_link_libraries(tfm-ns-interface
    PRIVATE
        ${s_veneers_generated}
)

target_compile_definitions(tfm-ns-interface
    PUBLIC
        BL2
        # Corstone-315 is not using the default crypto keys, it is defined in the TF-M platform port
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone300>:PLATFORM_DEFAULT_CRYPTO_KEYS>
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone310>:PLATFORM_DEFAULT_CRYPTO_KEYS>
        MCUBOOT_IMAGE_NUMBER=2
)
