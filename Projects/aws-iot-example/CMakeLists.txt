# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21)

# Start of the project
project(aws-iot-example-project LANGUAGES C CXX)

# Declare the target of the total solution
set(ARM_CORSTONE_BSP_TARGET_PLATFORM "" CACHE STRING "Featured Reference Integration target")

# Configure target
if (${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone300")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/corstone300/fvp")
    set( FREERTOS_PORT "GCC_ARM_CM55_TFM" CACHE STRING "" FORCE)
elseif(${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone310")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/corstone310/fvp")
    set( FREERTOS_PORT "GCC_ARM_CM85_TFM" CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "Invalid ARM_CORSTONE_BSP_TARGET_PLATFORM (${ARM_CORSTONE_BSP_TARGET_PLATFORM}) set. Supported are corstone300/corstone310")
endif()

set(TFM_PLATFORM_UPGRADE_STRATEGY "SWAP_USING_SCRATCH")
set(TFM_PLATFORM_CONFIRM_IMAGE ON)


set(TFM_PLATFORM "${TFM_PLATFORM_LOCAL_PATH}")
set(TFM_PARTITION_FIRMWARE_UPDATE ON)
set(TFM_PSA_FIRMWARE_UPDATE ON)
set(CONFIG_TFM_ENABLE_CP10CP11 ON)

# From: ota-for-aws-iot-embedded-sdk/source/include/ota_appversion32.h
# struct version
# {
#     uint8_t major;  /*!< @brief Major version number of the firmware (X in firmware version X.Y.Z). */
#     uint8_t minor;  /*!< @brief Minor version number of the firmware (Y in firmware version X.Y.Z). */
#
#     uint16_t build; /*!< @brief Build of the firmware (Z in firmware version X.Y.Z). */
# } x;                /*!< @brief Version number of the firmware. */

# AWS OTA client doesn't use the patch version. Therefore, build version from TF-M is used. Because of this,
# if only patch version is changed then the OTA will be rejected due to same firmware version.
set(MCUBOOT_IMAGE_VERSION_NS "0.0.1+10")
set(MCUBOOT_IMAGE_VERSION_NS_UPDATE "0.0.1+20")

# Extra arguments for TF-M and ML
set(TFM_CMAKE_ARGS
    -DPROJECT_CONFIG_HEADER_FILE=${PRJ_DIR}/Config/tfm-config/project_config.h
    -DPLATFORM_DEFAULT_PROVISIONING=OFF
    -DTFM_DUMMY_PROVISIONING=OFF
    # -DMCUBOOT_GENERATE_SIGNING_KEYPAIR=ON
    -DCONFIG_TFM_ENABLE_CP10CP11=ON
    -DTFM_EXCEPTION_INFO_DUMP=ON
    -DNS=ON
    -DPLATFORM_DEFAULT_UART_STDOUT=ON
    -DMCUBOOT_SIGNATURE_KEY_LEN=3072
    -DMCUBOOT_LOG_LEVEL=INFO
    -DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_INFO
    -DTFM_PARTITION_CRYPTO=ON
    -DTFM_PARTITION_FIRMWARE_UPDATE=ON
    -DTFM_PARTITION_INITIAL_ATTESTATION=ON
    -DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=ON
    -DTFM_PARTITION_PLATFORM=ON
    -DTFM_PARTITION_PROTECTED_STORAGE=ON
    -DTFM_PLATFORM=${TFM_PLATFORM_LOCAL_PATH}
    -DPLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT=ON
    -DMCUBOOT_DATA_SHARING=ON
    -DMCUBOOT_CONFIRM_IMAGE=${TFM_PLATFORM_CONFIRM_IMAGE}
    -DMCUBOOT_UPGRADE_STRATEGY=${TFM_PLATFORM_UPGRADE_STRATEGY}
    -DMCUBOOT_IMAGE_VERSION_NS=${MCUBOOT_IMAGE_VERSION_NS}
)

# Configuration of executable
set(EXE_SUFFIX ".axf")
set(CMAKE_EXECUTABLE_SUFFIX ${EXE_SUFFIX})

# Set global optimization level to reduce code size while keeping the debug experience.
# ARMClang does not have -Og but officially recommends -O1 for debug.
if (INTEGRATION_TESTS)
else()
    if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        add_compile_options(-Og)
    else()
        add_compile_options(-O1)
    endif()
endif()

# Select connectivity stack.
set(CONNECTIVITY_STACK "FREERTOS_TCP_IP" CACHE STRING "Choose the connectivity stack to use. Possible values are `FREERTOS_TCP_IP | IOT_VSOCKET`.")

set(MIDDLEWARE_DIR "../../Middleware")

add_subdirectory(${MIDDLEWARE_DIR}/ARM ${CMAKE_BINARY_DIR}/Middleware/ARM)
add_subdirectory(../../Config ${CMAKE_BINARY_DIR}/Config)
add_subdirectory(../../Bsp ${CMAKE_BINARY_DIR}/Bsp)


target_compile_definitions(arm-corstone-platform-bsp
    INTERFACE
        __DOMAIN_NS=1
)

# Compilation Database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
    INTERFACE
        ${PRJ_DIR}/Config/freertos-config
)

target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)

target_link_libraries(freertos_config
    INTERFACE
        tfm-ns-interface
        app-config
        fri-bsp
)

set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)

add_subdirectory(${MIDDLEWARE_DIR}/FreeRTOS ${CMAKE_BINARY_DIR}/Middleware/FreeRTOS)
add_subdirectory(${MIDDLEWARE_DIR}/AWS ${CMAKE_BINARY_DIR}/Middleware/AWS)

# integration tests
include(${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/qualification_test.cmake)
include(${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/transport_interface_test.cmake)
include(${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/ota_pal_test.cmake)
include(${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/mqtt_test.cmake)
# override provisioning in PKCS11 tests with our provisioning
set(PKCS11_TEST_SOURCES
    "${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/pkcs11/core_pkcs11_test.c")
set(PKCS11_TEST_INCLUDE_DIRS
    "${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/pkcs11"
    "${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/common")

add_library(unity STATIC)

target_sources(unity
    PRIVATE
        ${MIDDLEWARE_DIR}/Unity/src/unity.c
        ${MIDDLEWARE_DIR}/Unity/extras/fixture/src/unity_fixture.c
        ${MIDDLEWARE_DIR}/Unity/extras/memory/src/unity_memory.c
)

target_include_directories(unity
    PUBLIC
        ${MIDDLEWARE_DIR}/Unity/src
        ${MIDDLEWARE_DIR}/Unity/extras/fixture/src
        ${MIDDLEWARE_DIR}/Unity/extras/memory/src
)

add_library(integration-test-task STATIC)

target_sources(integration-test-task
    PRIVATE
        ${TRANSPORT_TEST_SOURCES}
        ${PKCS11_TEST_SOURCES}
        ${OTA_PAL_TEST_SOURCES}
        ${MQTT_TEST_SOURCES}
        ${QUALIFICATION_TEST_SOURCES}
        freertos-integration-tests/integration_tests_platform_function.c
)

target_include_directories(integration-test-task
    PRIVATE
        freertos-integration-tests/include
        ${TRANSPORT_TEST_INCLUDE_DIRS}
        ${PKCS11_TEST_INCLUDE_DIRS}
        ${OTA_PAL_TEST_INCLUDE_DIRS}
        ${MQTT_TEST_INCLUDE_DIRS}
        ${QUALIFICATION_TEST_INCLUDE_DIRS}
        ${MIDDLEWARE_DIR}/AWS/aws-iot-device-sdk-lib/corePKCS11/source/include
        ${MIDDLEWARE_DIR}/AWS/aws-iot-device-sdk-lib/corePKCS11/source/dependency/3rdparty/pkcs11
        ${MIDDLEWARE_DIR}/ARM/mbedtls-lib/mbedtls/include
        ${MIDDLEWARE_DIR}/AWS/aws-iot-device-sdk-lib/ota-for-aws-iot-embedded-sdk/source/include/
        ${MIDDLEWARE_DIR}/ARM/freertos-ota-pal-psa-lib/freertos-ota-pal-psa
        ${MIDDLEWARE_DIR}/AWS/aws-iot-device-sdk-lib/corePKCS11/source/dependency/3rdparty/mbedtls_utils
)

target_link_libraries(integration-test-task
    PRIVATE
        freertos_kernel
        aws-configs
        app-logging
        unity
        awsIoT
        connectivity-stack
        mbedtls
        fri-bsp
)

add_subdirectory(event-helper)
add_subdirectory(mqtt-agent-wrapper)
add_subdirectory(provisioning provisioning_data)

# Declare the aws-iot-example executable
add_executable(aws-iot-example
    main.c
    mqtt_demo_pub_sub.c
    dev_mode_key_provisioning.c
    ${MIDDLEWARE_DIR}/AWS/corePKCS11/source/dependency/3rdparty/mbedtls_utils/mbedtls_utils.c
)

add_dependencies(aws-iot-example provisioning_data_bin)

target_include_directories(aws-iot-example
    PRIVATE
        .
        freertos-integration-tests/include
        provisioning
)

if (INTEGRATION_TESTS)
    target_compile_definitions(aws-iot-example
        PRIVATE
            INTEGRATION_TESTS=1
    )
endif()

target_link_libraries(aws-iot-example
    PRIVATE
        app-config
        app-logging
        awsIoT
        mqtt-agent-task

        event-helper

        freertos-ota-pal-psa

        mbedtls
        mbedtls-threading-freertos

        freertos_kernel
        tfm-ns-interface
        fri-bsp
        $<$<BOOL:${INTEGRATION_TESTS}>:integration-test-task>
)

include(${PRJ_DIR}/Bsp/cmake/BspUtilities.cmake)
set_linker_script(aws-iot-example)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Middleware/ARM/TF-M/cmake)
include(SignTfmImage)
include(MergeTfmImages)

# The non-secure application image should be padded while being signed
# Hence, passing "TRUE" as the input parameter to the pad option of sign function.
iot_reference_arm_corstone3xx_tf_m_sign_image(aws-iot-example aws-iot-example_signed ${MCUBOOT_IMAGE_VERSION_NS} TRUE)

# The update image is not padded to fill the whole slot (no --pad), because
# 1) the image to download is smaller without padding
# 2) the trailer that keeps track of boot and update statuses should not be overwritten
# Hence, passing "FALSE" as the input parameter for the pad option to the sign function.
iot_reference_arm_corstone3xx_tf_m_sign_image(aws-iot-example aws-iot-example-update_signed ${MCUBOOT_IMAGE_VERSION_NS_UPDATE} FALSE)

# A user project that consumes the ARM FRI needs to explicitly provide
# addresses in order to merge images for TF-M. The addresses cannot
# be easily programmatically extracted as they are defined in the linker
# scripts.
iot_reference_arm_corstone3xx_tf_m_merge_images(aws-iot-example ${BL2_IMAGE_LOAD_ADDRESS} ${S_IMAGE_LOAD_ADDRESS} ${NS_IMAGE_LOAD_ADDRESS} ${S_PROVISIONING_BUNDLE_LOAD_ADDRESS} ${NS_PROVISIONING_BUNDLE_LOAD_ADDRESS} ${CMAKE_BINARY_DIR}/Projects/aws-iot-example/provisioning_data/provisioning_data.bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Middleware/AWS/cmake)
include(GenerateAWSUpdateDigestAndSignature)

iot_reference_arm_corstone3xx_generate_aws_update_digest_and_signature(aws-iot-example aws-iot-example-update_signed update-digest update-signature)
