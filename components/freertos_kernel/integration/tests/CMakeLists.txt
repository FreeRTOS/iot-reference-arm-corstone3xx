# Copyright 2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

add_executable(heap-management-test
    test_heap_management.cpp
)

target_include_directories(heap-management-test
    PRIVATE
        ../inc
        ../../library_mocks/inc
)

target_link_libraries(heap-management-test
    PRIVATE
        fff
        freertos-kernel-mock
        heap-management
)

target_link_options(heap-management-test 
    PRIVATE
        -Wl,--wrap=malloc
        -Wl,--wrap=calloc
        -Wl,--wrap=free
)

iot_reference_arm_corstone3xx_add_test(heap-management-test)
