# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${corejson_SOURCE_DIR}/jsonFilePaths.cmake)

add_library(corejson
    ${JSON_SOURCES}
)

target_include_directories(corejson
    PUBLIC
        ${JSON_INCLUDE_PUBLIC_DIRS}
)

add_library(corejson-config INTERFACE)

target_link_libraries(corejson
    PUBLIC
        corejson-config
)
