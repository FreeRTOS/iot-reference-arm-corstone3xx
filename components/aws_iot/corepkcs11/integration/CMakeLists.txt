# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${corepkcs11_SOURCE_DIR}/pkcsFilePaths.cmake)

# Do not use ${PKCS_SOURCES} because we are using the
# PKCS11 to PSA Crypto wrappers from freertos-pkcs11-psa
add_library(corepkcs11
    ${corepkcs11_SOURCE_DIR}/source/core_pkcs11.c
    ${corepkcs11_SOURCE_DIR}/source/core_pki_utils.c
)

target_include_directories(corepkcs11
    PUBLIC
        ${PKCS_INCLUDE_PUBLIC_DIRS}
)

add_library(corepkcs11-config INTERFACE)

target_link_libraries(corepkcs11
    PUBLIC
        corepkcs11-config
    PRIVATE
        freertos_kernel
        freertos-pkcs11-psa
        mbedtls
)
