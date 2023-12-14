# Copyright 2023 Arm Limited and/or its affiliates
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

add_library(mbedtls-threading-freertos
    src/mbedtls_freertos_port.c
)
target_link_libraries(mbedtls-threading-freertos
    PRIVATE
        freertos_kernel
        mbedtls
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
