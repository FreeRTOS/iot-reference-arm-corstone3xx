# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Logging
add_subdirectory(logging)

# coreMQTT-Agent
add_library(awsIoT STATIC)

include(coreMQTT-Agent/mqttAgentFilePaths.cmake)

target_include_directories(awsIoT
    PUBLIC
        ${MQTT_AGENT_INCLUDE_PUBLIC_DIRS}
)

target_sources(awsIoT
    PRIVATE
        ${MQTT_AGENT_SOURCES}
)

# coreMQTT
include(coreMQTT/mqttFilePaths.cmake)

target_include_directories(awsIoT
    PUBLIC
        ${MQTT_INCLUDE_PUBLIC_DIRS}
)

target_sources(awsIoT
    PRIVATE
        ${MQTT_SOURCES}
        ${MQTT_SERIALIZER_SOURCES}
)

target_compile_definitions(awsIoT
    PUBLIC
        MQTT_DO_NOT_USE_CUSTOM_CONFIG=1
        MQTT_AGENT_DO_NOT_USE_CUSTOM_CONFIG=1
)

# coreJSON
include(coreJSON/jsonFilePaths.cmake)

target_include_directories(awsIoT
    PUBLIC
        ${JSON_INCLUDE_PUBLIC_DIRS}
)

target_sources(awsIoT
    PRIVATE
        ${JSON_SOURCES}
)

# corePKCS11
execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_SOURCE_DIR}/patches/corePKCS11/0001-Do-not-define-__PASTE-macro-for-Arm-compiler.patch
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/corePKCS11/source/dependency/3rdparty/pkcs11"
    OUTPUT_QUIET
    ERROR_QUIET
)

include(corePKCS11/pkcsFilePaths.cmake)

target_include_directories(awsIoT
    PUBLIC
        ${PKCS_INCLUDE_PUBLIC_DIRS}
)

target_sources(awsIoT
    PRIVATE
        corePKCS11/source/core_pkcs11.c
        corePKCS11/source/core_pki_utils.c
)

# backoff_algorithm
include(backoffAlgorithm/backoffAlgorithmFilePaths.cmake)

target_include_directories(awsIoT
    PUBLIC
        ${BACKOFF_ALGORITHM_INCLUDE_PUBLIC_DIRS}
)

target_sources(awsIoT
    PRIVATE
        ${BACKOFF_ALGORITHM_SOURCES}
)

# OTA
execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_SOURCE_DIR}/patches/ota-for-aws-iot-embedded-sdk/0001-Replace-strnlen-with-strlen.patch
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ota-for-aws-iot-embedded-sdk"
    OUTPUT_QUIET
    ERROR_QUIET
)

set( TINYCBOR_SOURCES
    tinycbor/src/cborpretty.c
    tinycbor/src/cborpretty_stdio.c
    tinycbor/src/cborencoder.c
    tinycbor/src/cborencoder_close_container_checked.c
    tinycbor/src/cborerrorstrings.c
    tinycbor/src/cborparser.c
    tinycbor/src/cborparser_dup_string.c
)
set(TINYCBOR_INCLUDE_DIRS
    tinycbor/src
)

# OTA library source files, including 3rdparties.
set( OTA_SOURCES
    ota-for-aws-iot-embedded-sdk/source/include/ota.h
    ota-for-aws-iot-embedded-sdk/source/include/ota_os_interface.h
    ota-for-aws-iot-embedded-sdk/source/include/ota_platform_interface.h
    ota-for-aws-iot-embedded-sdk/source/include/ota_private.h
    ota-for-aws-iot-embedded-sdk/source/include/ota_interface_private.h
    ota-for-aws-iot-embedded-sdk/source/include/ota_base64_private.h
    ota-for-aws-iot-embedded-sdk/source/ota.c
    ota-for-aws-iot-embedded-sdk/source/ota_interface.c
    ota-for-aws-iot-embedded-sdk/source/ota_base64.c
    ${JSON_SOURCES}
    ${TINYCBOR_SOURCES}
)

# OTA library public include directories.
set( OTA_INCLUDE_PUBLIC_DIRS
    ota-for-aws-iot-embedded-sdk/source/include
    ota-for-aws-iot-embedded-sdk/source/portable
)

# OTA library FreeRTOS OS porting source files.
set( OTA_OS_FREERTOS_SOURCES
    ota-for-aws-iot-embedded-sdk/source/portable/os/ota_os_freertos.c
)

# OTA library FreeRTOS OS porting source files.
set( OTA_INCLUDE_OS_FREERTOS_DIRS
    ota-for-aws-iot-embedded-sdk/source/portable/os
)

# OTA library MQTT backend source files.
set( OTA_MQTT_SOURCES
    ota-for-aws-iot-embedded-sdk/source/ota_mqtt.c
    ota-for-aws-iot-embedded-sdk/source/ota_cbor.c
    ota-for-aws-iot-embedded-sdk/source/include/ota_mqtt_private.h
    ota-for-aws-iot-embedded-sdk/source/include/ota_cbor_private.h
)

target_include_directories(awsIoT
    PUBLIC
        ${OTA_INCLUDE_PUBLIC_DIRS}
        ${OTA_INCLUDE_OS_FREERTOS_DIRS}
        ${TINYCBOR_INCLUDE_DIRS}
        ${JSON_INCLUDE_PUBLIC_DIRS}
)

target_sources(awsIoT
    PRIVATE
        ${OTA_SOURCES}
        ${OTA_OS_FREERTOS_SOURCES}
        ${OTA_MQTT_SOURCES}
)

target_link_libraries(awsIoT
    aws-configs
    app-logging
    freertos_kernel
    mbedtls
    freertos-pkcs11-psa
)

