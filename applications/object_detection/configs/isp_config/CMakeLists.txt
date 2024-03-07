# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(isp_firmware_config INTERFACE)

target_include_directories(isp_firmware_config
    INTERFACE
        .
)

target_link_libraries(isp_firmware_config
    INTERFACE
        arm-corstone-platform-bsp
)
