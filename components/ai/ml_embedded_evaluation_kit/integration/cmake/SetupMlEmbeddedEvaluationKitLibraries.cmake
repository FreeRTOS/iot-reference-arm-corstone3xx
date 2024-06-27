# Copyright 2021-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(ExternalProject)

###########################
# Setup for the ML target #
###########################
#set(LOG_LEVEL LOG_LEVEL_TRACE)

# External repositories
set(CMSIS_SRC_PATH              "${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/cmsis")
set(CMSIS_DSP_SRC_PATH          "${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/cmsis-dsp")
set(CMSIS_DSP_INC_DIR           "${CMSIS_DSP_SRC_PATH}/Include")
set(CMSIS_CORE_INC_DIR          "${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/CMSIS/Core/Include")
set(CMSIS_NN_SRC_PATH           "${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/cmsis-nn")
set(TENSORFLOW_SRC_PATH         "${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/tensorflow")
set(ETHOS_U_NPU_DRIVER_SRC_PATH "${ml_embedded_evaluation_kit_SOURCE_DIR}/dependencies/core-driver")

set(MLEK_RESOURCES_DIR       ${CMAKE_CURRENT_BINARY_DIR}/mlek_resources_downloaded)

# Extra arguments for setting up default resources (for vela optimizer)
set(ML_RESOURCES_SET_UP_ARGS
    "--additional-ethos-u-config-name=${ETHOSU_TARGET_NPU_CONFIG}"
    "--use-case-resources-file=${ML_USE_CASE_RESOURCES_FILE}"
    "--downloaded-model-resources-path=${MLEK_RESOURCES_DIR}"
)

# Tensorflow settings
set(TENSORFLOW_LITE_MICRO_BUILD_TYPE "release_with_logs")
set(TENSORFLOW_LITE_MICRO_CLEAN_DOWNLOADS OFF)
set(TENSORFLOW_LITE_MICRO_CLEAN_BUILD ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tensorflow-microlite)

# Set up default resources. This downloads the TF-Lite models and optimizes them for the target
# Note: This step is using pip install in its venv setup, which involves installing Vela compiler.
#       If it is not already installed, it will use CC to compile it to the host machine, but the toolchain file overwrites the CC env.
set(RESOURCES_OUTFILE "${MLEK_RESOURCES_DIR}/resources_downloaded_metadata.json")

# Do not re-download the resources if they have already been downloaded
if(NOT EXISTS "${RESOURCES_OUTFILE}")
    execute_process(
        COMMAND
            ${CMAKE_COMMAND}
                -E env CC=gcc
                ${PYTHON} ${ml_embedded_evaluation_kit_SOURCE_DIR}/set_up_default_resources.py
                    ${ML_RESOURCES_SET_UP_ARGS}
        RESULT_VARIABLE return_code
    )
    if (NOT return_code EQUAL "0")
        message(FATAL_ERROR "Failed to set up default resources.")
    endif ()
endif()

# Setup virtualenv (done by setup_source_generator())
set(CMAKE_SCRIPTS_DIR   ${ml_embedded_evaluation_kit_SOURCE_DIR}/scripts/cmake)
include(${CMAKE_SCRIPTS_DIR}/source_gen_utils.cmake)
set(MLEK_SCRIPTS_DIR         ${ml_embedded_evaluation_kit_SOURCE_DIR}/scripts)
setup_source_generator()

# Used by tensorflow_lite_micro.cmake
# Function to check if a variable is defined, and throw
# an error if it is not.
function(assert_defined var_name)
    if (NOT DEFINED ${var_name})
        message(FATAL_ERROR "ERROR: ${var_name} is undefined!")
    endif()
endfunction()

include(${CMAKE_SCRIPTS_DIR}/tensorflow_lite_micro.cmake)

# Manually add libs
add_subdirectory(${ml_embedded_evaluation_kit_SOURCE_DIR}/source/log ${CMAKE_BINARY_DIR}/log)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SCRIPTS_DIR})
add_subdirectory(${ml_embedded_evaluation_kit_SOURCE_DIR}/source/math ${CMAKE_BINARY_DIR}/math)
target_include_directories(arm_math PUBLIC
    ${CMSIS_DSP_INC_DIR}
    ${CMSIS_CORE_INC_DIR}
)
add_subdirectory(${ml_embedded_evaluation_kit_SOURCE_DIR}/source/application/api/common ${CMAKE_BINARY_DIR}/common_api)
if (ETHOS_U_NPU_ENABLED)
    add_subdirectory(${ml_embedded_evaluation_kit_SOURCE_DIR}/source/hal/source/components/npu ${CMAKE_BINARY_DIR}/npu)
    # ethos_u_npu library needs the CPU Header (CMSIS_device_header)
    target_link_libraries(ethos_u_npu PUBLIC arm-corstone-platform-bsp)
endif()

# Add the dependency on tensorflow_build (defined in tensorflow.cmake)
add_dependencies(common_api tensorflow_build)
target_include_directories(common_api PUBLIC ${TFLITE_MICRO_PATH})
target_compile_options(common_api PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++14>)

# Add relevant use case API
add_subdirectory(${ml_embedded_evaluation_kit_SOURCE_DIR}/source/application/api/use_case/${ML_USE_CASE} ${CMAKE_BINARY_DIR}/${ML_USE_CASE}_api)

# Include directories for application module:
set(APPLICATION_INCLUDE_DIRS ${ml_embedded_evaluation_kit_SOURCE_DIR}/source/application/main/include)
target_include_directories(${ML_USE_CASE}_api PUBLIC
    ${APPLICATION_INCLUDE_DIRS}
)

# Generate use case C model from optimized tflite file
# Generate tflite model code
set(DEFAULT_MODEL_DIR ${MLEK_RESOURCES_DIR}/${ML_USE_CASE})
set(SRC_GEN_DIR ${CMAKE_BINARY_DIR}/generated/${ML_USE_CASE}/src)
set(INC_GEN_DIR ${CMAKE_BINARY_DIR}/generated/${ML_USE_CASE}/include)

# Remove old files and recreate dirs
file(REMOVE_RECURSE ${SRC_GEN_DIR} ${INC_GEN_DIR})
file(MAKE_DIRECTORY ${SRC_GEN_DIR} ${INC_GEN_DIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/model)
include(${ML_MODEL})

file(GLOB_RECURSE SRC_GEN
    "${SRC_GEN_DIR}/*.cc"
    "${SRC_GEN_DIR}/*.cpp"
    "${SRC_GEN_DIR}/*.c")

set(UC_LIB_NAME ${ML_USE_CASE}_model)

# Consolidated application static lib:
add_library(${UC_LIB_NAME} STATIC
    ${SRC_GEN})

target_include_directories(${UC_LIB_NAME} PUBLIC
    ${APPLICATION_INCLUDE_DIRS}
    ${INC_GEN_DIR})

string(TOUPPER ${ML_USE_CASE} ML_USE_CASE_UPPER)
target_compile_definitions(${UC_LIB_NAME} PUBLIC
    "ACTIVATION_BUF_SZ=${${ML_USE_CASE_UPPER}_ACTIVATION_BUF_SZ}")

target_link_libraries(${UC_LIB_NAME} PUBLIC
    $<$<BOOL:${ETHOS_U_NPU_ENABLED}>:ethos_u_npu>
    tensorflow-lite-micro
    common_api
)
