# Copyright 2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

list(APPEND CMAKE_MODULE_PATH ${CONFIG_SPE_PATH}/cmake)

# A platform specific MCPU and architecture flags for NS side
include(${CONFIG_SPE_PATH}/platform/cpuarch.cmake)
# Include common configs exported from TF-M
include(${CONFIG_SPE_PATH}/cmake/spe_config.cmake)

if(NOT DEFINED TFM_TOOLCHAIN_FILE)
    if(${TOOLCHAIN} STREQUAL "GNU")
        set(TFM_TOOLCHAIN_FILE ${CONFIG_SPE_PATH}/cmake/toolchain_ns_GNUARM.cmake)
    elseif(${TOOLCHAIN} STREQUAL "ARMCLANG")
        set(TFM_TOOLCHAIN_FILE ${CONFIG_SPE_PATH}/cmake/toolchain_ns_ARMCLANG.cmake)
    endif()
endif()

if(${TOOLCHAIN} STREQUAL "GNU")
    include(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/cmake/CompilerFlagsGNUARM.cmake)
elseif(${TOOLCHAIN} STREQUAL "ARMCLANG")
    include(${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/cmake/CompilerFlagsARMCLANG.cmake)
else()
    message(FATAL_ERROR "Unsupported toolchain: ${TOOLCHAIN}")
endif()

include(${TFM_TOOLCHAIN_FILE})
