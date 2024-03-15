# Copyright 2023-2024 Arm Limited and/or its affiliates
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

    set(LINKER_SECTION_NAMES  "ddr.bin")
    set(OUTPUT_BINARY_NAME    "flash")

    extract_sections_from_axf(
        ${target}
        SECTIONS_NAMES   "${LINKER_SECTION_NAMES}"
        OUTPUT_BIN_NAME  "${OUTPUT_BINARY_NAME}"
    )

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
                ${SECTORS_BIN_DIR}/${OUTPUT_BINARY_NAME}.bin
                $<TARGET_FILE_DIR:${target}>/${signed_target_name}.bin
        COMMAND
            ${CMAKE_COMMAND} -E echo "-- signed: $<TARGET_FILE_DIR:${target}>/${signed_target_name}.bin"
        VERBATIM
    )
endfunction()
