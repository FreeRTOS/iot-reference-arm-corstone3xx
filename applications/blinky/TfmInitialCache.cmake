# Copyright 2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

include(${ROOT}/cmake/TfmInitialCacheCommon.cmake)

set(MCUBOOT_IMAGE_VERSION_NS 0.0.1 CACHE STRING "MCUBOOT_IMAGE_VERSION_NS" FORCE)
set(MCUBOOT_IMAGE_NUMBER 2 CACHE STRING "MCUBOOT_IMAGE_NUMBER" FORCE)
set(DEFAULT_MCUBOOT_FLASH_MAP ON CACHE STRING "DEFAULT_MCUBOOT_FLASH_MAP" FORCE)
set(PROJECT_CONFIG_HEADER_FILE ${ROOT}/applications/blinky/configs/tfm_config/project_config.h CACHE FILEPATH "PROJECT_CONFIG_HEADER_FILE" FORCE)
