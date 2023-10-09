# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type" FORCE)
endif()

if (${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone300")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/corstone300/fvp")
    set( FREERTOS_PORT "GCC_ARM_CM55_TFM" CACHE STRING "" FORCE)
elseif(${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone310")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/corstone310/fvp")
    set( FREERTOS_PORT "GCC_ARM_CM85_TFM" CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "Invalid ARM_CORSTONE_BSP_TARGET_PLATFORM (${ARM_CORSTONE_BSP_TARGET_PLATFORM}) set. Supported are corstone300/corstone310")
endif()

set(TFM_CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCONFIG_TFM_ENABLE_CP10CP11=ON
    -DMCUBOOT_GENERATE_SIGNING_KEYPAIR=ON
    -DMCUBOOT_LOG_LEVEL=INFO
    -DMCUBOOT_SIGNATURE_KEY_LEN=3072
    -DNS=ON
    -DPLATFORM_DEFAULT_PROVISIONING=OFF
    -DPLATFORM_DEFAULT_UART_STDOUT=ON
    -DPROJECT_CONFIG_HEADER_FILE=${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/Projects/blinky/configs/tfm-config/project_config.h
    -DTFM_DUMMY_PROVISIONING=OFF
    -DTFM_EXCEPTION_INFO_DUMP=ON
    -DTFM_PARTITION_CRYPTO=ON
    -DTFM_PARTITION_INITIAL_ATTESTATION=ON
    -DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=ON
    -DTFM_PARTITION_PLATFORM=ON
    -DTFM_PARTITION_PROTECTED_STORAGE=ON
    -DTFM_PLATFORM=${TFM_PLATFORM_LOCAL_PATH}
    -DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_INFO
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(blinky-example LANGUAGES C)

set(CMAKE_EXECUTABLE_SUFFIX ".axf")

# Set global optimization level to reduce code size while keeping the debug experience.
if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    add_compile_options(-Og)
elseif(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    add_compile_options(-O1)
endif()

add_subdirectory(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR} ${CMAKE_BINARY_DIR}/iot_reference_arm_corstone3xx)

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/Bsp/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake)
include(BspUtilities)
include(MergeTfmImages)
include(SignTfmImage)

add_subdirectory(configs)

add_executable(blinky main.c)
# Trusted Firmware-M must be built before the application, because
# the application depends on the NS interface and the BL2 signing scripts,
# both of which are generated as parts of the Trusted Firmware-M build process.
add_dependencies(blinky trusted_firmware-m-build)
target_link_libraries(blinky
    freertos_kernel
    tfm-ns-interface
    fri-bsp
)

set_linker_script(blinky)

# The non-secure application image should be padded while being signed
# Hence, passing "TRUE" as the input parameter to the pad option of sign function.
iot_reference_arm_corstone3xx_tf_m_sign_image(blinky blinky_signed 0.0.1 TRUE)

# A user project that consumes the ARM FRI needs to explicitly provide
# addresses in order to merge images for TF-M. The addresses cannot
# be easily programmatically extracted as they are defined in the linker
# scripts.
iot_reference_arm_corstone3xx_tf_m_merge_images(blinky
    ${BL2_IMAGE_LOAD_ADDRESS}
    ${S_IMAGE_LOAD_ADDRESS}
    ${NS_IMAGE_LOAD_ADDRESS}
    ${S_PROVISIONING_BUNDLE_LOAD_ADDRESS}
)
