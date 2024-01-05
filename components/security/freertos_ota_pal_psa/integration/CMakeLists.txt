# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-ota-pal-psa
    ${freertos_ota_pal_psa_SOURCE_DIR}/version/application_version.c
    ${freertos_ota_pal_psa_SOURCE_DIR}/ota_pal.c
)

target_compile_definitions(freertos-ota-pal-psa
    PRIVATE
        OTA_PAL_CODE_SIGNING_ALGO=OTA_PAL_CODE_SIGNING_RSA
        OTA_PAL_SIGNATURE_FORMAT=OTA_PAL_SIGNATURE_RAW
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
        ota-for-aws-iot-embedded-sdk
)
