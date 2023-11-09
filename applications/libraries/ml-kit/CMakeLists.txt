# Copyright 2022-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(ExternalProject)

ExternalProject_Get_Property(ml_embedded_evaluation_kit-build BINARY_DIR)

# ML-eval Kit
add_library(ml-kit INTERFACE)

# Set byproducts from ML build, so that CMake treats them as generated files
# that don't need to exist before the build.

set(ml_kit_libs
    ${BINARY_DIR}/lib/libcommon_api.a
    ${BINARY_DIR}/lib/libtensorflow-microlite.a
    ${BINARY_DIR}/lib/libarm_math.a
    ${BINARY_DIR}/lib/libcmsis-dsp.a
)

if(${ML_INFERENCE_ENGINE} STREQUAL "ETHOS")
    list(APPEND ml_kit_libs
        ${BINARY_DIR}/lib/libethos_u_npu.a
        ${BINARY_DIR}/lib/libethosu_core_driver.a
    )

    if(${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone300")
        list(APPEND ml_kit_libs ${BINARY_DIR}/lib/libtiming_adapter.a)
    endif()
endif()

set(ml_kit_kws_libs
    ${BINARY_DIR}/lib/libkws_api.a
    ${BINARY_DIR}/lib/libkws.a
)
set(ml_kit_asr_libs
    ${BINARY_DIR}/lib/libasr_api.a
    ${BINARY_DIR}/lib/libasr.a
)

add_custom_target(get_ml_kit_libs
    DEPENDS
        ml_embedded_evaluation_kit-build
    COMMAND
        # Dummy command that does nothing
        ${CMAKE_COMMAND} -E true
    BYPRODUCTS
        ${ml_kit_libs} ${ml_kit_kws_libs} ${ml_kit_asr_libs}
)


target_include_directories(ml-kit
    INTERFACE
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/cmsis-dsp/Include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/core-driver/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/tensorflow
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/tensorflow/tensorflow/lite/micro
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/tensorflow/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/tensorflow/tensorflow/lite/micro/tools/make/downloads/gemmlowp/
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/application/hal/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/application/main/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/application/api/common/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/hal/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/hal/source/platform/mps3/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/log/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/hal/source/components/platform_pmu/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/hal/source/components/npu/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/hal/source/components/stdout/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/hal/source/components/lcd/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/profiler/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/hal/source/platform/mps3/source
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/core-platform/drivers/timing_adapter/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/math/include
)

if(${ARM_CORSTONE_BSP_TARGET_PLATFORM} STREQUAL "corstone300")
    target_include_directories(ml-kit
        INTERFACE
            ${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/core-platform/drivers/timing_adapter/include
    )
endif()


target_link_libraries(ml-kit
    INTERFACE
        ${ml_kit_libs}
)

target_compile_definitions(ml-kit
    INTERFACE
        -DACTIVATION_BUF_SZ=0x00200000
        -DARM_MATH_DSP
        -DARM_MATH_LOOPUNROLL
        -DARM_MATH_MVEI
        -DARM_NPU=1
        -DTF_LITE_STATIC_MEMORY
        -DETHOS_U_NPU_MEMORY_MODE=ETHOS_U_NPU_MEMORY_MODE_SHARED_SRAM
)

add_dependencies(ml-kit
    ml_embedded_evaluation_kit-build
)


# Add kws library
add_library(ml-kit-kws INTERFACE)

target_link_libraries(ml-kit-kws
    INTERFACE
        ${ml_kit_kws_libs}
)

target_link_libraries(ml-kit-kws
    INTERFACE
        ml-kit
)

target_include_directories(ml-kit-kws
    INTERFACE
        ${BINARY_DIR}/generated/kws/include/
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/generated/kws/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/use_case/kws/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/application/api/use_case/kws/include
)

# Remove retarget from kws library
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_custom_target(ml-kit-remove-kws-retarget
        COMMAND arm-none-eabi-ar dv ${BINARY_DIR}/lib/libkws.a retarget.c.obj
    )
elseif(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    add_custom_target(ml-kit-remove-kws-retarget
        COMMAND armar -d ${BINARY_DIR}/lib/libkws.a retarget.o
    )
endif()

add_dependencies(ml-kit-remove-kws-retarget ml-kit)
add_dependencies(ml-kit-kws ml-kit-remove-kws-retarget)


# Add asr library
add_library(ml-kit-asr INTERFACE)

target_link_libraries(ml-kit-asr
    INTERFACE
        ${ml_kit_asr_libs}
)

target_link_libraries(ml-kit-asr
    INTERFACE
        ml-kit
)

target_include_directories(ml-kit-asr
    INTERFACE
        ${BINARY_DIR}/generated/asr/include/
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/generated/asr/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/use_case/asr/include
        ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/application/api/use_case/asr/include
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_custom_target(ml-kit-remove-asr-retarget
        COMMAND arm-none-eabi-ar dv ${BINARY_DIR}/lib/libasr.a retarget.c.obj
    )
elseif(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    add_custom_target(ml-kit-remove-asr-retarget
        COMMAND armar -d ${BINARY_DIR}/lib/libasr.a retarget.o
    )
endif()

add_dependencies(ml-kit-remove-asr-retarget ml-kit)
add_dependencies(ml-kit-asr ml-kit-remove-asr-retarget)
