# Copyright 2021-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(ExternalProject)

# Use provided toolchain file as part of ml_embedded_evaluation_kit.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(ml_toolchain_file "scripts/cmake/toolchains/bare-metal-gcc.cmake")
elseif(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    set(ml_toolchain_file "scripts/cmake/toolchains/bare-metal-armclang.cmake")
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()

ExternalProject_Add(
    ml_embedded_evaluation_kit-build

    DOWNLOAD_COMMAND    ""
    SOURCE_DIR          ${ml_embedded_evaluation_kit_SOURCE_DIR}

    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD     ON

    BUILD_ALWAYS ON

    INSTALL_COMMAND ""

    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=<SOURCE_DIR>/${ml_toolchain_file}
        ${ML_CMAKE_ARGS}

    PATCH_COMMAND
        ${ML_PATCH_COMMAND}

    BUILD_COMMAND
        ${CMAKE_COMMAND} --build <BINARY_DIR> --target ${ML_TARGETS}
)

ExternalProject_Add_Step(ml_embedded_evaluation_kit-build
    download_model
    COMMAND
        python3 ${ml_embedded_evaluation_kit_SOURCE_DIR}/set_up_default_resources.py
    DEPENDERS
        configure
    USES_TERMINAL ON
)
