# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    include(GoogleTest)

    function(iot_reference_arm_corstone3xx_add_test target)
        target_link_libraries(${target}
            PRIVATE
                GTest::gtest_main
        )

        target_compile_definitions (${target}
            PRIVATE
                UNIT_TESTING
        )

        gtest_discover_tests(${target})
    endfunction()
endif()
