# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

find_package(Python3)

#FIXME: This is needed in order to run prepoccessor on the linker script.
#       The GCC prepoccessor puts std defines if -g3 is set.
if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    string(REPLACE "-g3" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
endif()

include(linkerscript_preprocess.cmake)

add_executable(provisioning_data
    provisioning_data.c
)
target_include_directories(provisioning_data PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/aws_clientcredential_keys.h
    DEPENDS ${AWS_CLIENT_PRIVATE_KEY_PEM_PATH}
    DEPENDS ${AWS_CLIENT_CERTIFICATE_PEM_PATH}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generate_credentials_header.py
                                    --client_private_key_pem ${AWS_CLIENT_PRIVATE_KEY_PEM_PATH}
                                    --client_certificate_pem ${AWS_CLIENT_CERTIFICATE_PEM_PATH}
)

add_custom_target(aws_clientcredential_keys_header
    SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/aws_clientcredential_keys.h
)

add_dependencies(provisioning_data aws_clientcredential_keys_header)

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    target_link_options(provisioning_data
        PRIVATE
            "--specs=nosys.specs"
            "-nostartfiles"
    )
    target_add_scatter_file(provisioning_data ${CMAKE_CURRENT_SOURCE_DIR}/provisioning_data.ld)
    set(provisioning_elf_to_bin_cmd ${GCC_ELF2BIN} -O binary ${CMAKE_CURRENT_BINARY_DIR}/provisioning_data${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/provisioning_data.bin)
else()
    target_add_scatter_file(provisioning_data ${CMAKE_CURRENT_SOURCE_DIR}/provisioning_data.sct)
    set(provisioning_elf_to_bin_cmd ${ARM_ELF2BIN} --bin --output ${CMAKE_CURRENT_BINARY_DIR}/provisioning_data.bin ${CMAKE_CURRENT_BINARY_DIR}/provisioning_data${CMAKE_EXECUTABLE_SUFFIX} --bincombined)
endif()

add_custom_command(OUTPUT provisioning_data.bin
    DEPENDS $<TARGET_FILE_DIR:provisioning_data>/provisioning_data${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND ${provisioning_elf_to_bin_cmd}
)

add_custom_target(provisioning_data_bin ALL
    SOURCES provisioning_data.bin
)

