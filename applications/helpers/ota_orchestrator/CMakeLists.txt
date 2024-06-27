# Copyright 2024-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # Left empty for future mocks.
else()
    add_library(ota-update
        src/mqtt_helpers.c
        src/ota_orchestrator_helpers.c
        src/ota_os_freertos.c
        src/ota_orchestrator.c
    )

    target_include_directories(ota-update
        PUBLIC
            inc/
    )

    target_compile_definitions(ota-update
        PRIVATE
            -DMCUBOOT_IMAGE_NUMBER=${MCUBOOT_IMAGE_NUMBER}
    )

    target_link_libraries(ota-update
    PUBLIC
        jobs-for-aws-iot-embedded-sdk
        aws-iot-core-mqtt-file-streams-embedded-c
        freertos_kernel
        corejson
        coremqtt
        coremqtt-agent
        tinycbor
        freertos-ota-pal-psa
        helpers-events
        backoff-algorithm
        crt-helpers
    PRIVATE
        helpers-logging
    )
endif()
