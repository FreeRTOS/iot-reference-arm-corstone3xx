# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# mbedtls-config library is used to supply configuration for MbedTLS
# IMPORTANT: user applications must provide a "mbedtls_config.h" file which can be an empty
# file or contain MbedTLS options the user wants to override. This can be done by adding
# an include directory to mbed-config that contains the file.
add_library(mbedtls-config INTERFACE)
target_link_libraries(mbedtls
    PUBLIC
        mbedtls-config
)

if(PSA_CRYPTO_IMPLEMENTATION STREQUAL "TF-M")
    target_link_libraries(mbedtls
        PRIVATE
            tfm_api_ns
    )
    target_compile_definitions(mbedtls-config
        INTERFACE
            -DPSA_CRYPTO_IMPLEMENTATION_TFM
    )
else()
    target_compile_definitions(mbedtls-config
        INTERFACE
            -DPSA_CRYPTO_IMPLEMENTATION_MBEDTLS
    )
endif()

add_library(mbedtls-threading-freertos
    src/mbedtls_freertos_port.c
)
target_link_libraries(mbedtls-threading-freertos
    PRIVATE
        freertos_kernel
        mbedtls
)

target_link_libraries(mbedtls-config
    INTERFACE
        mbedtls-threading-freertos
)

target_link_libraries(mbedx509
    PUBLIC
        mbedtls-config
)

target_link_libraries(mbedcrypto
    PUBLIC
        mbedtls-config
)

add_library(iot-tls
    src/iot_tls.c
)
target_include_directories(iot-tls
    PUBLIC
        inc
)
target_link_libraries(iot-tls
    PUBLIC
        mbedtls
        corepkcs11
    PRIVATE
        helpers-logging
)

# The toolchain enables different warnings (ex. -Wswitch-default)
# and MbedTLS enables -Werror on all its libraries. Some of the files
# under the mbedcrypto, mbedtls, mbedx509, and everest third party library
# violate these rules which result in compilation time errors.
# Hence, disbaling errors on violated warnings for those libraries.
target_compile_options(mbedcrypto
    PRIVATE
        -Wno-error=switch-default
        -Wno-error=null-dereference
)
target_compile_options(mbedtls
    PRIVATE
        -Wno-error=switch-default
        -Wno-error=null-dereference
)
target_compile_options(mbedx509
    PRIVATE
        -Wno-error=null-dereference
)
target_compile_options(everest
    PRIVATE
        -Wno-error=redundant-decls
)
