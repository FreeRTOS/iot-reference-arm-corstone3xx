# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

project(iot-reference-arm-corstone3xx LANGUAGES C CXX ASM)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/tools/cmake)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type" FORCE)
endif()

if(CMAKE_CROSSCOMPILING)
    set(CMAKE_EXECUTABLE_SUFFIX ".axf" PARENT_SCOPE)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)
endif()

# CMAKE_PROJECT_NAME is the sub-project to build. E.g. 'blinky-example', or any other application.
# We only unit test if the build script has set CMAKE_PROJECT_NAME to the name of the project, and
# not an application's name (i.e. 'iot-reference-arm-corstone3xx').
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    include(AddUnitTest)
    # Include helpers mocks.
    add_subdirectory(applications/helpers)
endif()


if(CMAKE_CROSSCOMPILING)
    add_subdirectory(bsp)
    add_subdirectory(components EXCLUDE_FROM_ALL)
else()
    add_subdirectory(components)
endif()
