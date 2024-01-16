# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(freertos_libraries_integration_tests_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to FreeRTOS-Libraries-Integration-Tests source code"
)

execute_process(COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/integration/patches/0001-Reduce-the-TRANSPORT_TEST_BUFFER_WRITABLE_LENGTH.patch
    WORKING_DIRECTORY "${freertos_libraries_integration_tests_SOURCE_DIR}"
    OUTPUT_QUIET
    ERROR_QUIET
)

add_subdirectory(integration)
