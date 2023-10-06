# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(ExternalProject)

ExternalProject_Get_Property(trusted_firmware-m-build BINARY_DIR)

# This function is meant to generate the AWS update signature and digest
# for the <update_target_name> input parameter, the name of the signature
# and digest to be generated are passed to the function as <digest_name>
# and <signature_name>.
function(iot_reference_arm_corstone3xx_generate_aws_update_digest_and_signature target update_target_name digest_name signature_name)
    add_custom_command(
        TARGET
            ${target}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${target}>/${update_target_name}.bin
        COMMAND
            openssl dgst -sha256 -binary
                -out  $<TARGET_FILE_DIR:${target}>/${digest_name}.bin
                $<TARGET_FILE_DIR:${target}>/${update_target_name}.bin
        COMMAND
            openssl pkeyutl -sign
                -pkeyopt digest:sha256
                -pkeyopt rsa_padding_mode:pss
                -pkeyopt rsa_mgf1_md:sha256
                -inkey ${BINARY_DIR}/api_ns/image_signing/keys/image_ns_signing_private_key.pem
                -in  $<TARGET_FILE_DIR:${target}>/${digest_name}.bin
                -out  $<TARGET_FILE_DIR:${target}>/${signature_name}.bin
        COMMAND
            openssl base64 -A
                -in  $<TARGET_FILE_DIR:${target}>/${signature_name}.bin
                -out  $<TARGET_FILE_DIR:${target}>/${signature_name}.txt
        COMMAND
            ${CMAKE_COMMAND} -E echo "Use this base 64 encoded signature in OTA job:"
        COMMAND
            ${CMAKE_COMMAND} -E cat  $<TARGET_FILE_DIR:${target}>/${signature_name}.txt
    )
endfunction()
