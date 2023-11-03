# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(helpers-logging
    src/iot_logging_task_dynamic_buffers.c
)

target_include_directories(helpers-logging
    PUBLIC
        inc
)

target_link_libraries(helpers-logging
    freertos_kernel
)
