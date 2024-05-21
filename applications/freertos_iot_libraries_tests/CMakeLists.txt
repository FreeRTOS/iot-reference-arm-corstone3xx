# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

# From: ota-for-aws-iot-embedded-sdk/source/include/ota_appversion32.h
# struct version
# {
#     uint8_t major;  /*!< @brief Major version number of the firmware (X in firmware version X.Y.Z). */
#     uint8_t minor;  /*!< @brief Minor version number of the firmware (Y in firmware version X.Y.Z). */
#
#     uint16_t build; /*!< @brief Build of the firmware (Z in firmware version X.Y.Z). */
# } x;                /*!< @brief Version number of the firmware. */

# AWS OTA client does not use the SemVer PATCH version.
# Because of this, if only PATCH version is changed then the OTA will be rejected
# due to same firmware version.
# We will therefore change the build version from TF-M.
set(MCUBOOT_IMAGE_VERSION_NS "0.0.1+10")
set(MCUBOOT_IMAGE_VERSION_NS_UPDATE "0.0.1+20")

set(APPLICATION_PATH "${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/applications/freertos_iot_libraries_tests" CACHE STRING "Path to the application folder")

# Trusted Firmware-M setup
set(TFM_CMAKE_APP_ARGS
    -DPROJECT_CONFIG_HEADER_FILE=${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/applications/freertos_iot_libraries_tests/configs/tfm_config/project_config.h
    -DMCUBOOT_DATA_SHARING=ON
    -DMCUBOOT_CONFIRM_IMAGE=ON
    -DMCUBOOT_UPGRADE_STRATEGY=SWAP_USING_SCRATCH
    -DMCUBOOT_IMAGE_VERSION_NS=${MCUBOOT_IMAGE_VERSION_NS}
    -DPLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT=ON
    -DTFM_PARTITION_FIRMWARE_UPDATE=ON
)

project(freertos-iot-libraries-tests LANGUAGES C)

# Set global optimization level to reduce code size while keeping the debug experience.
if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    add_compile_options(-Og)
elseif(${CMAKE_C_COMPILER_ID} STREQUAL "ARMClang")
    add_compile_options(-O1)
endif()

# This variable is checked to apply configurations specific to FreeRTOS Libraries Integrations Tests
set(FREERTOS_LIBRARIES_INTEGRATION_TESTS 1)

add_subdirectory(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR} ${CMAKE_BINARY_DIR}/iot_reference_arm_corstone3xx)

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/aws_iot/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake)
include(SetLinkerOptions)
include(GenerateAWSUpdateDigestAndSignature)
include(MergeTfmImages)
include(SignTfmImage)

add_subdirectory(configs)
add_subdirectory(../helpers ${CMAKE_BINARY_DIR}/helpers)


# Configure FreeRTOS Libraries Integration Tests component
add_dependencies(freertos-libraries-integration-tests-config provisioning_data_bin)
target_include_directories(freertos-libraries-integration-tests-config
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        provisioning
        ${CMAKE_BINARY_DIR}/provisioning
        ${CMAKE_BINARY_DIR}/provisioning_data
        ${CMAKE_BINARY_DIR}/helpers/provisioning
        ../helpers/provisioning
)
target_link_libraries(freertos-libraries-integration-tests-config
    INTERFACE
        connectivity-stack
        coremqtt
        helpers-logging
        freertos-libraries-integration-tests-mqtt
        freertos-libraries-integration-tests-ota-pal
        freertos-libraries-integration-tests-qualification
        freertos-libraries-integration-tests-transport
        unity
)

add_executable(${CMAKE_PROJECT_NAME}
    main.c
    integration_tests_platform_function.c
)

# Trusted Firmware-M must be built before the application, because
# the application depends on the NS interface and the BL2 signing scripts,
# both of which are generated as parts of the Trusted Firmware-M build process.
add_dependencies(${CMAKE_PROJECT_NAME} trusted_firmware-m-build)
# The provision data must be built before the application because
# it provides credentials to connect to AWS
add_dependencies(${CMAKE_PROJECT_NAME} provisioning_data_bin)
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        backoff-algorithm
        connectivity-stack
        coremqtt
        coremqtt-agent
        corepkcs11
        freertos_kernel
        freertos-libraries-integration-tests-config
        freertos-ota-pal-psa
        fri-bsp
        helpers-events
        ota-for-aws-iot-embedded-sdk
        provisioning-lib
        mbedtls
        tfm-ns-interface
)

set_linker_script(${CMAKE_PROJECT_NAME})

# The non-secure application image should be padded while being signed
iot_reference_arm_corstone3xx_tf_m_sign_image(
    ${CMAKE_PROJECT_NAME}
    ${CMAKE_PROJECT_NAME}_signed
    ${MCUBOOT_IMAGE_VERSION_NS}
    TRUE
)

# The update image is not padded to fill the whole slot (no --pad), because
# 1) the image to download is smaller without padding
# 2) the trailer that keeps track of boot and update statuses should not be overwritten
iot_reference_arm_corstone3xx_tf_m_sign_image(
    ${CMAKE_PROJECT_NAME}
    ${CMAKE_PROJECT_NAME}-update_signed
    ${MCUBOOT_IMAGE_VERSION_NS_UPDATE}
    FALSE
)

# A user project that consumes the ARM FRI needs to explicitly provide
# addresses in order to merge images for TF-M. The addresses cannot
# be easily programmatically extracted as they are defined in the linker
# scripts.
iot_reference_arm_corstone3xx_tf_m_merge_images(
    ${CMAKE_PROJECT_NAME}
    ${NS_PROVISIONING_BUNDLE_LOAD_ADDRESS}
    ${CMAKE_BINARY_DIR}/helpers/provisioning/provisioning_data.bin
)

# Generate the AWS OTA update digest and signature
iot_reference_arm_corstone3xx_generate_aws_update_digest_and_signature(
    ${CMAKE_PROJECT_NAME}
    ${CMAKE_PROJECT_NAME}-update_signed
    update-digest
    update-signature
)
