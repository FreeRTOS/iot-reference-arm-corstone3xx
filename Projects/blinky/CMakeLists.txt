# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21)

# Start of the project
project(blinky-example LANGUAGES C CXX)

# Declare the target of the total solution
set(ARM_CORSTONE_BSP_TARGET_PLATFORM "corstone300" CACHE STRING "Featured Reference Integration target")

# Configure target
if (${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone300")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/an552")
else()
    message(FATAL_ERROR "Invalid ARM_CORSTONE_BSP_TARGET_PLATFORM (${ARM_CORSTONE_BSP_TARGET_PLATFORM}) set. It should be corstone300")
endif()

# Extra arguments for TF-M and ML
set(TFM_CMAKE_ARGS
    -DPROJECT_CONFIG_HEADER_FILE=${PRJ_DIR}/Config/tfm-config/project_config.h
    -DPLATFORM_DEFAULT_PROVISIONING=OFF
    -DTFM_DUMMY_PROVISIONING=OFF
    -DCONFIG_TFM_ENABLE_CP10CP11=ON
    -DTFM_EXCEPTION_INFO_DUMP=ON
    -DNS=ON;
    -DPLATFORM_DEFAULT_UART_STDOUT=ON;
    -DMCUBOOT_SIGNATURE_KEY_LEN=2048
    -DMCUBOOT_LOG_LEVEL=INFO
    -DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_INFO
    -DTFM_PARTITION_CRYPTO=ON
    -DTFM_PARTITION_INITIAL_ATTESTATION=ON
    -DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=ON
    -DTFM_PARTITION_PLATFORM=ON
    -DTFM_PARTITION_PROTECTED_STORAGE=ON
    -DTFM_PLATFORM=${TFM_PLATFORM_LOCAL_PATH}
)

# Configuration of executable
set(EXE_SUFFIX ".axf")
set(CMAKE_EXECUTABLE_SUFFIX ${EXE_SUFFIX})

# Set global optimization level to reduce code size while keeping the debug experience.
# ARMClang does not have -Og but officially recommends -O1 for debug.
if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    add_compile_options(-Og)
else()
    add_compile_options(-O1)
endif()

add_subdirectory(../../Middleware/ARM ${CMAKE_BINARY_DIR}/Middleware/ARM)
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

add_subdirectory(../../Middleware/FreeRTOS ${CMAKE_BINARY_DIR}/Middleware/FreeRTOS)

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

# Declare the blinky executable
add_executable(blinky main.c)

target_link_libraries(blinky
    freertos_kernel
    tfm-ns-interface
    fri-bsp
)

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    target_link_options(blinky
        PRIVATE
            "-T" "${PRJ_DIR}/Bsp/an552_ns.ld"
            "--specs=nosys.specs"
    )
    set(elf_to_bin_cmd ${GCC_ELF2BIN} -O binary ${CMAKE_CURRENT_BINARY_DIR}/blinky${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/blinky_unsigned.bin)
else()
    target_link_options(blinky
        PRIVATE
            "--scatter=${PRJ_DIR}/Bsp/an552_ns.sct"
            "--map"
    )
    set(elf_to_bin_cmd ${ARM_ELF2BIN} --bin --output ${CMAKE_CURRENT_BINARY_DIR}/blinky_unsigned.bin ${CMAKE_CURRENT_BINARY_DIR}/blinky${CMAKE_EXECUTABLE_SUFFIX} --bincombined)
endif()

add_custom_command(
    TARGET
        blinky
    POST_BUILD
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/blinky${CMAKE_EXECUTABLE_SUFFIX}

    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/blinky_unsigned.bin
        ${CMAKE_CURRENT_BINARY_DIR}/blinky_signed.bin

    COMMAND
        ${elf_to_bin_cmd}

    COMMAND
        python3 ${BINARY_DIR}/install/image_signing/scripts/wrapper/wrapper.py
            -v 0.0.1
            --layout ${BINARY_DIR}/install/image_signing/layout_files/signing_layout_ns.o
            -k ${BINARY_DIR}/install/image_signing/keys/root-RSA-2048_1.pem
            --public-key-format full
            --align 1
            --pad
            --pad-header
            -H 0x400
            -s auto
            --measured-boot-record
            ${CMAKE_CURRENT_BINARY_DIR}/blinky_unsigned.bin
            ${CMAKE_CURRENT_BINARY_DIR}/blinky_signed.bin
)
