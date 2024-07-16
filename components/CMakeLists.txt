# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(CMAKE_CROSSCOMPILING)
    # FreeRTOS must be first as other components depend on it
    add_subdirectory(freertos_kernel)

    add_subdirectory(ai EXCLUDE_FROM_ALL)
    add_subdirectory(aws_iot EXCLUDE_FROM_ALL)
    add_subdirectory(graphics EXCLUDE_FROM_ALL)
    add_subdirectory(connectivity EXCLUDE_FROM_ALL)
    add_subdirectory(security EXCLUDE_FROM_ALL)
    add_subdirectory(tools EXCLUDE_FROM_ALL)
elseif(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # Test tools must be available before any component as they are used for unit testing
    add_subdirectory(tools)

    # FreeRTOS must be included before everything except tools as other components depend on it
    add_subdirectory(freertos_kernel)

    add_subdirectory(aws_iot)

    add_subdirectory(connectivity)

    add_subdirectory(security)
endif()
