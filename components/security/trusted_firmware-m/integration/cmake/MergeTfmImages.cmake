# Copyright 2023-2024, Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(ExternalProject)

ExternalProject_Get_Property(trusted_firmware-m-build BINARY_DIR)

# To merge the bootloader image, TF-M secure image, non-secure user application image,
# secure and non-secure provsioning bundle images into one image, their addresses are
# needed. As the addresses are defined in their respective linker scripts, there is no
# simple way to programmatically get them, so they need to be specified by the user project.
# Order: <bootloader>, <signed secure TF-M firmware>, <signed non-secure user app>, <secure provisioning bundle address>, <non-secure provisioning data load address> (optional), <non-secure provisioning data path> (optional).

# This function is making use of CMake optional arguments feature, the reason why this feature
# is used is that not every application will need to pass the non-secure provisioning data load address
# and the non-secure provisioning data path to this function.
# ARGV1 is mapped to non-secure provisioning data load address.
# ARGV2 is mapped to non-secure provisioning data path.
#target bl2_address tfm_s_address ns_address s_prov_bundle_address
function(iot_reference_arm_corstone3xx_tf_m_merge_images target)
    if(DEFINED ARGV1 AND DEFINED ARGV2)
        set(ns_provisioning_data_param ${ARGV2} -Binary -offset ${ARGV1})
    else()
        set(ns_provisioning_data_param "")
    endif()
    if(DEFINED ARGV3 AND DEFINED ARGV4)
        set(ddr_binary_param ${ARGV4} -Binary -offset ${ARGV3})
    else()
        set(ddr_binary_param "")
    endif()
    find_program(srec_cat NAMES srec_cat REQUIRED)
    find_program(objcopy NAMES arm-none-eabi-objcopy objcopy REQUIRED)
    if(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone300" OR ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone310")
        add_custom_command(
            TARGET
                ${target}
            POST_BUILD
            DEPENDS
                $<TARGET_FILE_DIR:${target}>/${target}_signed.bin
            COMMAND
                ${srec_cat} ${BINARY_DIR}/api_ns/bin/bl2.bin -Binary -offset ${BL2_IMAGE_LOAD_ADDRESS}
                    ${BINARY_DIR}/api_ns/bin/tfm_s_signed.bin -Binary -offset ${S_IMAGE_LOAD_ADDRESS}
                    $<TARGET_FILE_DIR:${target}>/${target}_signed.bin -Binary -offset ${NS_IMAGE_LOAD_ADDRESS}
                    ${ddr_binary_param}
                    ${ns_provisioning_data_param}
                    ${BINARY_DIR}/api_ns/bin/encrypted_provisioning_bundle.bin -Binary -offset ${S_PROVISIONING_BUNDLE_LOAD_ADDRESS}
                    -o $<TARGET_FILE_DIR:${target}>/${target}_merged.hex
            COMMAND
                ${objcopy} -I ihex -O elf32-little
                    $<TARGET_FILE_DIR:${target}>/${target}_merged.hex
                    $<TARGET_FILE_DIR:${target}>/${target}_merged.elf
            COMMAND
                ${CMAKE_COMMAND} -E echo "-- merged: $<TARGET_FILE_DIR:${target}>/${target}_merged.elf"
            VERBATIM
        )
    else()
        add_custom_command(
            TARGET
                ${target}
            POST_BUILD
            DEPENDS
                $<TARGET_FILE_DIR:${target}>/${target}_signed.bin
            COMMAND
                ${srec_cat} ${BINARY_DIR}/api_ns/bin/bl1_1.bin -Binary -offset ${BL1_IMAGE_LOAD_ADDRESS}
                    ${BINARY_DIR}/api_ns/bin/cm_provisioning_bundle.bin -Binary -offset ${S_CM_PROVISIONING_BUNDLE_LOAD_ADDRESS}
                    ${BINARY_DIR}/api_ns/bin/dm_provisioning_bundle.bin -Binary -offset ${S_DM_PROVISIONING_BUNDLE_LOAD_ADDRESS}
                    ${BINARY_DIR}/api_ns/bin/bl2_signed.bin -Binary -offset ${BL2_IMAGE_LOAD_ADDRESS}
                    ${BINARY_DIR}/api_ns/bin/tfm_s_signed.bin -Binary -offset ${S_IMAGE_LOAD_ADDRESS}
                    $<TARGET_FILE_DIR:${target}>/${target}_signed.bin -Binary -offset ${NS_IMAGE_LOAD_ADDRESS}
                    ${ddr_binary_param}
                    ${ns_provisioning_data_param}
                    -o $<TARGET_FILE_DIR:${target}>/${target}_merged.hex
            COMMAND
                ${objcopy} -I ihex -O elf32-little
                    $<TARGET_FILE_DIR:${target}>/${target}_merged.hex
                    $<TARGET_FILE_DIR:${target}>/${target}_merged.elf
            COMMAND
                ${CMAKE_COMMAND} -E echo "-- merged: $<TARGET_FILE_DIR:${target}>/${target}_merged.elf"
            VERBATIM
        )
    endif()
endfunction()
