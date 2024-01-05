# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(ExternalProject)

add_library(freertos-pkcs11-psa
    ${freertos_pkcs11_psa_SOURCE_DIR}/iot_pkcs11_psa_input_format.c
    ${freertos_pkcs11_psa_SOURCE_DIR}/iot_pkcs11_psa_object_management.c
    ${freertos_pkcs11_psa_SOURCE_DIR}/iot_pkcs11_psa.c
)

ExternalProject_Get_Property(trusted_firmware-m-build BINARY_DIR)

target_include_directories(freertos-pkcs11-psa
    PUBLIC
        ${freertos_pkcs11_psa_SOURCE_DIR}
        ${BINARY_DIR}/api_ns/interface/include
)

add_library(freertos-pkcs11-psa-config INTERFACE)

target_link_libraries(freertos-pkcs11-psa
    PUBLIC
        freertos-pkcs11-psa-config
    PRIVATE
        corepkcs11
        freertos_kernel
        mbedtls
)
