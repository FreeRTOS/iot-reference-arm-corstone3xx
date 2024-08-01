# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/cmake)
include(ConvertElfToBin)
include(ExternalProject)

ExternalProject_Get_Property(trusted_firmware-m-build BINARY_DIR)

# This function is documented under `Image signing` section in `trusted_firmware-m.md` document located at
# `${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/docs/components/security/` directory.
function(iot_reference_arm_corstone3xx_tf_m_sign_image target unsigned_image_bin_name signed_bin_name signed_bin_version signature_layout_file pad)
    if(${pad})
        set(pad_option "--pad")
    else()
        set(pad_option "")
    endif()

    add_custom_command(
        TARGET
            ${target}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${target}>/${target}.bin
        COMMAND
            # Sign the non-secure (application) image for TF-M bootloader (BL2)
            python3 ${BINARY_DIR}/api_ns/image_signing/scripts/wrapper/wrapper.py
                -v ${signed_bin_version}
                --layout ${signature_layout_file}
                -k ${BINARY_DIR}/api_ns/image_signing/keys/image_ns_signing_private_key.pem
                --public-key-format full
                --align 1 --pad-header ${pad_option} -H 0x400 -s auto
                --measured-boot-record
                --confirm
                ${SECTORS_BIN_DIR}/${unsigned_image_bin_name}.bin
                $<TARGET_FILE_DIR:${target}>/${signed_bin_name}.bin
        COMMAND
            ${CMAKE_COMMAND} -E echo "-- signed: $<TARGET_FILE_DIR:${target}>/${signed_bin_name}.bin"
        VERBATIM
    )
endfunction()
