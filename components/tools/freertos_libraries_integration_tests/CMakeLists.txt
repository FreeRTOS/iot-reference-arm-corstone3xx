# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

set(freertos_libraries_integration_tests_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to FreeRTOS-Libraries-Integration-Tests source code"
)

include(ApplyPatches)

set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
set(PATCH_FILES
    "${PATCH_FILES_DIRECTORY}/0001-Reduce-the-TRANSPORT_TEST_BUFFER_WRITABLE_LENGTH.patch"
    "${PATCH_FILES_DIRECTORY}/0002-closefile-validsignature-test-Fix-test-bugs.patch"
    "${PATCH_FILES_DIRECTORY}/0003-Introduce-user-defined-MQTT-keep-alive-value.patch"
)
iot_reference_arm_corstone3xx_apply_patches("${freertos_libraries_integration_tests_SOURCE_DIR}" "${PATCH_FILES}")

add_subdirectory(integration)
