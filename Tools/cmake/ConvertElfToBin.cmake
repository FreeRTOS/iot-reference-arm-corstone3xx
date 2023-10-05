# Copyright 2021-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

function(target_elf_to_bin target output_binary_name)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(elf_to_bin arm-none-eabi-objcopy -O binary $<TARGET_FILE:${target}> $<TARGET_FILE_DIR:${target}>/${output_binary_name}.bin)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
        set(elf_to_bin fromelf --bin --output $<TARGET_FILE_DIR:${target}>/${output_binary_name}.bin $<TARGET_FILE:${target}>  --bincombined)
    endif()

    add_custom_command(
        TARGET
            ${target}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE:${target}>
        COMMAND
            ${elf_to_bin}
        COMMAND
            ${CMAKE_COMMAND} -E echo "-- built: $<TARGET_FILE_DIR:${target}>/${output_binary_name}.bin"
        VERBATIM
    )
endfunction()
