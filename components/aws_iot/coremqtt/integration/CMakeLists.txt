# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${coremqtt_SOURCE_DIR}/mqttFilePaths.cmake)

add_library(coremqtt
    ${MQTT_SOURCES}
    ${MQTT_SERIALIZER_SOURCES}
)

target_include_directories(coremqtt
    PUBLIC
        ${MQTT_INCLUDE_PUBLIC_DIRS}
)

add_library(coremqtt-config INTERFACE)

target_link_libraries(coremqtt
    PUBLIC
        coremqtt-config
    PRIVATE
        freertos_kernel
        helpers-logging
)
