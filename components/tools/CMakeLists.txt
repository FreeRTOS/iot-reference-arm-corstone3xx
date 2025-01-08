# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(fff)
    add_subdirectory(googletest)
else()
    add_subdirectory(freertos_libraries_integration_tests)
    add_subdirectory(unity)
endif()
