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

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/patches/0001-corstone300-Add-missing-FVP-VSI-handlers.patch
    WORKING_DIRECTORY ${arm_corstone_platform_bsp_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/patches/0002-platform-irq-Add-VSI-IRQ-numbers-for-CS300-and-CS310.patch
    WORKING_DIRECTORY ${arm_corstone_platform_bsp_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

execute_process(COMMAND git am --abort
    COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/patches/0003-bsp-Add-CMSIS-Driver-implementation-for-VSI.patch
    WORKING_DIRECTORY ${arm_corstone_platform_bsp_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
)

add_subdirectory(library)

target_compile_definitions(arm-corstone-platform-bsp
    INTERFACE
        __DOMAIN_NS=1
)

if(ARM_CORSTONE_BSP_TARGET_PLATFORM STREQUAL "corstone300")
    set(BL2_IMAGE_LOAD_ADDRESS 0x00000000 CACHE STRING "Bootload image loading address")
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 CACHE STRING "Secure TF-M firmware loading address")
    set(NS_IMAGE_LOAD_ADDRESS 0x28040000 CACHE STRING "Non-secure user application loading address")
    set(S_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x10022000 CACHE STRING "Secure provisioning bundle loading address")
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x211FF000 CACHE STRING "Non-Secure provisioning bundle loading address")
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
    set(BL2_IMAGE_LOAD_ADDRESS 0x11000000 CACHE STRING "Bootload image loading address")
    set(S_IMAGE_LOAD_ADDRESS 0x38000000 CACHE STRING "Secure TF-M firmware loading address")
    set(NS_IMAGE_LOAD_ADDRESS 0x28040000 CACHE STRING "Non-secure user application loading address")
    set(S_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x11022000 CACHE STRING "Secure provisioning bundle loading address")
    set(NS_PROVISIONING_BUNDLE_LOAD_ADDRESS 0x213FF000 CACHE STRING "Non-Secure provisioning bundle loading address")
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
endif()
set(ETHOS_U_NPU_NUM_MACS ${DEFAULT_ML_MAC_${ETHOS_U_NPU_ID}} CACHE STRING "Number of 8x8 MACs performed per cycle by the NPU")

build_ethosu_target_npu_config_values(${ETHOS_U_NPU_ID} ${ETHOS_U_NPU_NUM_MACS})

if(NOT ETHOSU_TARGET_NPU_CONFIG IN_LIST VALID_ETHOSU_TARGET_NPU_CONFIGS)
    message(FATAL_ERROR "Invalid ETHOSU target configuration '${ETHOSU_TARGET_NPU_CONFIG}' for '${ARM_CORSTONE_BSP_TARGET_PLATFORM}'. "
                        "Choose from: ${VALID_ETHOSU_TARGET_NPU_CONFIGS}")
endif()

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

# BSP serial library

add_library(fri-bsp STATIC)

target_sources(fri-bsp
    PRIVATE
        common/bsp_serial.c
        common/mps3_leds.c
)

target_include_directories(fri-bsp
    PUBLIC
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone300>:${CMAKE_CURRENT_LIST_DIR}/corstone300/include>
        $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone310>:${CMAKE_CURRENT_LIST_DIR}/corstone310/include>
        common
)

target_link_libraries(fri-bsp
    PUBLIC
        arm-corstone-platform-bsp
        freertos_kernel
)
