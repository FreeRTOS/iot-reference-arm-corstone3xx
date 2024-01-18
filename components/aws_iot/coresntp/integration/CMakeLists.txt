# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${coresntp_SOURCE_DIR}/coreSntpFilePaths.cmake)

add_library(coresntp
    ${CORE_SNTP_SOURCES}
)

target_include_directories(coresntp
    PUBLIC
        ${CORE_SNTP_INCLUDE_PUBLIC_DIRS}
)

add_library(coresntp-config INTERFACE)

target_link_libraries(coresntp
    PUBLIC
        coresntp-config
    PRIVATE
        helpers-logging
)
