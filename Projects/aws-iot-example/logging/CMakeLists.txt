# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# MQTT agent task library

add_library(app-logging STATIC iot_logging_task_dynamic_buffers.c)

target_include_directories(app-logging
    PUBLIC
        include
)

target_link_libraries(app-logging
    freertos_kernel
)
