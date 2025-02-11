# Copyright 2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${ROOT}/cmake/TfmInitialCacheCommon.cmake)

set(TFM_MBEDCRYPTO_CONFIG_CLIENT_PATH "${ROOT}/applications/keyword_detection/configs/mbedtls_config/aws_mbedtls_config.h" CACHE FILEPATH "TFM_MBEDCRYPTO_CONFIG_CLIENT_PATH" FORCE)
set(PROJECT_CONFIG_HEADER_FILE ${ROOT}/applications/keyword_detection/configs/tfm_config/project_config.h CACHE FILEPATH "PROJECT_CONFIG_HEADER_FILE" FORCE)
set(MCUBOOT_UPGRADE_STRATEGY "SWAP_USING_SCRATCH" CACHE STRING "MCUBOOT_UPGRADE_STRATEGY" FORCE)
set(MCUBOOT_IMAGE_VERSION_NS  "0.0.1+10" CACHE STRING "MCUBOOT_IMAGE_VERSION_NS" FORCE)
set(CONFIG_TFM_HALT_ON_CORE_PANIC ON CACHE BOOL "CONFIG_TFM_HALT_ON_CORE_PANIC" FORCE)
set(MCUBOOT_DATA_SHARING ON CACHE BOOL "MCUBOOT_DATA_SHARING" FORCE)
set(PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT ON CACHE BOOL "PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT" FORCE)
set(TFM_PARTITION_FIRMWARE_UPDATE ON CACHE BOOL "TFM_PARTITION_FIRMWARE_UPDATE" FORCE)
set(TFM_PARTITION_LOG_LEVEL TFM_PARTITION_LOG_LEVEL_INFO CACHE STRING "TFM_PARTITION_LOG_LEVEL" FORCE)
set(CONFIG_TFM_ENABLE_MVE ON CACHE STRING "CONFIG_TFM_ENABLE_MVE" FORCE)
set(CONFIG_TFM_ENABLE_MVE_FP ON CACHE STRING "CONFIG_TFM_ENABLE_MVE_FP" FORCE)

# These variables are only defined in case of GNU toolchain as it is currently the only toolchain
# that supports the ML Model component OTA update feature where these variables are needed.
if (${TOOLCHAIN} STREQUAL "GNU")
    set(MCUBOOT_IMAGE_VERSION_NS_ML_MODEL "0.0.1+11" CACHE STRING "MCUBOOT_IMAGE_VERSION_NS_ML_MODEL" FORCE)
    set(MCUBOOT_NS_ML_MODEL_IMAGE_FLASH_AREA_NUM "1_0" CACHE STRING "MCUBOOT_NS_ML_MODEL_IMAGE_FLASH_AREA_NUM" FORCE)
    set(MCUBOOT_SECURITY_COUNTER_NS_ML_MODEL 1 CACHE STRING "MCUBOOT_SECURITY_COUNTER_NS_ML_MODEL" FORCE)
    set(MCUBOOT_IMAGE_NUMBER 3 CACHE STRING "MCUBOOT_IMAGE_NUMBER" FORCE)
    set(DEFAULT_MCUBOOT_FLASH_MAP OFF CACHE BOOL "DEFAULT_MCUBOOT_FLASH_MAP" FORCE)
else()
    set(DEFAULT_MCUBOOT_FLASH_MAP ON CACHE BOOL "DEFAULT_MCUBOOT_FLASH_MAP" FORCE)
    set(MCUBOOT_IMAGE_NUMBER 2 CACHE STRING "MCUBOOT_IMAGE_NUMBER" FORCE)
endif()
