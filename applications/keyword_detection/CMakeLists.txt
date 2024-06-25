# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

set(ML_INFERENCE_ENGINE "ETHOS" CACHE STRING "Machine Learning inference engine (ETHOS | SOFTWARE)")

set(AUDIO_SOURCE "ROM" CACHE STRING "Source of audio data (ROM | VSI)")

set(APPLICATION_PATH "${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/applications/keyword_detection" CACHE STRING "Path to the application folder")

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

if (${ML_INFERENCE_ENGINE} STREQUAL "ETHOS")
    set(ETHOS_U_NPU_ENABLED ON)
    set(ETHOS_U_NPU_TIMING_ADAPTER_ENABLED OFF)
else()
    set(ETHOS_U_NPU_ENABLED OFF)
endif()
set(ML_USE_CASE "kws")
set(ML_MODEL "GenerateKWSModel")
set(ML_USE_CASE_RESOURCES_FILE "${CMAKE_CURRENT_LIST_DIR}/resources/use_case_resources.json")
set(TFM_PLATFORM_UPGRADE_STRATEGY "SWAP_USING_SCRATCH")
set(TFM_PLATFORM_CONFIRM_IMAGE ON)

# Trusted Firmware-M setup
set(TFM_CMAKE_APP_ARGS
    -DPROJECT_CONFIG_HEADER_FILE=${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/applications/keyword_detection/configs/tfm_config/project_config.h
    -DMCUBOOT_CONFIRM_IMAGE=${TFM_PLATFORM_CONFIRM_IMAGE}
    -DMCUBOOT_UPGRADE_STRATEGY=${TFM_PLATFORM_UPGRADE_STRATEGY}
    -DMCUBOOT_IMAGE_VERSION_NS=${MCUBOOT_IMAGE_VERSION_NS}
    -DCONFIG_TFM_HALT_ON_CORE_PANIC=ON
    -DMCUBOOT_DATA_SHARING=ON
    -DPLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT=ON
    -DTFM_PARTITION_FIRMWARE_UPDATE=ON
    -DTFM_PARTITION_LOG_LEVEL=TFM_PARTITION_LOG_LEVEL_INFO
)

project(keyword-detection LANGUAGES C CXX)

# Set global optimization level to reduce code size while keeping the debug experience.
if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    add_compile_options(-Og)
elseif(${CMAKE_C_COMPILER_ID} STREQUAL "ARMClang")
    add_compile_options(-O1)
endif()


add_subdirectory(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR} ${CMAKE_BINARY_DIR}/iot_reference_arm_corstone3xx)

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/aws_iot/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/ai/ml_embedded_evaluation_kit/integration/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake)
include(SetupMlEmbeddedEvaluationKitLibraries)
include(ConvertAudioSourceToCode)
include(GenerateAWSUpdateDigestAndSignature)
include(MergeTfmImages)
include(SignTfmImage)

add_subdirectory(configs ${CMAKE_BINARY_DIR}/Config)
add_subdirectory(../helpers helpers)

add_executable(keyword-detection
    blink_task.c
    main.c
    ml_interface.cc
    model_config.cc
)

target_include_directories(keyword-detection
    PUBLIC
        ../helpers/provisioning
)

if(${AUDIO_SOURCE} STREQUAL "ROM")
    iot_reference_arm_corstone3xx_convert_audio_source_to_code(
        ${CMAKE_CURRENT_LIST_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    target_sources(keyword-detection
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/InputFiles.cc
            ${CMAKE_CURRENT_BINARY_DIR}/test.cc
    )

    target_include_directories(keyword-detection
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
    )
elseif(${AUDIO_SOURCE} STREQUAL "VSI")
    target_compile_definitions(keyword-detection
        PRIVATE
            AUDIO_VSI
    )
endif()

if (${ML_INFERENCE_ENGINE} STREQUAL "ETHOS")
    target_compile_definitions(keyword-detection PRIVATE USE_ETHOS)
    target_sources(keyword-detection PRIVATE ../helpers/ethosu/src/ethosu_platform_adaptation.c)
endif()

target_compile_options(keyword-detection
    PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++14>
        $<$<COMPILE_LANGUAGE:C>:-std=c99>
)

# Trusted Firmware-M must be built before the application, because
# the application depends on the NS interface and the BL2 signing scripts,
# both of which are generated as parts of the Trusted Firmware-M build process.
add_dependencies(keyword-detection trusted_firmware-m-build)
# The provision data must be built before the application because
# it provides credentials to connect to AWS.
add_dependencies(keyword-detection provisioning_data_bin)

target_link_libraries(keyword-detection
    PRIVATE
        backoff-algorithm
        connectivity-stack
        coremqtt
        coremqtt-agent
        corepkcs11
        freertos_kernel
        freertos-ota-pal-psa
        fri-bsp
        helpers-device-advisor
        helpers-events
        mbedtls
        ota-for-aws-iot-embedded-sdk
        provisioning-lib
        tfm-ns-interface
        toolchain-override
        kws_api
        kws_model
)

# sntp helper library depends on FreeRTOS-Plus-TCP connectivity stack as it
# includes `FreeRTOS_IP.h` header file in one of its source files (sntp_client_task.c),
# thus this library is only added in case of using FREERTOS_PLUS_TCP connectivity stack.
if(CONNECTIVITY_STACK STREQUAL "FREERTOS_PLUS_TCP")
    target_link_libraries(keyword-detection
        PRIVATE
            coresntp
            helpers-sntp
    )
endif()

include(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/cmake/SetLinkerOptions.cmake)
set_linker_script(keyword-detection)

# The non-secure application image should be padded while being signed
# Hence, passing "TRUE" as the input parameter to the pad option of sign function.
iot_reference_arm_corstone3xx_tf_m_sign_image(
    keyword-detection
    keyword-detection_signed
    ${MCUBOOT_IMAGE_VERSION_NS}
    TRUE
)

# The update image is not padded to fill the whole slot (no --pad), because
# 1) the image to download is smaller without padding
# 2) the trailer that keeps track of boot and update statuses should not be overwritten
# Hence, passing "FALSE" as the input parameter for the pad option to the sign function.
iot_reference_arm_corstone3xx_tf_m_sign_image(
    keyword-detection
    keyword-detection-update_signed
    ${MCUBOOT_IMAGE_VERSION_NS_UPDATE}
    FALSE
)


# A user project that consumes the ARM FIR needs to explicitly provide
# addresses in order to merge images for TF-M. The addresses cannot
# be easily programmatically extracted as they are defined in the linker
# scripts.
iot_reference_arm_corstone3xx_tf_m_merge_images(
    keyword-detection
    ${NS_PROVISIONING_BUNDLE_LOAD_ADDRESS}
    ${CMAKE_BINARY_DIR}/helpers/provisioning/provisioning_data.bin
    ${NS_DDR4_IMAGE_LOAD_ADDRESS}
    ${SECTORS_BIN_DIR}/ddr.bin
)

iot_reference_arm_corstone3xx_generate_aws_update_digest_and_signature(
    keyword-detection
    keyword-detection-update_signed
    update-digest
    update-signature
)
