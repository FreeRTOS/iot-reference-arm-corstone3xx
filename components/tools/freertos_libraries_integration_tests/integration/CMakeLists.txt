# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(freertos-libraries-integration-tests-config INTERFACE)

include(${freertos_libraries_integration_tests_SOURCE_DIR}/qualification_test.cmake)
include(${freertos_libraries_integration_tests_SOURCE_DIR}/src/mqtt_test.cmake)
include(${freertos_libraries_integration_tests_SOURCE_DIR}/src/pkcs11_test.cmake)
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
# The MQTT server resets connection if the device doesn't send keep alive
# packets within keep-alive interval, which is configured to 5 seconds by
# default. Increase the keep-alive interval to 60 seconds to avoid connection
# reset request when the device is busy.
target_compile_definitions(freertos-libraries-integration-tests-mqtt
    PRIVATE
        MQTT_KEEP_ALIVE_INTERVAL_SECONDS=300
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

target_compile_definitions(freertos-libraries-integration-tests-pkcs11
    PRIVATE
        -DMBEDTLS_ALLOW_PRIVATE_ACCESS
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
