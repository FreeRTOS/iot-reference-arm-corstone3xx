# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21)

project(arm-featured-reference-integration)

# Declare root and other build paths
set(PRJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Add examples targets
if(EXAMPLE STREQUAL "blinky")
    add_subdirectory(Projects/blinky)
endif()
