# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-libraries-integration-tests-config INTERFACE)

include(${freertos_libraries_integration_tests_SOURCE_DIR}/qualification_test.cmake)
include(${freertos_libraries_integration_tests_SOURCE_DIR}/src/mqtt_test.cmake)
include(${freertos_libraries_integration_tests_SOURCE_DIR}/src/pkcs11_test.cmake)
include(${freertos_libraries_integration_tests_SOURCE_DIR}/src/ota_pal_test.cmake)
include(${freertos_libraries_integration_tests_SOURCE_DIR}/src/transport_interface_test.cmake)

add_library(freertos-libraries-integration-tests-mqtt
    ${MQTT_TEST_SOURCES}
)
target_include_directories(freertos-libraries-integration-tests-mqtt
    PUBLIC
        ${MQTT_TEST_INCLUDE_DIRS}
)
target_link_libraries(freertos-libraries-integration-tests-mqtt
    PRIVATE
        coremqtt
        freertos-libraries-integration-tests-config
        unity
)

add_library(freertos-libraries-integration-tests-ota-pal
    ${OTA_PAL_TEST_SOURCES}
)
target_include_directories(freertos-libraries-integration-tests-ota-pal
    PUBLIC
        ${OTA_PAL_TEST_INCLUDE_DIRS}
)
target_link_libraries(freertos-libraries-integration-tests-ota-pal
    PRIVATE
        freertos-libraries-integration-tests-config
        freertos-ota-pal-psa
        ota-for-aws-iot-embedded-sdk
        unity
)

add_library(freertos-libraries-integration-tests-pkcs11
    ${PKCS11_TEST_SOURCES}
)
# Do not use ${PKCS11_TEST_INCLUDE_DIRS} so applications can provide provisioning
target_include_directories(freertos-libraries-integration-tests-pkcs11
    PUBLIC
        ${freertos_libraries_integration_tests_SOURCE_DIR}/src/pkcs11
        ${freertos_libraries_integration_tests_SOURCE_DIR}/src/common
)
target_link_libraries(freertos-libraries-integration-tests-pkcs11
    PRIVATE
        corepkcs11
        freertos-libraries-integration-tests-config
        mbedtls
        unity
)

add_library(freertos-libraries-integration-tests-qualification
    ${QUALIFICATION_TEST_SOURCES}
)
target_include_directories(freertos-libraries-integration-tests-qualification
    PUBLIC
        ${QUALIFICATION_TEST_INCLUDE_DIRS}
        ${freertos_libraries_integration_tests_SOURCE_DIR}/src/common
        ${TRANSPORT_TEST_INCLUDE_DIRS}
)
target_link_libraries(freertos-libraries-integration-tests-qualification
    PRIVATE
        coremqtt
        freertos-libraries-integration-tests-config
        freertos-libraries-integration-tests-mqtt
        freertos-libraries-integration-tests-ota-pal
        freertos-libraries-integration-tests-pkcs11
)

add_library(freertos-libraries-integration-tests-transport
    ${TRANSPORT_TEST_SOURCES}
)
target_include_directories(freertos-libraries-integration-tests-transport
    PUBLIC
        ${TRANSPORT_TEST_INCLUDE_DIRS}
)
target_link_libraries(freertos-libraries-integration-tests-transport
    PRIVATE
        coremqtt
        freertos-libraries-integration-tests-config
        unity
)
