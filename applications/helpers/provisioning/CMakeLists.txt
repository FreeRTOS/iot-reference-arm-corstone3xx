# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # left empty for future mocks.
else()
    find_package(Python3)

    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

    include(SetProvisioningLinkOptions)

    add_executable(provisioning_data provisioning_data.c)
    target_include_directories(provisioning_data
        PUBLIC
            ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/inc
    )

    # Only use interface includes for fri-bsp for provisioning config, public sources are not needed
    target_include_directories(provisioning_data PRIVATE $<TARGET_PROPERTY:fri-bsp,INTERFACE_INCLUDE_DIRECTORIES>)

    set(
        CODE_SIGNING_PUBLIC_KEY_PEM_PATH
        ${AWS_OTA_SIGNATURE_PUBLIC_KEY_PATH}
    )

    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/aws_clientcredential_keys.h
        DEPENDS ${AWS_CLIENT_PRIVATE_KEY_PEM_PATH}
        DEPENDS ${AWS_CLIENT_CERTIFICATE_PEM_PATH}
        DEPENDS aws_ota_signing_keys
        COMMAND
            ${Python3_EXECUTABLE} ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/tools/scripts/generate_credentials_header.py
                ${CMAKE_CURRENT_BINARY_DIR}
                --path-to-client-private-key-pem ${AWS_CLIENT_PRIVATE_KEY_PEM_PATH}
                --path-to-client-certificate-pem ${AWS_CLIENT_CERTIFICATE_PEM_PATH}
                --path-to-code-signing-public-key-pem ${CODE_SIGNING_PUBLIC_KEY_PEM_PATH}
    )

    add_custom_target(aws_clientcredential_keys_header
        SOURCES
            ${CMAKE_CURRENT_BINARY_DIR}/aws_clientcredential_keys.h
    )

    add_dependencies(provisioning_data aws_clientcredential_keys_header)

    if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        target_link_options(provisioning_data
            PRIVATE
            # We should define an entry point to override the default Reset_Handler
            "--entry=provisioningBundle"
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

    add_library(provisioning-lib
        dev_mode_key_provisioning.c
        ${corepkcs11_SOURCE_DIR}/source/dependency/3rdparty/mbedtls_utils/mbedtls_utils.c
    )

    target_link_libraries(provisioning-lib
        corepkcs11
        freertos_kernel
        mbedtls
    )

    target_compile_definitions(provisioning-lib
        PUBLIC
            $<$<STREQUAL:${AWS_OTA_SIG_TYPE},RSA>:AWS_OTA_SIGN_RSA>
            $<$<STREQUAL:${AWS_OTA_SIG_TYPE},EC>:AWS_OTA_SIGN_ECDSA>
            $<$<STREQUAL:${AWS_OTA_SIG_TYPE},EC>:AWS_OTA_ECDSA_HEADER_SIZE=26>

            AWS_OTA_SIGNATURE_KEY_LEN=${AWS_OTA_SIG_LEN}
    )
endif() # BUILD_TESTING AND NOT CMAKE_CROSS_COMPILING
