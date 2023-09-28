# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

macro(target_add_scatter_file target)
    add_library(${target}_scatter OBJECT)

    target_link_libraries(${target}_scatter PRIVATE
        fri-bsp
    )

    if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        target_link_options(${target}
            PRIVATE
            -T $<TARGET_OBJECTS:${target}_scatter>
        )

        foreach(scatter_file ${ARGN})
            target_sources(${target}_scatter
                PRIVATE
                    ${scatter_file}
            )
            # Cmake cannot use generator expressions in the
            # set_source_file_properties command, so instead we just parse the regex
            # for the filename and set the property on all files, regardless of if
            # the generator expression would evaluate to true or not.
            string(REGEX REPLACE ".*>:(.*)>$" "\\1" SCATTER_FILE_PATH "${scatter_file}")
            set_source_files_properties(${SCATTER_FILE_PATH}
                PROPERTIES
                LANGUAGE C
                KEEP_EXTENSION True # Don't use .o extension for the preprocessed file
            )
        endforeach()

        add_dependencies(${target}
            ${target}_scatter
        )

        set_target_properties(${target} PROPERTIES LINK_DEPENDS $<TARGET_OBJECTS:${target}_scatter>)

        target_compile_options(${target}_scatter
            PRIVATE
                -E
                -P
                -xc
        )

    else()
        target_link_options(${target}
            PRIVATE
            --scatter=$<TARGET_OBJECTS:${target}_scatter>
        )

        foreach(scatter_file ${ARGN})
            target_sources(${target}_scatter
                PRIVATE
                    ${scatter_file}
            )
            # Cmake cannot use generator expressions in the
            # set_source_file_properties command, so instead we just parse the regex
            # for the filename and set the property on all files, regardless of if
            # the generator expression would evaluate to true or not.
            string(REGEX REPLACE ".*>:(.*)>$" "\\1" SCATTER_FILE_PATH "${scatter_file}")
            set_source_files_properties(${SCATTER_FILE_PATH}
                PROPERTIES
                LANGUAGE C
            )
        endforeach()

        add_dependencies(${target}
            ${target}_scatter
        )

        set_target_properties(${target} PROPERTIES LINK_DEPENDS $<TARGET_OBJECTS:${target}_scatter>)

        target_compile_options(${target}_scatter
            PRIVATE
                -E
                -xc
        )
    endif()
endmacro()
