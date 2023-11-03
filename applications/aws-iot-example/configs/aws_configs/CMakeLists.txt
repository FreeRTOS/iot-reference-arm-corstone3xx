# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

target_include_directories(coremqtt-config
    INTERFACE
        .
)

target_link_libraries(coremqtt-config
    INTERFACE
        helpers-logging
)

target_compile_definitions(coremqtt-agent-config
    INTERFACE
        MQTT_AGENT_DO_NOT_USE_CUSTOM_CONFIG
)

target_include_directories(corepkcs11-config
    INTERFACE
        .
)

target_include_directories(freertos-ota-pal-psa-config
    INTERFACE
        .
)

target_include_directories(freertos-pkcs11-psa-config
    INTERFACE
        .
)

target_include_directories(ota-for-aws-iot-embedded-sdk-config
    INTERFACE
        .
)
