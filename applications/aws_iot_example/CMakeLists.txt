# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type" FORCE)
endif()

if (${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone300")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/corstone300/fvp")
elseif(${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone310")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/corstone310/fvp")
else()
    message(FATAL_ERROR "Invalid ARM_CORSTONE_BSP_TARGET_PLATFORM (${ARM_CORSTONE_BSP_TARGET_PLATFORM}) set. Supported are corstone300/corstone310")
endif()

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

set(TFM_CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCONFIG_TFM_ENABLE_CP10CP11=ON
    -DMCUBOOT_CONFIRM_IMAGE=ON
    -DMCUBOOT_DATA_SHARING=ON
    -DMCUBOOT_IMAGE_VERSION_NS=${MCUBOOT_IMAGE_VERSION_NS}
    -DMCUBOOT_GENERATE_SIGNING_KEYPAIR=ON
    -DMCUBOOT_LOG_LEVEL=INFO
    -DMCUBOOT_SIGNATURE_KEY_LEN=3072
    -DMCUBOOT_UPGRADE_STRATEGY=SWAP_USING_SCRATCH
    -DNS=ON
    -DPLATFORM_DEFAULT_PROVISIONING=OFF
    -DPLATFORM_DEFAULT_UART_STDOUT=ON
    -DPLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT=ON
    -DPROJECT_CONFIG_HEADER_FILE=${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/applications/aws_iot_example/configs/tfm_config/project_config.h
    -DTFM_DUMMY_PROVISIONING=OFF
    -DTFM_EXCEPTION_INFO_DUMP=ON
    -DTFM_PARTITION_CRYPTO=ON
    -DTFM_PARTITION_FIRMWARE_UPDATE=ON
    -DTFM_PARTITION_INITIAL_ATTESTATION=ON
    -DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=ON
    -DTFM_PARTITION_PLATFORM=ON
    -DTFM_PARTITION_PROTECTED_STORAGE=ON
    -DTFM_PLATFORM=${TFM_PLATFORM_LOCAL_PATH}
    -DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_INFO
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(aws-iot-example-project LANGUAGES C)

set(CMAKE_EXECUTABLE_SUFFIX ".axf")

if (INTEGRATION_TESTS EQUAL 0)
    # Set global optimization level to reduce code size while keeping the debug experience.
    if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        add_compile_options(-Og)
    elseif(${CMAKE_C_COMPILER_ID} STREQUAL "ARMClang")
        add_compile_options(-O1)
    endif()
endif()

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

# Add main application executable
add_executable(aws-iot-example
    mqtt_demo_pub_sub.c
    main.c
)
target_include_directories(aws-iot-example
    PUBLIC
        freertos_integration_tests
        ../helpers/provisioning
)

# Trusted Firmware-M must be built before the application, because
# the application depends on the NS interface and the BL2 signing scripts,
# both of which are generated as parts of the Trusted Firmware-M build process.
add_dependencies(aws-iot-example trusted_firmware-m-build)
# The provision data must be built before the application because
# it provides credentials to connect to AWS
add_dependencies(aws-iot-example provisioning_data_bin)
target_link_libraries(aws-iot-example
    PRIVATE
        backoff-algorithm
        connectivity-stack
        coremqtt
        coremqtt-agent
        corepkcs11
        freertos_kernel
        freertos-ota-pal-psa
        fri-bsp
        helpers-events
        mbedtls
        mbedtls-threading-freertos
        ota-for-aws-iot-embedded-sdk
        provisioning-lib
        tfm-ns-interface
        toolchain-override
)

set_linker_script(aws-iot-example)

# The non-secure application image should be padded while being signed
# Hence, passing "TRUE" as the input parameter to the pad option of sign function.
iot_reference_arm_corstone3xx_tf_m_sign_image(
    aws-iot-example
    aws-iot-example_signed
    ${MCUBOOT_IMAGE_VERSION_NS}
    TRUE
)

# The update image is not padded to fill the whole slot (no --pad), because
# 1) the image to download is smaller without padding
# 2) the trailer that keeps track of boot and update statuses should not be overwritten
# Hence, passing "FALSE" as the input parameter for the pad option to the sign function.
iot_reference_arm_corstone3xx_tf_m_sign_image(
    aws-iot-example
    aws-iot-example-update_signed
    ${MCUBOOT_IMAGE_VERSION_NS_UPDATE}
    FALSE
)

# A user project that consumes the ARM FRI needs to explicitly provide
# addresses in order to merge images for TF-M. The addresses cannot
# be easily programmatically extracted as they are defined in the linker
# scripts.
iot_reference_arm_corstone3xx_tf_m_merge_images(
    aws-iot-example
    ${BL2_IMAGE_LOAD_ADDRESS}
    ${S_IMAGE_LOAD_ADDRESS}
    ${NS_IMAGE_LOAD_ADDRESS}
    ${S_PROVISIONING_BUNDLE_LOAD_ADDRESS}
    ${NS_PROVISIONING_BUNDLE_LOAD_ADDRESS}
    ${CMAKE_BINARY_DIR}/helpers/provisioning/provisioning_data.bin
)

# Generate the AWS OTA update digest and signature
iot_reference_arm_corstone3xx_generate_aws_update_digest_and_signature(
    aws-iot-example
    aws-iot-example-update_signed
    update-digest
    update-signature
)

# ---------------------------------------------------------------------------------------------------------
# Configure FreeRTOS Libraries Integration Tests
add_dependencies(freertos-libraries-integration-tests-config provisioning_data_bin)
target_sources(freertos-libraries-integration-tests-config
    INTERFACE
        freertos_integration_tests/integration_tests_platform_function.c
)
target_include_directories(freertos-libraries-integration-tests-config
    INTERFACE
        freertos_integration_tests
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

# Add FreeRTOS Libraries Integration Tests application executable
add_executable(aws-iot-example-tests
    mqtt_demo_pub_sub.c
    main.c
)
target_include_directories(aws-iot-example-tests
    PUBLIC
        freertos_integration_tests
)

target_compile_definitions(aws-iot-example-tests
    PRIVATE
        INTEGRATION_TESTS=1
)
# Trusted Firmware-M must be built before the application, because
# the application depends on the NS interface and the BL2 signing scripts,
# both of which are generated as parts of the Trusted Firmware-M build process.
add_dependencies(aws-iot-example-tests trusted_firmware-m-build)
# The provision data must be built before the application because
# it provides credentials to connect to AWS
add_dependencies(aws-iot-example-tests provisioning_data_bin)
target_link_libraries(aws-iot-example-tests
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
        mbedtls-threading-freertos
        tfm-ns-interface
)

set_linker_script(aws-iot-example-tests)

# The non-secure application image should be padded while being signed
iot_reference_arm_corstone3xx_tf_m_sign_image(
    aws-iot-example-tests
    aws-iot-example-tests_signed
    ${MCUBOOT_IMAGE_VERSION_NS}
    TRUE
)

# The update image is not padded to fill the whole slot (no --pad), because
# 1) the image to download is smaller without padding
# 2) the trailer that keeps track of boot and update statuses should not be overwritten
iot_reference_arm_corstone3xx_tf_m_sign_image(
    aws-iot-example-tests
    aws-iot-example-tests-update_signed
    ${MCUBOOT_IMAGE_VERSION_NS_UPDATE}
    FALSE
)

# A user project that consumes the ARM FRI needs to explicitly provide
# addresses in order to merge images for TF-M. The addresses cannot
# be easily programmatically extracted as they are defined in the linker
# scripts.
iot_reference_arm_corstone3xx_tf_m_merge_images(
    aws-iot-example-tests
    ${BL2_IMAGE_LOAD_ADDRESS}
    ${S_IMAGE_LOAD_ADDRESS}
    ${NS_IMAGE_LOAD_ADDRESS}
    ${S_PROVISIONING_BUNDLE_LOAD_ADDRESS}
    ${NS_PROVISIONING_BUNDLE_LOAD_ADDRESS}
    ${CMAKE_BINARY_DIR}/helpers/provisioning/provisioning_data.bin
)

# Generate the AWS OTA update digest and signature
iot_reference_arm_corstone3xx_generate_aws_update_digest_and_signature(
    aws-iot-example-tests
    aws-iot-example-tests-update_signed
    update-digest
    update-signature
)
