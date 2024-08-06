# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Add helpers for testing mqtt_agent_task.c. Helpers are application-specific.
# E.g. app_config.h is a helper since it varies by application.
add_library(ota-for-aws-iot-embedded-sdk-test-specific-mock
    INTERFACE
)
target_include_directories(ota-for-aws-iot-embedded-sdk-test-specific-mock
    INTERFACE
        inc
)
target_link_libraries(ota-for-aws-iot-embedded-sdk-test-specific-mock
    INTERFACE
        fff
)
