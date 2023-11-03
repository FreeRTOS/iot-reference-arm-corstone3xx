# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

project(iot-reference-arm-corstone3xx LANGUAGES C ASM)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Tools/cmake)

add_subdirectory(bsp)
add_subdirectory(components EXCLUDE_FROM_ALL)
