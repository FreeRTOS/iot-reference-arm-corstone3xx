# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # Left empty for future mocks.
else()
    set(aws-iot-core-mqtt-file-streams-embedded-c_SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/library
        CACHE INTERNAL
        "Path to MQTT File Streams source code"
    )

    include(ApplyPatches)

    set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
    set(PATCH_FILES
        "${PATCH_FILES_DIRECTORY}/0001-Use-custom-strnlen-implementation.patch"
    )
    iot_reference_arm_corstone3xx_apply_patches("${aws-iot-core-mqtt-file-streams-embedded-c_SOURCE_DIR}" "${PATCH_FILES}")

    add_subdirectory(integration)
endif()
