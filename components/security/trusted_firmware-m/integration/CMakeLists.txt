# Copyright 2021-2025, Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Use different startup and driver files than TF-M defaults
set(PLATFORM_CUSTOM_NS_FILES TRUE)
# The exported TF-M interfaces
add_subdirectory(${CONFIG_SPE_PATH} ${CMAKE_BINARY_DIR}/spe)

target_link_libraries(tfm_api_ns
    PRIVATE
        tfm-ns-interface-mbedtls-config
)

add_library(tfm-ns-interface-mbedtls-config INTERFACE)

if(APPLICATION_PATH MATCHES ".*blinky")
    target_link_libraries(tfm-ns-interface-mbedtls-config
            INTERFACE
                psa_crypto_config
    )
else()
    target_link_libraries(tfm-ns-interface-mbedtls-config
        INTERFACE
            app-config
    )
    target_compile_definitions(tfm-ns-interface-mbedtls-config
        INTERFACE
            -DPSA_CRYPTO_IMPLEMENTATION_TFM
    )
    # Change PUBLIC sources to PRIVATE:
    # PUBLIC sources are added to both INTERFACE_SOURCES and SOURCES property,
    # so removing from interface makes them PRIVATE (only use them when
    # building tfm_api_ns, not consuming targets)
    set_target_properties(tfm_api_ns PROPERTIES INTERFACE_SOURCES "")
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
