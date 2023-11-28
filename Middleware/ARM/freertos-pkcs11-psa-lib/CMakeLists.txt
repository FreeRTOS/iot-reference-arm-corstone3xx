# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-pkcs11-psa STATIC
    freertos-pkcs11-psa/iot_pkcs11_psa_input_format.c
    freertos-pkcs11-psa/iot_pkcs11_psa_object_management.c
    freertos-pkcs11-psa/iot_pkcs11_psa.c
)

ExternalProject_Get_Property(tf-m-build BINARY_DIR)

target_include_directories(freertos-pkcs11-psa
    PUBLIC
        freertos-pkcs11-psa
        ${BINARY_DIR}/api_ns/interface/include
)

target_link_libraries(freertos-pkcs11-psa
    PUBLIC
        mbedtls
        aws-configs
        awsIoT
)
