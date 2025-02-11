# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

# NS target name the TF-M api_ns CMakeLists.txt uses
set(NS_TARGET_NAME object-detection)
set(ML_INFERENCE_ENGINE     "ETHOS" CACHE STRING "Machine Learning inference engine (ETHOS)")
set(AUDIO_SOURCE            "ROM"   CACHE STRING "Source of audio data (ROM | VSI)")
set(APPLICATION_PATH        "${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/applications/object_detection" CACHE STRING "Path to the application folder")
set(AWS_OTA_SIGNATURE_TYPE  "RSA-3072" CACHE STRING "Supported algorithms for signature validation [RSA-2048, RSA-3072, EC-P256, EC-P384]")

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
# MCUBOOT_IMAGE_VERSION_NS is passed to the TF-M build in TfmInitialCache.cmake
set(MCUBOOT_IMAGE_VERSION_NS_UPDATE "0.0.20+0")

if (${ML_INFERENCE_ENGINE} STREQUAL "ETHOS")
    set(ETHOS_U_NPU_ENABLED ON)
    set(ETHOS_U_NPU_TIMING_ADAPTER_ENABLED OFF)
else()
    set(ETHOS_U_NPU_ENABLED OFF)
endif()
set(ML_USE_CASE "object_detection")
set(ML_MODEL "GenerateObjectDetectionModel")
set(ML_USE_CASE_RESOURCES_FILE "${CMAKE_CURRENT_LIST_DIR}/resources/use_case_resources.json")

# Toolchain file has to be included before the very first project() call
include(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake/TfmNsToolchain.cmake)


project(object-detection LANGUAGES C CXX)

set_compiler_and_linker_flags()
include(${CONFIG_SPE_PATH}/config/cp_check.cmake)

add_subdirectory(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR} ${CMAKE_BINARY_DIR}/iot_reference_arm_corstone3xx)

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/aws_iot/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/ai/ml_embedded_evaluation_kit/integration/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake)
include(SetupMlEmbeddedEvaluationKitLibraries)
include(AWSIoTHelpers)
include(GenerateAWSUpdateDigestAndSignature)
include(MergeTfmImages)
include(SignTfmImage)

add_subdirectory(configs ${CMAKE_BINARY_DIR}/Config)
add_subdirectory(isp)
add_subdirectory(../helpers helpers)

add_executable(object-detection
    blink_task.c
    main.c
    ml_interface.cc
    ${CONFIG_SPE_PATH}/interface/src/os_wrapper/tfm_ns_interface_rtos.c
)

target_include_directories(object-detection
    PUBLIC
        ../helpers/provisioning
        ${CMAKE_CURRENT_LIST_DIR}
)

if (${ML_INFERENCE_ENGINE} STREQUAL "ETHOS")
    target_compile_definitions(object-detection PRIVATE USE_ETHOS)
    target_sources(object-detection PRIVATE ../helpers/ethosu/src/ethosu_platform_adaptation.c)
endif()

target_compile_options(object-detection
    PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++14>
        $<$<COMPILE_LANGUAGE:C>:-std=c99>
)

#FIXME USE RTE_USART2 for control tool
target_compile_definitions(arm-corstone-platform-bsp
    PUBLIC
        RTE_USART1=1
)

# The provision data must be built before the application because
# it provides credentials to connect to AWS.
add_dependencies(object-detection provisioning_data_bin)

target_link_libraries(object-detection
    PRIVATE
        arm-2d
        backoff-algorithm
        connectivity-stack
        coremqtt
        coremqtt-agent
        corepkcs11
        freertos_kernel
        freertos-ota-pal-psa
        fri-bsp
        helpers-events
        helpers-hdlcd
        isp-config
        isp_platform_driver
        mbedtls
        ota-update
        provisioning-lib
        tfm_api_ns
        toolchain-override
        object_detection_api
        object_detection_model
        helpers-logging
        # FRI always uses TrustZone
        tfm_api_ns_tz
)

include(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/cmake/SetLinkerOptions.cmake)
set_linker_script(object-detection)

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/cmake)
include(ConvertElfToBin)

extract_sections_from_axf(
    object-detection
    SECTIONS_NAMES   "ddr.bin"
    OUTPUT_BIN_NAME  "ns_image"
)

# Copy the binary flash content to the location expected by default signing
# Signing is implemented in the exported TF-M NS CMakeLists.txt (in the
# ${CONFIG_SPE_PATH} directory)
add_custom_target(object-detection_bin
    SOURCES ${CMAKE_BINARY_DIR}/object-detection.bin
    DEPENDS object-detection
)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/object-detection.bin
    DEPENDS object-detection
    COMMAND ${CMAKE_COMMAND}
        -E copy ${SECTORS_BIN_DIR}/ns_image.bin
        ${CMAKE_BINARY_DIR}/object-detection.bin
)

# The update image is not padded to fill the whole slot (no --pad), because
# 1) the image to download is smaller without padding
# 2) the trailer that keeps track of boot and update statuses should not be overwritten
# Hence, passing "FALSE" as the input parameter for the pad option to the sign function.
iot_reference_arm_corstone3xx_tf_m_sign_image(
    object-detection
    "ns_image"
    object-detection-update_signed
    ${MCUBOOT_IMAGE_VERSION_NS_UPDATE}
    "${CONFIG_SPE_PATH}/image_signing/layout_files/signing_layout_ns.o"
    FALSE
)


# A user project that consumes the ARM FIR needs to explicitly provide
# addresses in order to merge images for TF-M. The addresses cannot
# be easily programmatically extracted as they are defined in the linker
# scripts.
iot_reference_arm_corstone3xx_tf_m_merge_images(
    object-detection
    ${NS_PROVISIONING_BUNDLE_LOAD_ADDRESS}
    ${CMAKE_BINARY_DIR}/helpers/provisioning/provisioning_data.bin
    ${NS_DDR4_IMAGE_LOAD_ADDRESS}
    ${SECTORS_BIN_DIR}/ddr.bin
)

iot_reference_arm_corstone3xx_generate_aws_update_digest_and_signature(
    object-detection
    object-detection-update_signed
    update-digest
    update-signature
)
