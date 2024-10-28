# Copyright 2023-2024, Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-ota-pal-psa
    ${freertos_ota_pal_psa_SOURCE_DIR}/version/application_version.c
    ${freertos_ota_pal_psa_SOURCE_DIR}/ota_pal.c
)

if(NOT ${AWS_OTA_SIGNATURE_TYPE} STREQUAL "")
    string(REGEX MATCH "^[A-Z]*" AWS_OTA_SIG_TYPE ${AWS_OTA_SIGNATURE_TYPE})
endif()

target_compile_definitions(freertos-ota-pal-psa
    PRIVATE
        $<$<STREQUAL:${AWS_OTA_SIG_TYPE},RSA>:OTA_PAL_CODE_SIGNING_ALGO=OTA_PAL_CODE_SIGNING_RSA>
        $<$<STREQUAL:${AWS_OTA_SIG_TYPE},RSA>:OTA_PAL_SIGNATURE_FORMAT=OTA_PAL_SIGNATURE_RAW>
        $<$<STREQUAL:${AWS_OTA_SIG_TYPE},EC>:OTA_PAL_CODE_SIGNING_ALGO=OTA_PAL_CODE_SIGNING_ECDSA>
        $<$<STREQUAL:${AWS_OTA_SIG_TYPE},EC>:OTA_PAL_SIGNATURE_FORMAT=OTA_PAL_SIGNATURE_ASN1_DER>
)

target_include_directories(freertos-ota-pal-psa
    PUBLIC
        ${freertos_ota_pal_psa_SOURCE_DIR}
)

add_library(freertos-ota-pal-psa-config INTERFACE)

target_link_libraries(freertos-ota-pal-psa
    PUBLIC
        freertos-ota-pal-psa-config
    PRIVATE
        corepkcs11
        freertos_kernel
        helpers-logging
        mbedtls
        jobs-for-aws-iot-embedded-sdk
        ota-update
)
