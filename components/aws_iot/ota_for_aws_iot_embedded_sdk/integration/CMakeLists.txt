# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${ota_for_aws_iot_embedded_sdk_SOURCE_DIR}/otaFilePaths.cmake)

add_library(ota-for-aws-iot-embedded-sdk
    ${OTA_SOURCES}
    ${OTA_MQTT_SOURCES}
    ${OTA_OS_FREERTOS_SOURCES}
    src/ota_agent_task.c
)

target_include_directories(ota-for-aws-iot-embedded-sdk
    PUBLIC
        ${OTA_INCLUDE_PUBLIC_DIRS}
        ${OTA_INCLUDE_OS_FREERTOS_DIRS}
)

add_library(ota-for-aws-iot-embedded-sdk-config INTERFACE)

target_link_libraries(ota-for-aws-iot-embedded-sdk
    PUBLIC
        ota-for-aws-iot-embedded-sdk-config
    PRIVATE
        corejson
        coremqtt
        coremqtt-agent
        freertos-ota-pal-psa
        helpers-logging
        tinycbor
)
