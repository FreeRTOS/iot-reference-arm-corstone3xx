# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

project(iot-reference-arm-corstone3xx LANGUAGES C ASM)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/tools/cmake)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type" FORCE)
endif()

set(CMAKE_EXECUTABLE_SUFFIX ".axf" PARENT_SCOPE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)

add_subdirectory(bsp)
add_subdirectory(components EXCLUDE_FROM_ALL)
