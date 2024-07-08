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
        ${BINARY_DIR}/api_ns/platform/include
)

target_link_libraries(tfm-ns-interface
    PRIVATE
        ${s_veneers_generated}
        tfm-ns-interface-mbedtls-config
)

target_compile_definitions(tfm-ns-interface
    PUBLIC
        BL2
        # Corstone-315 is not using the default crypto keys, it is defined in the TF-M platform port
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone300>:PLATFORM_DEFAULT_CRYPTO_KEYS>
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone310>:PLATFORM_DEFAULT_CRYPTO_KEYS>
        MCUBOOT_IMAGE_NUMBER=2
)

add_library(tfm-ns-interface-mbedtls-config INTERFACE)

if(APPLICATION_PATH MATCHES ".*blinky")
    # TODO: These compile definitions shouldn't be defined explicitly for `blinky` application as
    # they should have been defined by `psa_crypto_config` library which should be linked
    # to `tfm-ns-interface-mbedtls-config` library. However, since we are not using TF-M split-build feature,
    # the exported library `psa_crypto_config` is not used by the non-secure side and these definitions are missing.
    target_compile_definitions(tfm-ns-interface-mbedtls-config
        INTERFACE
            MBEDTLS_CONFIG_FILE="${trusted_firmware-m_SOURCE_DIR}/lib/ext/mbedcrypto/mbedcrypto_config/tfm_mbedcrypto_config_default_client.h"
            MBEDTLS_PSA_CRYPTO_CONFIG_FILE="${trusted_firmware-m_SOURCE_DIR}/lib/ext/mbedcrypto/mbedcrypto_config/crypto_config_default.h"
    )
else()
    target_link_libraries(tfm-ns-interface-mbedtls-config
        INTERFACE
            app-config
    )
    target_compile_definitions(tfm-ns-interface-mbedtls-config
        INTERFACE
            MBEDTLS_CONFIG_FILE="${APPLICATION_PATH}/configs/mbedtls_config/aws_mbedtls_config.h"
            -DPSA_CRYPTO_IMPLEMENTATION_TFM
    )
    # In case of using Mbed TLS library to provide the PSA Crypto APIs
    # implementation, the PSA Crypto APIs implemented and provided by
    # TF-M shall be renamed to start with a prefix of tfm_crypto__
    # to avoid multiple definitions for PSA Crypto APIs.
    if(PSA_CRYPTO_IMPLEMENTATION STREQUAL "MBEDTLS")
        target_compile_definitions(tfm-ns-interface-mbedtls-config
            INTERFACE
                -DCONFIG_TFM_CRYPTO_API_RENAME=1
        )
    endif()
endif()
