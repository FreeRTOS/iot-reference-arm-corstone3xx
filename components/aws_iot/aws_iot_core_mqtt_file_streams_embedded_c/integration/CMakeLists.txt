# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(aws-iot-core-mqtt-file-streams-embedded-c
    ${aws-iot-core-mqtt-file-streams-embedded-c_SOURCE_DIR}/source/MQTTFileDownloader_base64.c
    ${aws-iot-core-mqtt-file-streams-embedded-c_SOURCE_DIR}/source/MQTTFileDownloader_cbor.c
    ${aws-iot-core-mqtt-file-streams-embedded-c_SOURCE_DIR}/source/MQTTFileDownloader.c
)

target_include_directories(aws-iot-core-mqtt-file-streams-embedded-c
    PUBLIC
        ${aws-iot-core-mqtt-file-streams-embedded-c_SOURCE_DIR}/source/include/
)

target_link_libraries(aws-iot-core-mqtt-file-streams-embedded-c
    PUBLIC
        corejson
        coremqtt
        coremqtt-agent
        crt-helpers
        tinycbor
)
