# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(library_mocks)
else ()
    set(iot_vsocket_SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/library
        CACHE INTERNAL
        "Path to Arm Virtual Hardware - IoT Virtual Socket source code"
    )

    include(ApplyPatches)

    set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
    set(PATCH_FILES "${PATCH_FILES_DIRECTORY}/0001-Replace-CMSIS-RTX-APIs-with-FreeRTOS-APIs.patch")
    iot_reference_arm_corstone3xx_apply_patches("${iot_vsocket_SOURCE_DIR}" "${PATCH_FILES}")

endif()
add_subdirectory(integration)
