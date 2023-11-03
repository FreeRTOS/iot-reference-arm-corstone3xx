# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/cmake)
include(ConvertElfToBin)
include(ExternalProject)

ExternalProject_Get_Property(trusted_firmware-m-build BINARY_DIR)

function(iot_reference_arm_corstone3xx_tf_m_sign_image target signed_target_name version pad)
    if(${pad})
        set(pad_option "--pad")
    else()
        set(pad_option "")
    endif()
    target_elf_to_bin(${target} ${target}_unsigned)
    add_custom_command(
        TARGET
            ${target}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${target}>/${target}.bin
        COMMAND
            # Sign the non-secure (application) image for TF-M bootloader (BL2)
            python3 ${BINARY_DIR}/api_ns/image_signing/scripts/wrapper/wrapper.py
                -v ${version}
                --layout ${BINARY_DIR}/api_ns/image_signing/layout_files/signing_layout_ns.o
                -k ${BINARY_DIR}/api_ns/image_signing/keys/image_ns_signing_private_key.pem
                --public-key-format full
                --align 1 --pad-header ${pad_option} -H 0x400 -s auto
                --measured-boot-record
                --confirm
                $<TARGET_FILE_DIR:${target}>/${target}_unsigned.bin
                $<TARGET_FILE_DIR:${target}>/${signed_target_name}.bin
        COMMAND
            ${CMAKE_COMMAND} -E echo "-- signed: $<TARGET_FILE_DIR:${target}>/${signed_target_name}.bin"
        VERBATIM
    )
endfunction()
