# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

target_include_directories(freertos_config SYSTEM
    INTERFACE
        .
)

target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)

target_link_libraries(freertos_config
    INTERFACE
        tfm_api_ns
        app-config
)
