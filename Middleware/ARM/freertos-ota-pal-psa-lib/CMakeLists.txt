# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-ota-pal-psa STATIC
    freertos-ota-pal-psa/version/application_version.c
    freertos-ota-pal-psa/ota_pal.c
    src/ota_provision.c
)

target_compile_definitions(freertos-ota-pal-psa
    PRIVATE
        # FreeRTOS OTA PSA PAL configuration
        OTA_PAL_CODE_SIGNING_ALGO=OTA_PAL_CODE_SIGNING_RSA
        OTA_PAL_SIGNATURE_FORMAT=OTA_PAL_SIGNATURE_RAW
)

target_include_directories(freertos-ota-pal-psa
    PUBLIC
        freertos-ota-pal-psa
        src
)

target_link_libraries(freertos-ota-pal-psa
    PUBLIC
        aws-configs
        tfm-ns-interface
        awsIoT
)
