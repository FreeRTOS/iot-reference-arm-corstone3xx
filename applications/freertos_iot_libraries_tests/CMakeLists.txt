# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

set(NS_TARGET_NAME freertos-iot-libraries-tests)
set(APPLICATION_PATH "${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/applications/freertos_iot_libraries_tests" CACHE STRING "Path to the application folder")
set(AWS_OTA_SIGNATURE_TYPE  "RSA-3072" CACHE STRING "Supported algorithms for signature validation [RSA-2048, RSA-3072, EC-P256, EC-P384]")

# Toolchain file has to be included before the very first project() call
include(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake/TfmNsToolchain.cmake)


project(freertos-iot-libraries-tests LANGUAGES C)

set_compiler_and_linker_flags()
include(${CONFIG_SPE_PATH}/config/cp_check.cmake)

# This variable is checked to apply configurations specific to FreeRTOS Libraries Integrations Tests
set(FREERTOS_LIBRARIES_INTEGRATION_TESTS 1)

add_subdirectory(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR} ${CMAKE_BINARY_DIR}/iot_reference_arm_corstone3xx)

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/aws_iot/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake)
include(SetLinkerOptions)
include(AWSIoTHelpers)
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
        freertos-libraries-integration-tests-qualification
        freertos-libraries-integration-tests-transport
        unity
)

add_executable(${CMAKE_PROJECT_NAME}
    main.c
    integration_tests_platform_function.c
    ${CONFIG_SPE_PATH}/interface/src/os_wrapper/tfm_ns_interface_rtos.c
)

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
        provisioning-lib
        mbedtls
        toolchain-override
        tfm_api_ns
        # FRI always uses TrustZone
        tfm_api_ns_tz
)

set_linker_script(${CMAKE_PROJECT_NAME})

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/cmake)
include(ConvertElfToBin)

extract_sections_from_axf(
    ${CMAKE_PROJECT_NAME}
    SECTIONS_NAMES   "ddr.bin"
    OUTPUT_BIN_NAME  "ns_image"
)

# Copy the binary flash content to the location expected by default signing
# Signing is implemented in the exported TF-M NS CMakeLists.txt (in the
# ${CONFIG_SPE_PATH} directory)
add_custom_target(freertos-iot-libraries-tests_bin
    SOURCES ${CMAKE_BINARY_DIR}/freertos-iot-libraries-tests.bin
    DEPENDS freertos-iot-libraries-tests
)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/freertos-iot-libraries-tests.bin
    DEPENDS freertos-iot-libraries-tests
    COMMAND ${CMAKE_COMMAND}
        -E copy ${SECTORS_BIN_DIR}/ns_image.bin
        ${CMAKE_BINARY_DIR}/freertos-iot-libraries-tests.bin
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
