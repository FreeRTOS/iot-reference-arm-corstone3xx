# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21)

# Start of the project
project(aws-iot-example-project LANGUAGES C CXX)

# Declare the target of the total solution
set(ARM_CORSTONE_BSP_TARGET_PLATFORM "corstone300" CACHE STRING "Featured Reference Integration target")

# Configure target
if (${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone300")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/an552")
    set(TFM_PLATFORM_UPGRADE_STRATEGY "SWAP_USING_SCRATCH")
    set(TFM_PLATFORM_CONFIRM_IMAGE ON)
else()
    message(FATAL_ERROR "Invalid ARM_CORSTONE_BSP_TARGET_PLATFORM (${ARM_CORSTONE_BSP_TARGET_PLATFORM}) set. It should be corstone300")
endif()

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
# if only patch version is changed then the OTA will be rejected due to same firmwrae version.
set(MCUBOOT_IMAGE_VERSION_NS "0.0.1+10")
set(MCUBOOT_IMAGE_VERSION_NS_UPDATE "0.0.1+20")

# Extra arguments for TF-M and ML
set(TFM_CMAKE_ARGS
    -DPROJECT_CONFIG_HEADER_FILE=${PRJ_DIR}/Config/tfm-config/project_config.h
    -DCONFIG_TFM_ENABLE_CP10CP11=ON
    -DTFM_EXCEPTION_INFO_DUMP=ON
    -DNS=ON
    -DPLATFORM_DEFAULT_UART_STDOUT=ON
    -DMCUBOOT_SIGNATURE_KEY_LEN=2048
    -DMCUBOOT_LOG_LEVEL=INFO
    -DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_INFO
    -DTFM_PARTITION_CRYPTO=ON
    -DTFM_PARTITION_FIRMWARE_UPDATE=ON
    -DTFM_PARTITION_INITIAL_ATTESTATION=ON
    -DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=ON
    -DTFM_PARTITION_PLATFORM=ON
    -DTFM_PARTITION_PROTECTED_STORAGE=ON
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

set(MIDDLEWARE_DIR "../../Middleware")

add_subdirectory(${MIDDLEWARE_DIR}/ARM ${CMAKE_BINARY_DIR}/Middleware/ARM)
add_subdirectory(../../Bsp ${CMAKE_BINARY_DIR}/Bsp)
add_subdirectory(../../Config ${CMAKE_BINARY_DIR}/Config)

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
)

set( FREERTOS_HEAP "3" CACHE STRING "" FORCE)
# Select the native compile PORT
set( FREERTOS_PORT "GCC_ARM_CM55_TFM" CACHE STRING "" FORCE)

add_subdirectory(${MIDDLEWARE_DIR}/FreeRTOS ${CMAKE_BINARY_DIR}/Middleware/FreeRTOS)

# integration tests
include(${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/qualification_test.cmake)
include(${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/transport_interface_test.cmake)
include(${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/ota_pal_test.cmake)
include(${MIDDLEWARE_DIR}/FreeRTOS/FreeRTOS-Libraries-Integration-Tests/src/mqtt_test.cmake)
# overrride provisioning in PKCS11 tests with our provisioning
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
        iot-vsocket
        mbedtls
        fri-bsp
)

add_subdirectory(${MIDDLEWARE_DIR}/AWS ${CMAKE_BINARY_DIR}/Middleware/AWS)

# Copy tf-m binaries at the root
ExternalProject_Get_Property(tf-m-build BINARY_DIR)
ExternalProject_Get_Property(tf-m-build SOURCE_DIR)

add_custom_target(tfm-binaries
  BYPRODUCTS
    ${CMAKE_BINARY_DIR}/bootloader/bl2.axf
    ${CMAKE_BINARY_DIR}/secure_partition/tfm_s.axf
    ${CMAKE_BINARY_DIR}/secure_partition/tfm_s_unsigned.bin
    ${CMAKE_BINARY_DIR}/secure_partition/tfm_s_signed.bin

  COMMAND ${CMAKE_COMMAND} -E copy
    ${BINARY_DIR}/install/outputs/bl2.axf
    "${CMAKE_BINARY_DIR}/bootloader/bl2${CMAKE_EXECUTABLE_SUFFIX}"

    COMMAND ${CMAKE_COMMAND} -E copy
    ${BINARY_DIR}/install/outputs/tfm_s_signed.bin
    "${CMAKE_BINARY_DIR}/secure_partition/tfm_s_signed.bin"
)

add_dependencies(tfm-binaries tf-m-build)
add_dependencies(tfm-ns-interface tfm-binaries)

add_subdirectory(mqtt-agent-wrapper)

# Declare the aws-iot-example executable
add_executable(aws-iot-example
    main.c
    mqtt_demo_pub_sub.c
    dev_mode_key_provisioning.c
    ${MIDDLEWARE_DIR}/AWS/corePKCS11/source/dependency/3rdparty/mbedtls_utils/mbedtls_utils.c
)

target_include_directories(aws-iot-example
    PRIVATE
        freertos-integration-tests/include
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

        freertos-ota-pal-psa

        mbedtls
        mbedtls-threading-freertos

        freertos_kernel
        tfm-ns-interface
        fri-bsp
        $<$<BOOL:${INTEGRATION_TESTS}>:integration-test-task>
)

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    target_link_options(aws-iot-example
        PRIVATE
            "-T" "${PRJ_DIR}/Bsp/an552_ns.ld"
            "--specs=nosys.specs"
    )
    set(elf_to_bin_cmd ${GCC_ELF2BIN} -O binary ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example_unsigned.bin)
else()
    target_link_options(aws-iot-example
        PRIVATE
            "--scatter=${PRJ_DIR}/Bsp/an552_ns.sct"
            "--map"
    )
    set(elf_to_bin_cmd ${ARM_ELF2BIN} --bin --output ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example_unsigned.bin ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example${CMAKE_EXECUTABLE_SUFFIX} --bincombined)
endif()

add_custom_command(
    TARGET
        aws-iot-example
    POST_BUILD
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example${CMAKE_EXECUTABLE_SUFFIX}

    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example_unsigned.bin
        ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example-update_signed.bin
        ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example_signed.bin

    COMMAND
        ${elf_to_bin_cmd}

    COMMAND
        python3 ${BINARY_DIR}/install/image_signing/scripts/wrapper/wrapper.py
            -v ${MCUBOOT_IMAGE_VERSION_NS}
            --layout ${BINARY_DIR}/install/image_signing/layout_files/signing_layout_ns.o
            -k ${BINARY_DIR}/install/image_signing/keys/root-RSA-2048_1.pem
            --public-key-format full
            --align 1
            --pad
            --pad-header
            -H 0x400
            -s auto
            --measured-boot-record
            --confirm
            ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example_unsigned.bin
            ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example_signed.bin

    COMMAND
            # The update image is not padded to fill the whole slot (no --pad), because
            # 1) the image to download is smaller without padding
            # 2) the trailer that keeps track of boot and update statuses should not be overwritten
            python3 ${BINARY_DIR}/install/image_signing/scripts/wrapper/wrapper.py
                -v ${MCUBOOT_IMAGE_VERSION_NS_UPDATE}
                --layout ${BINARY_DIR}/install/image_signing/layout_files/signing_layout_ns.o
                -k ${BINARY_DIR}/install/image_signing/keys/root-RSA-2048_1.pem
                --public-key-format full
                --align 1
                --pad-header
                -H 0x400
                -s auto
                --measured-boot-record
                --confirm
                ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example_unsigned.bin
                ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example-update_signed.bin
)

add_custom_command(
    TARGET
        aws-iot-example
    POST_BUILD
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example-update_signed.bin

    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/update-digest.bin
        ${CMAKE_CURRENT_BINARY_DIR}/update-signature.bin
        ${CMAKE_CURRENT_BINARY_DIR}/update-signature.txt

    COMMAND
        openssl dgst -sha256 -binary -out ${CMAKE_CURRENT_BINARY_DIR}/update-digest.bin ${CMAKE_CURRENT_BINARY_DIR}/aws-iot-example-update_signed.bin

    COMMAND
        openssl pkeyutl -sign -pkeyopt digest:sha256 -pkeyopt rsa_padding_mode:pss -pkeyopt rsa_mgf1_md:sha256 -inkey ${BINARY_DIR}/install/image_signing/keys/root-RSA-2048_1.pem -in ${CMAKE_CURRENT_BINARY_DIR}/update-digest.bin -out ${CMAKE_CURRENT_BINARY_DIR}/update-signature.bin

    COMMAND
        openssl base64 -A -in ${CMAKE_CURRENT_BINARY_DIR}/update-signature.bin -out ${CMAKE_CURRENT_BINARY_DIR}/update-signature.txt

    COMMAND
        ${CMAKE_COMMAND} -E echo "Use this base 64 encoded signature in OTA job:"

    COMMAND
        ${CMAKE_COMMAND} -E cat ${CMAKE_CURRENT_BINARY_DIR}/update-signature.txt
)
