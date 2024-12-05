# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(jobs-for-aws-iot-embedded-sdk
    ${jobs-for-aws-iot-embedded-sdk_SOURCE_DIR}/source/otaJobParser/job_parser.c
    ${jobs-for-aws-iot-embedded-sdk_SOURCE_DIR}/source/otaJobParser/ota_job_handler.c
    ${jobs-for-aws-iot-embedded-sdk_SOURCE_DIR}/source/jobs.c
)

target_include_directories(jobs-for-aws-iot-embedded-sdk
    PUBLIC
        ${jobs-for-aws-iot-embedded-sdk_SOURCE_DIR}/source/include/
        ${jobs-for-aws-iot-embedded-sdk_SOURCE_DIR}/source/otaJobParser/include/
)

target_link_libraries(jobs-for-aws-iot-embedded-sdk
    PUBLIC
        freertos_kernel
        corejson
        coremqtt
        coremqtt-agent
        crt-helpers
        tinycbor
)
