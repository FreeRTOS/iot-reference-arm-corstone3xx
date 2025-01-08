# Copyright 2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# TF-M patches until long term solution
set(trusted_firmware-m_SOURCE_DIR ${CMAKE_SOURCE_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/../tools/cmake/ApplyPatches.cmake)

set(PATCH_FILES_DIRECTORY "${CMAKE_SOURCE_DIR}/../integration/patches")
set(PATCH_FILES
    "${PATCH_FILES_DIRECTORY}/0004-build-Enable-armclang-MVE.patch"
)
# These patches are only applied in case of building keyword_detection application with GNU toolchain
# as it is currently the only application that utilises the ML Model component OTA update feature
# where these patches are needed.
if((${EXAMPLE} STREQUAL "keyword-detection") AND (${TOOLCHAIN} STREQUAL "GNU"))
    list(APPEND PATCH_FILES
        "${PATCH_FILES_DIRECTORY}/0001-corstone300-Add-ML-model-component.patch"
        "${PATCH_FILES_DIRECTORY}/0002-corstone310-Add-ML-model-component.patch"
        "${PATCH_FILES_DIRECTORY}/0003-mps4-Add-ML-model-component.patch"
    )
endif()
iot_reference_arm_corstone3xx_apply_patches("${trusted_firmware-m_SOURCE_DIR}" "${PATCH_FILES}")


if(TARGET_NAME STREQUAL "corstone300")
    set(TFM_PLATFORM "arm/mps3/corstone300/fvp" CACHE STRING "TFM Platform local path" FORCE)
    set(FLASH_S_PARTITION_SIZE "0X40000" CACHE STRING "FLASH_S_PARTITION_SIZE" FORCE)

    # These variables are only defined in case of building keyword_detection application with GNU toolchain
    # as it is currently the only application that utilises the ML Model component OTA update feature
    # where these variables are needed.
    if((${EXAMPLE} STREQUAL "keyword-detection") AND (${TOOLCHAIN} STREQUAL "GNU"))
        set(FLASH_NS_PARTITION_SIZE "0X240000" CACHE STRING "FLASH_NS_PARTITION_SIZE" FORCE)
        set(FLASH_NS_ML_MODEL_PARTITION_SIZE "0X100000" CACHE STRING "FLASH_NS_ML_MODEL_PARTITION_SIZE" FORCE)
    else()
        set(FLASH_NS_PARTITION_SIZE "0X340000" CACHE STRING "FLASH_NS_PARTITION_SIZE" FORCE)
    endif()

elseif(TARGET_NAME STREQUAL "corstone310")
    set(TFM_PLATFORM "arm/mps3/corstone310/fvp" CACHE STRING "TFM Platform local path" FORCE)
    set(FLASH_S_PARTITION_SIZE "0X40000" CACHE STRING "FLASH_S_PARTITION_SIZE" FORCE)

    # These variables are only defined in case of building keyword_detection application with GNU toolchain
    # as it is currently the only application that utilises the ML Model component OTA update feature
    # where these variables are needed.
    if((${EXAMPLE} STREQUAL "keyword-detection") AND (${TOOLCHAIN} STREQUAL "GNU"))
        set(FLASH_NS_PARTITION_SIZE "0X240000" CACHE STRING "FLASH_NS_PARTITION_SIZE" FORCE)
        set(FLASH_NS_ML_MODEL_PARTITION_SIZE "0X100000" CACHE STRING "FLASH_NS_ML_MODEL_PARTITION_SIZE" FORCE)
    else()
        set(FLASH_NS_PARTITION_SIZE "0X340000" CACHE STRING "FLASH_NS_PARTITION_SIZE" FORCE)
    endif()

elseif(TARGET_NAME STREQUAL "corstone315")
    set(TFM_PLATFORM "arm/mps4/corstone315" CACHE STRING "TFM Platform local path" FORCE)
    set(FLASH_S_PARTITION_SIZE "0X40000" CACHE STRING "FLASH_S_PARTITION_SIZE" FORCE)
    set(TFM_BL1_LOGGING ON CACHE BOOL "TFM_BL1_LOGGING" FORCE)

    # These variables are only defined in case of building keyword_detection application with GNU toolchain
    # as it is currently the only application that utilises the ML Model component OTA update feature
    # where these variables are needed.
    if((${EXAMPLE} STREQUAL "keyword-detection") AND (${TOOLCHAIN} STREQUAL "GNU"))
        set(FLASH_NS_PARTITION_SIZE "0X240000" CACHE STRING "FLASH_NS_PARTITION_SIZE" FORCE)
        set(FLASH_NS_ML_MODEL_PARTITION_SIZE "0X100000" CACHE STRING "FLASH_NS_ML_MODEL_PARTITION_SIZE" FORCE)
    else()
        set(FLASH_NS_PARTITION_SIZE "0X340000" CACHE STRING "FLASH_NS_PARTITION_SIZE" FORCE)
    endif()

elseif(TARGET_NAME STREQUAL "corstone320")
    set(TFM_PLATFORM "arm/mps4/corstone320" CACHE STRING "TFM Platform local path" FORCE)
    set(FLASH_S_PARTITION_SIZE "0X40000" CACHE STRING "FLASH_S_PARTITION_SIZE" FORCE)
    set(TFM_BL1_LOGGING ON CACHE BOOL "TFM_BL1_LOGGING" FORCE)

    # These variables are only defined in case of building keyword_detection application with GNU toolchain
    # as it is currently the only application that utilises the ML Model component OTA update feature
    # where these variables are needed.
    if((${EXAMPLE} STREQUAL "keyword-detection") AND (${TOOLCHAIN} STREQUAL "GNU"))
        set(FLASH_NS_PARTITION_SIZE "0X240000" CACHE STRING "FLASH_NS_PARTITION_SIZE" FORCE)
        set(FLASH_NS_ML_MODEL_PARTITION_SIZE "0X100000" CACHE STRING "FLASH_NS_ML_MODEL_PARTITION_SIZE" FORCE)
    else()
        set(FLASH_NS_PARTITION_SIZE "0X340000" CACHE STRING "FLASH_NS_PARTITION_SIZE" FORCE)
    endif()

else()
    message(FATAL_ERROR "Invalid TARGET_NAME (${TARGET_NAME}) set. Supported are corstone300/corstone310/corstone315/corstone320")
endif()

set(CONFIG_TFM_ENABLE_FP ON CACHE BOOL "CONFIG_TFM_ENABLE_FP" FORCE)
set(MCUBOOT_SECURITY_COUNTER_NS auto CACHE STRING "MCUBOOT_SECURITY_COUNTER_NS" FORCE)
set(MCUBOOT_CONFIRM_IMAGE ON CACHE BOOL "MCUBOOT_CONFIRM_IMAGE" FORCE)
set(MCUBOOT_SIGNATURE_TYPE "EC-P256" CACHE STRING "Supported algorithms for signature validation [RSA-2048, RSA-3072, EC-P256, EC-P384]" FORCE)
set(TFM_BL1_LOG_LEVEL "LOG_LEVEL_INFO" CACHE STRING "TFM_BL1_LOG_LEVEL")
set(CONFIG_TFM_ENABLE_CP10CP11 ON CACHE BOOL "CONFIG_TFM_ENABLE_CP10CP11" FORCE)
set(MCUBOOT_GENERATE_SIGNING_KEYPAIR ON CACHE BOOL "MCUBOOT_GENERATE_SIGNING_KEYPAIR" FORCE)
set(MCUBOOT_LOG_LEVEL INFO CACHE STRING "MCUBOOT_LOG_LEVEL" FORCE)
set(PLATFORM_DEFAULT_PROVISIONING OFF CACHE BOOL "PLATFORM_DEFAULT_PROVISIONING" FORCE)
set(PLATFORM_DEFAULT_UART_STDOUT ON CACHE BOOL "PLATFORM_DEFAULT_UART_STDOUT" FORCE)
set(TFM_DUMMY_PROVISIONING OFF CACHE BOOL "TFM_DUMMY_PROVISIONING" FORCE)
set(TFM_EXCEPTION_INFO_DUMP ON CACHE BOOL "TFM_EXCEPTION_INFO_DUMP" FORCE)
set(TFM_PARTITION_CRYPTO ON CACHE BOOL "TFM_PARTITION_CRYPTO" FORCE)
set(TFM_PARTITION_INITIAL_ATTESTATION ON CACHE BOOL "TFM_PARTITION_INITIAL_ATTESTATION" FORCE)
set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE ON CACHE BOOL "TFM_PARTITION_INTERNAL_TRUSTED_STORAGE" FORCE)
set(TFM_PARTITION_PLATFORM ON CACHE BOOL "TFM_PARTITION_PROTECTED_STORAGE" FORCE)
set(TFM_PARTITION_PROTECTED_STORAGE ON CACHE BOOL "TFM_PARTITION_PROTECTED_STORAGE" FORCE)
set(TFM_SPM_LOG_LEVEL TFM_SPM_LOG_LEVEL_INFO CACHE STRING "TFM_SPM_LOG_LEVEL" FORCE)

# TF-M can be built with a different toolchain, but the toolchain that
# builds the IoT SDK is guaranteed to be available in the environment.
if(${TOOLCHAIN} STREQUAL "GNU")
    set(TFM_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain_GNUARM.cmake" CACHE STRING "TFM_TOOLCHAIN_FILE" FORCE)
elseif(${TOOLCHAIN} STREQUAL "ARMCLANG")
    set(TFM_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain_ARMCLANG.cmake" CACHE STRING "TFM_TOOLCHAIN_FILE" FORCE)
else()
    message(FATAL_ERROR "Unsupported toolchain: ${TOOLCHAIN}")
endif()
