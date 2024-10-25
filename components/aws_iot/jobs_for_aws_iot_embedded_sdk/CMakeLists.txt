# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # Left empty for future mocks.
else()
    set(jobs-for-aws-iot-embedded-sdk_SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/library
        CACHE INTERNAL
        "Path to AWS IoT Jobs source code"
    )

    add_subdirectory(integration)
endif()
