# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # left empty for future mocks.
else()
    add_library(helpers-device-advisor
        src/aws_device_advisor_task.c
    )

    target_include_directories(helpers-device-advisor
        PUBLIC
            inc
    )

    target_link_libraries(helpers-device-advisor
        coremqtt
        coremqtt-agent
        helpers-events
    )
endif()
