# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

find_package(Python3)

#FIXME: This is needed in order to run prepoccessor on the linker script.
#       The GCC prepoccessor puts std defines if -g3 is set.
if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    string(REPLACE "-g3" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(SetProvisioningLinkOptions)

add_executable(provisioning_data provisioning_data.c)
target_include_directories(provisioning_data
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_link_libraries(provisioning_data PRIVATE
    fri-bsp
)

ExternalProject_Get_Property(trusted_firmware-m-build BINARY_DIR)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/aws_clientcredential_keys.h
    DEPENDS ${AWS_CLIENT_PRIVATE_KEY_PEM_PATH}
    DEPENDS ${AWS_CLIENT_CERTIFICATE_PEM_PATH}
    COMMAND
        ${Python3_EXECUTABLE} ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/scripts/generate_credentials_header.py
            ${CMAKE_CURRENT_BINARY_DIR}
            --path-to-client-private-key-pem ${AWS_CLIENT_PRIVATE_KEY_PEM_PATH}
            --path-to-client-certificate-pem ${AWS_CLIENT_CERTIFICATE_PEM_PATH}
            --path-to-code-signing-public-key-pem ${BINARY_DIR}/api_ns/image_signing/keys/image_ns_signing_public_key.pem
)

add_custom_target(aws_clientcredential_keys_header
    SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/aws_clientcredential_keys.h
)

add_dependencies(aws_clientcredential_keys_header trusted_firmware-m-build)
add_dependencies(provisioning_data aws_clientcredential_keys_header)

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    target_link_options(provisioning_data
        PRIVATE
            "--specs=nosys.specs"
            "-nostartfiles"
    )
    target_add_scatter_file(provisioning_data ${CMAKE_CURRENT_LIST_DIR}/provisioning_data.ld)
else()
    target_add_scatter_file(provisioning_data ${CMAKE_CURRENT_LIST_DIR}/provisioning_data.sct)
endif()

list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/cmake)
include(ConvertElfToBin)

add_custom_command(OUTPUT provisioning_data.bin
    DEPENDS $<TARGET_FILE_DIR:provisioning_data>/provisioning_data${CMAKE_EXECUTABLE_SUFFIX}
)

target_elf_to_bin(provisioning_data provisioning_data)

add_custom_target(provisioning_data_bin ALL
    SOURCES provisioning_data.bin
)
