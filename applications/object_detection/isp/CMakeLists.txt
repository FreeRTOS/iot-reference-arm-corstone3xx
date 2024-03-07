# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(isp-config INTERFACE)

target_include_directories(isp-config
    INTERFACE
        .
)

target_sources(isp-config
    INTERFACE
        acamera_callbacks.c
        isp_app.c
        isp_config.c
)

target_link_libraries(isp-config
    INTERFACE
        isp_control
        helpers-logging
)
