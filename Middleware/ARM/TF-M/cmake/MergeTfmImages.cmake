# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(ConvertElfToBin)
include(ExternalProject)

ExternalProject_Get_Property(tf-m-build BINARY_DIR)

# To merge the bootloader image, TF-M secure image, non-secure user application image,
# secure and non-secure provsioning bundle images into one image, their addresses are
# needed. As the addresses are defined in their respective linker scripts, there is no
# simple way to programmatically get them, so they need to be specified by the user project.
# Order: <bootloader>, <signed secure TF-M firmware>, <signed non-secure user app>, <secure provisioning bundle address>, <non-secure provisioning data load address> (optional), <non-secure provisioning data path> (optional).

# This function is making use of CMake optional arguments feature, the reason why this feature
# is used is that not every application will need to pass the non-secure provisioning data load address
# and the non-secure provisioning data path to this function.
# ARGV5 is mapped to non-secure provisioning data load address.
# ARGV6 is mapped to non-secure provisioning data path.
function(iot_reference_arm_corstone3xx_tf_m_merge_images target bl2_address tfm_s_address ns_address s_prov_bundle_address)
    if(DEFINED ARGV5)
        set(ns_provisioning_data_param ${ARGV6} -Binary -offset ${ARGV5})
    else()
        set(ns_provisioning_data_param "")
    endif()
    find_program(srec_cat NAMES srec_cat REQUIRED)
    find_program(objcopy NAMES arm-none-eabi-objcopy objcopy REQUIRED)
    add_custom_command(
        TARGET
            ${target}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${target}>/${target}_signed.bin
        COMMAND
            ${srec_cat} ${BINARY_DIR}/api_ns/bin/bl2.bin -Binary -offset ${bl2_address}
                ${BINARY_DIR}/api_ns/bin/tfm_s_signed.bin -Binary -offset ${tfm_s_address}
                $<TARGET_FILE_DIR:${target}>/${target}_signed.bin -Binary -offset ${ns_address}
                ${ns_provisioning_data_param}
                ${CMAKE_BINARY_DIR}/Middleware/ARM/TF-M/tf-m-build-prefix/src/tf-m-build-build/api_ns/bin/encrypted_provisioning_bundle.bin -Binary -offset ${s_prov_bundle_address}
                -o $<TARGET_FILE_DIR:${target}>/${target}_merged.hex
        COMMAND
            ${objcopy} -I ihex -O elf32-little
                $<TARGET_FILE_DIR:${target}>/${target}_merged.hex
                $<TARGET_FILE_DIR:${target}>/${target}_merged.elf
        COMMAND
            ${CMAKE_COMMAND} -E echo "-- merged: $<TARGET_FILE_DIR:${target}>/${target}_merged.elf"
        VERBATIM
    )
endfunction()
