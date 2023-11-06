# Copyright 2022-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(speexdsp
    ${speexdsp_SOURCE_DIR}/libspeexdsp/fftwrap.c
    ${speexdsp_SOURCE_DIR}/libspeexdsp/filterbank.c
    ${speexdsp_SOURCE_DIR}/libspeexdsp/mdf.c
    ${speexdsp_SOURCE_DIR}/libspeexdsp/preprocess.c
    ${speexdsp_SOURCE_DIR}/libspeexdsp/kiss_fft.c
    ${speexdsp_SOURCE_DIR}/libspeexdsp/kiss_fftr.c
)

target_include_directories(speexdsp
    PUBLIC
        ${speexdsp_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/lib/speexdsp/include
        ${CMAKE_SOURCE_DIR}/lib/speexdsp/libspeexdsp
        inc
)

target_compile_definitions(speexdsp
    PUBLIC
        FIXED_POINT
        HAVE_CONFIG_H
        OS_SUPPORT_CUSTOM
)

target_compile_options(speexdsp
    PRIVATE
        -Wno-double-promotion
)
