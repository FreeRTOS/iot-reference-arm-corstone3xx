# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

# NS target name the TF-M api_ns CMakeLists.txt uses
set(NS_TARGET_NAME blinky)
set(APPLICATION_PATH "${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/applications/blinky" CACHE STRING "Path to the application folder")

# Toolchain file has to be included before the very first project() call
include(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake/TfmNsToolchain.cmake)

project(blinky-example LANGUAGES C)

set_compiler_and_linker_flags()
include(${CONFIG_SPE_PATH}/config/cp_check.cmake)

add_subdirectory(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR} ${CMAKE_BINARY_DIR}/iot_reference_arm_corstone3xx)

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/cmake)
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake)
include(SetLinkerOptions)
include(MergeTfmImages)
include(SignTfmImage)

add_subdirectory(configs)

add_executable(blinky
    main.c
    ${CONFIG_SPE_PATH}/interface/src/os_wrapper/tfm_ns_interface_rtos.c
)

target_link_libraries(blinky
    freertos_kernel
    fri-bsp
    tfm_api_ns
    toolchain-override
    # FRI always uses TrustZone
    tfm_api_ns_tz
)

set_linker_script(blinky)

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/cmake)
include(ConvertElfToBin)

extract_sections_from_axf(
    blinky
    SECTIONS_NAMES   "ddr.bin"
    OUTPUT_BIN_NAME  "ns_image"
)

# Copy the binary flash content to the location expected by default signing
# Signing is implemented in the exported TF-M NS CMakeLists.txt (in the
# ${CONFIG_SPE_PATH} directory)
add_custom_target(blinky_bin
    SOURCES ${CMAKE_BINARY_DIR}/blinky.bin
    DEPENDS blinky
)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/blinky.bin
    DEPENDS blinky
    COMMAND ${CMAKE_COMMAND}
        -E copy ${SECTORS_BIN_DIR}/ns_image.bin
        ${CMAKE_BINARY_DIR}/blinky.bin
)

# A user project that consumes the ARM FRI needs to explicitly provide
# addresses in order to merge images for TF-M. The addresses cannot
# be easily programmatically extracted as they are defined in the linker
# scripts.
iot_reference_arm_corstone3xx_tf_m_merge_images(blinky)
