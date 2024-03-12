# Copyright 2023-2024, Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0

# Set the linker script for the target specified
macro(set_linker_script executable_target)
    if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        target_link_options(${executable_target}
            PRIVATE
                $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone300>:-T ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone300/an552_ns.ld>
                $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone310>:-T ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone310/an555_ns.ld>
                $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone315>:-T ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone315/corstone_315_ns.ld>
                -Wl,--gc-sections,-Map=${executable_target}.map
        )
    else()
        target_link_options(${executable_target}
            PRIVATE
                $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone300>:--scatter=${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone300/an552_ns.sct>
                $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone310>:--scatter=${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone310/an555_ns.sct>
                $<$<STREQUAL:${ARM_CORSTONE_BSP_TARGET_PLATFORM},corstone315>:--scatter=${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/bsp/corstone315/corstone_315_ns.sct>
                --map
        )
    endif()
endmacro(set_linker_script executable_target)
