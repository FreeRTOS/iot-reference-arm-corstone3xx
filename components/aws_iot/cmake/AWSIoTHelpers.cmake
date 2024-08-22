# Copyright 2024, Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

#include(ExternalProject)
find_package(Python3)

string(REGEX MATCH "^[A-Z]*" AWS_OTA_SIG_TYPE ${AWS_OTA_SIGNATURE_TYPE})
string(REGEX MATCH "[0-9]*$" AWS_OTA_SIG_LEN ${AWS_OTA_SIGNATURE_TYPE})
set(AWS_OTA_SIG_TYPE    ${AWS_OTA_SIG_TYPE} CACHE INTERNAL "Ota signature algorythm")
set(AWS_OTA_SIG_LEN     ${AWS_OTA_SIG_LEN}  CACHE INTERNAL "Ota signature length")

set(AWS_OTA_SIGNATURE_PRIVATE_KEY_PATH  ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/certs/private_key.pem   CACHE STRING "Ota signature private key path")
set(AWS_OTA_SIGNATURE_PUBLIC_KEY_PATH   ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/certs/public_key.pem    CACHE STRING "Ota signature public key path")
set(AWS_OTA_SIGNATURE_CERTIFICATE_PATH  ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/certs/certificate.pem)  #Only used for key generation script

# NOTE: As of now generate_credentials.py is only capable of generating RSA keys
add_custom_command(
    OUTPUT
        ${AWS_OTA_SIGNATURE_PRIVATE_KEY_PATH}
        ${AWS_OTA_SIGNATURE_PUBLIC_KEY_PATH}
    COMMAND
        ${Python3_EXECUTABLE} ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/scripts/generate_credentials.py
            --certificate_country_name          UK
            --certificate_state_province_name   Dummy
            --certificate_locality_name         Dummy
            --certificate_org_name              Arm
            --certificate_out_path              ${AWS_OTA_SIGNATURE_CERTIFICATE_PATH}
            --private_key_out_path              ${AWS_OTA_SIGNATURE_PRIVATE_KEY_PATH}
            --public_key_out_path               ${AWS_OTA_SIGNATURE_PUBLIC_KEY_PATH}
            --key_bit_length                    ${AWS_OTA_SIG_LEN}
)
add_custom_target(aws_ota_signing_keys
    SOURCES
        ${AWS_OTA_SIGNATURE_PRIVATE_KEY_PATH}
        ${AWS_OTA_SIGNATURE_PUBLIC_KEY_PATH}
)
