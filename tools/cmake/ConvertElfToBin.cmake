# Copyright 2021-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

function(target_elf_to_bin target output_binary_name)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        find_program(objcopy NAMES arm-none-eabi-objcopy objcopy REQUIRED)
        set(elf_to_bin ${objcopy}
            -O binary
            $<TARGET_FILE:${target}>
            $<TARGET_FILE_DIR:${target}>/${output_binary_name}.bin
        )
    elseif(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
        find_program(fromelf NAMES fromelf REQUIRED)
        set(elf_to_bin ${fromelf}
            --bin
            --output $<TARGET_FILE_DIR:${target}>/${output_binary_name}.bin
            $<TARGET_FILE:${target}>
            --bincombined
        )
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

# This function is used to extract sections in binaries from input AXF file.
# It is making use of CMake optional arguments feature, the reason why this feature
# is used is that in case of using Arm GNU toolchain, it is up for each application to decide
# which sections are to be extracted using `SECTIONS_NAMES` variable, and the output binary name
# to be generated after sections are eliminated. However, in case of using Arm Compiler For Embedded (ArmClang),
# all the image's code sections are extracted automatically in binary files using `fromelf` tool.
function(extract_sections_from_axf target)
    set(multiValueArgs SECTIONS_NAMES)
    set(oneValueArgs OUTPUT_BIN_NAME)
    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SECTORS_BIN_DIR ${CMAKE_BINARY_DIR}/application_sectors CACHE INTERNAL "Output sectors binaries directory")
    file(MAKE_DIRECTORY ${SECTORS_BIN_DIR})

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        find_program(objcopy NAMES arm-none-eabi-objcopy objcopy REQUIRED)
        list(LENGTH PARSED_SECTIONS_NAMES N_SECTIONS)
        math(EXPR MAX_IDX "${N_SECTIONS} - 1")

        foreach(IDX RANGE ${MAX_IDX})
            list(GET PARSED_SECTIONS_NAMES ${IDX} SECTION_NAME)

            add_custom_command(
                TARGET
                    ${target}
                POST_BUILD
                DEPENDS
                    $<TARGET_FILE:${target}>
                COMMAND
                    ${objcopy} -O binary
                    --only-section ${SECTION_NAME}
                    $<TARGET_FILE:${target}>
                    ${SECTORS_BIN_DIR}/${SECTION_NAME}
                COMMAND
                    ${objcopy} -O binary
                    --remove-section ${SECTION_NAME}
                    $<TARGET_FILE:${target}>
                    ${SECTORS_BIN_DIR}/${PARSED_OUTPUT_BIN_NAME}.bin
            )
        endforeach()

    elseif(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
        find_program(fromelf NAMES fromelf REQUIRED)
        add_custom_command(
            TARGET
                ${target}
            DEPENDS
                $<TARGET_FILE:${target}>
            POST_BUILD
            COMMAND
            ${fromelf} --bin
            --output=${SECTORS_BIN_DIR}/
            $<TARGET_FILE:${target}>
        )
    endif()

endfunction()
