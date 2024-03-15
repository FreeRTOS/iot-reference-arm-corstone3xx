# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0

function(build_ethosu_target_npu_config_values id num_macs)
    if(id STREQUAL "U55")
        set(ETHOS_U_NPU_CONFIG_ID "H${num_macs}" CACHE STRING "Specifies the configuration ID for the NPU." FORCE)
        set(ETHOS_U_NPU_MEMORY_MODE Shared_Sram CACHE STRING "Specifies the memory mode used in the Vela command." FORCE)
    elseif(id STREQUAL "U65")
        set(ETHOS_U_NPU_CONFIG_ID "Y${num_macs}" CACHE STRING "Specifies the configuration ID for the NPU." FORCE)
        set(ETHOS_U_NPU_MEMORY_MODE Dedicated_Sram CACHE STRING "Specifies the memory mode used in the Vela command." FORCE)
        set(ETHOS_U_NPU_CACHE_SIZE  "393216" CACHE STRING "Arm Ethos-U65 NPU Cache Size" FORCE)
    endif()
    string(TOLOWER "${id}" id)
    set(ETHOSU_TARGET_NPU_CONFIG "ethos-${id}-${num_macs}" CACHE STRING "Default NPU configuration" FORCE)
endfunction()

set(ARM_CORSTONE_BSP_TARGET_PLATFORM "corstone310" CACHE STRING "Featured Reference Integration target")

set(arm_corstone_platform_bsp_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to Arm Corstone-3xx Platform CMSIS-Driver Based Board Support Package source code"
)

include(ApplyPatches)

set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/patches")
set(PATCH_FILES
    "${PATCH_FILES_DIRECTORY}/0001-bsp-Add-CMSIS-Driver-implementation-for-VSI.patch"
)
iot_reference_arm_corstone3xx_apply_patches("${arm_corstone_platform_bsp_SOURCE_DIR}" "${PATCH_FILES}")

add_subdirectory(library)

target_compile_definitions(arm-corstone-platform-bsp
    INTERFACE
        __DOMAIN_NS=1
)

if(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone300")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/corstone300/fvp" CACHE STRING "TFM Platform local path")
    set(TFM_FLASH_S_PARTITION_SIZE "0x40000")
    set(TFM_FLASH_NS_PARTITION_SIZE "0x340000")

    set(BL2_IMAGE_LOAD_ADDRESS 0x00000000 CACHE STRING "Bootload image loading address")
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 CACHE STRING "Secure TF-M firmware loading address")
    set(NS_IMAGE_LOAD_ADDRESS 0x28040000 CACHE STRING "Non-secure user application loading address")
    set(S_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x10022000 CACHE STRING "Secure provisioning bundle loading address")
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x211FF000 CACHE STRING "Non-Secure provisioning bundle loading address")
    set(NS_DDR4_IMAGE_LOAD_ADDRESS 0x60000000 CACHE STRING "Non-Secure Double Data Rate RAM image loading address")

    set(ETHOS_U_BASE_ADDR "0x48102000" CACHE STRING "Ethos-U NPU base address" FORCE)
    set(ETHOS_U_IRQN "56" CACHE STRING "Ethos-U NPU Interrupt" FORCE)
    set(ETHOS_U_SEC_ENABLED "0" CACHE STRING "Ethos-U NPU Security enable" FORCE)
    set(ETHOS_U_PRIV_ENABLED "0" CACHE STRING "Ethos-U NPU Privilege enable" FORCE)
    set(ETHOS_U_NPU_ID "U55" CACHE STRING "Arm Ethos-U NPU IP (U55 or U65)")
    set(DEFAULT_ML_MAC_U55 "128")
    set(DEFAULT_ML_MAC_U65 "256")
    target_compile_definitions(arm-corstone-platform-bsp PUBLIC CORSTONE300_FVP)
    set(VALID_ETHOSU_TARGET_NPU_CONFIGS
        "ethos-u55-32"
        "ethos-u55-64"
        "ethos-u55-128"
        "ethos-u55-256"
        "ethos-u65-256"
        "ethos-u65-512"
    )
elseif(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone310")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps3/corstone310/fvp" CACHE STRING "TFM Platform local path")
    set(TFM_FLASH_S_PARTITION_SIZE "0x40000")
    set(TFM_FLASH_NS_PARTITION_SIZE "0x340000")

    set(BL2_IMAGE_LOAD_ADDRESS 0x11000000 CACHE STRING "Bootload image loading address")
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 CACHE STRING "Secure TF-M firmware loading address")
    set(NS_IMAGE_LOAD_ADDRESS 0x28040000 CACHE STRING "Non-secure user application loading address")
    set(S_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x11022000 CACHE STRING "Secure provisioning bundle loading address")
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x213FF000 CACHE STRING "Non-Secure provisioning bundle loading address")
    set(NS_DDR4_IMAGE_LOAD_ADDRESS 0x60000000 CACHE STRING "Non-Secure Double Data Rate RAM image loading address")

    set(ETHOS_U_BASE_ADDR "0x40004000" CACHE STRING "Ethos-U NPU base address" FORCE)
    set(ETHOS_U_IRQN "16" CACHE STRING "Ethos-U NPU Interrupt" FORCE)
    set(ETHOS_U_SEC_ENABLED "0" CACHE STRING "Ethos-U NPU Security enable" FORCE)
    set(ETHOS_U_PRIV_ENABLED "0" CACHE STRING "Ethos-U NPU Privilege enable" FORCE)
    set(ETHOS_U_NPU_ID "U55" CACHE STRING "Arm Ethos-U NPU IP (U55 or U65)")
    set(DEFAULT_ML_MAC_U55 "256")
    set(DEFAULT_ML_MAC_U65 "256")
    target_compile_definitions(arm-corstone-platform-bsp PUBLIC CORSTONE310_FVP)
    set(VALID_ETHOSU_TARGET_NPU_CONFIGS
        "ethos-u55-32"
        "ethos-u55-64"
        "ethos-u55-128"
        "ethos-u55-256"
        "ethos-u65-256"
        "ethos-u65-512"
    )
elseif(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone315")
    set(TFM_PLATFORM_LOCAL_PATH "arm/mps4/corstone315" CACHE STRING "TFM Platform local path")
    set(TFM_FLASH_S_PARTITION_SIZE "0x40000")
    set(TFM_FLASH_NS_PARTITION_SIZE "0x340000")

    set(BL1_IMAGE_LOAD_ADDRESS 0x11000000 CACHE STRING "Bootload stage 1 image loading address")
    set(BL2_IMAGE_LOAD_ADDRESS 0x12031400 CACHE STRING "Bootload image loading address")
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 CACHE STRING "Secure TF-M firmware loading address")
    set(NS_IMAGE_LOAD_ADDRESS 0x28040000 CACHE STRING "Non-secure user application loading address")
    set(S_CM_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x12024000 CACHE STRING "Secure CM provisioning bundle loading address")
    set(S_DM_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x1202aa00 CACHE STRING "Secure DM provisioning bundle loading address")
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x213FF000 CACHE STRING "Non-Secure provisioning bundle loading address")
    set(NS_DDR4_IMAGE_LOAD_ADDRESS 0x60000000 CACHE STRING "Non-Secure Double Data Rate RAM image loading address")

    set(ETHOS_U_BASE_ADDR "0x40004000" CACHE STRING "Ethos-U NPU base address" FORCE)
    set(ETHOS_U_IRQN "16" CACHE STRING "Ethos-U NPU Interrupt" FORCE)
    set(ETHOS_U_SEC_ENABLED "0" CACHE STRING "Ethos-U NPU Security enable" FORCE)
    set(ETHOS_U_PRIV_ENABLED "0" CACHE STRING "Ethos-U NPU Privilege enable" FORCE)
    set(ETHOS_U_NPU_ID "U65" CACHE STRING "Arm Ethos-U NPU IP (U55 or U65)")
    set(DEFAULT_ML_MAC_U65 "256")
    set(VALID_ETHOSU_TARGET_NPU_CONFIGS
        "ethos-u65-256"
        "ethos-u65-512"
    )

    list(APPEND TFM_CMAKE_APP_ARGS
        -DTFM_BL1_LOGGING=ON
    )

    set(ISP_MVE_FP ON CACHE BOOL "MVE FP is used by ISP driver" FORCE)

    add_subdirectory(isp_mali-c55)
else()
    message(FATAL_ERROR "Invalid ARM_CORSTONE_BSP_TARGET_PLATFORM (${ARM_CORSTONE_BSP_TARGET_PLATFORM}) set. Supported are corstone300/corstone310")
endif()
set(ETHOS_U_NPU_NUM_MACS ${DEFAULT_ML_MAC_${ETHOS_U_NPU_ID}} CACHE STRING "Number of 8x8 MACs performed per cycle by the NPU")

build_ethosu_target_npu_config_values(${ETHOS_U_NPU_ID} ${ETHOS_U_NPU_NUM_MACS})

if(NOT ETHOSU_TARGET_NPU_CONFIG IN_LIST VALID_ETHOSU_TARGET_NPU_CONFIGS)
    message(FATAL_ERROR "Invalid ETHOSU target configuration '${ETHOSU_TARGET_NPU_CONFIG}' for '${ARM_CORSTONE_BSP_TARGET_PLATFORM}'. "
                        "Choose from: ${VALID_ETHOSU_TARGET_NPU_CONFIGS}")
endif()

set(ARM_CORSTONE_BSP_TARGET_PLATFORM_TFM_CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCONFIG_TFM_ENABLE_CP10CP11=ON
    -DMCUBOOT_GENERATE_SIGNING_KEYPAIR=ON
    -DMCUBOOT_LOG_LEVEL=INFO
    -DMCUBOOT_SIGNATURE_KEY_LEN=3072
    -DNS=ON
    -DPLATFORM_DEFAULT_PROVISIONING=OFF
    -DPLATFORM_DEFAULT_UART_STDOUT=ON
    -DTFM_DUMMY_PROVISIONING=OFF
    -DTFM_EXCEPTION_INFO_DUMP=ON
    -DTFM_PARTITION_CRYPTO=ON
    -DTFM_PARTITION_INITIAL_ATTESTATION=ON
    -DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=ON
    -DTFM_PARTITION_PLATFORM=ON
    -DTFM_PARTITION_PROTECTED_STORAGE=ON
    -DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_INFO
    -DTFM_PLATFORM=${TFM_PLATFORM_LOCAL_PATH}
    -DFLASH_S_PARTITION_SIZE=${TFM_FLASH_S_PARTITION_SIZE}
    -DFLASH_NS_PARTITION_SIZE=${TFM_FLASH_NS_PARTITION_SIZE}
    ${TFM_CMAKE_APP_ARGS}
    CACHE STRING "TFM CMake arguments" FORCE
)

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    # Execute the command as CMAKE_C_COMPILER_VERSION is not guaranteed to be defined.
    EXECUTE_PROCESS( COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE ARM_GNU_TOOLCHAIN_VERSION )
endif()

# The `toolchain-override` is an INTERFACE library
# as it would not contain any source file in case
# of the Arm Compiler For Embedded
add_library(toolchain-override INTERFACE)

target_sources(toolchain-override
    PUBLIC
        $<$<VERSION_GREATER_EQUAL:${ARM_GNU_TOOLCHAIN_VERSION},11.3.1>:${CMAKE_CURRENT_LIST_DIR}/common/syscalls_stub.c>
)

target_include_directories(arm-corstone-platform-bsp
    PUBLIC
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone300>:${CMAKE_CURRENT_LIST_DIR}/corstone300/include>
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone310>:${CMAKE_CURRENT_LIST_DIR}/corstone310/include>
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone315>:${CMAKE_CURRENT_LIST_DIR}/corstone315/include>
)

# BSP serial library

add_library(fri-bsp STATIC)

target_sources(fri-bsp
    PRIVATE
        common/bsp_serial.c
        common/mps3_leds.c
)

target_include_directories(fri-bsp
    PUBLIC
        common
)

target_link_libraries(fri-bsp
    PUBLIC
        arm-corstone-platform-bsp
        freertos_kernel
)
