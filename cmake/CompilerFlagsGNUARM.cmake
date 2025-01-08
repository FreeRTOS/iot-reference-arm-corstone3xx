# Copyright 2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

macro(set_compiler_and_linker_flags)
    # Clear toolchain options for all languages similar to IOTSDK as FRI uses
    # different initialization options (such as for optimization and debug symbols)
    # These variables only hold -O, -g and -DNDEBUG options originally
    set(CMAKE_ASM_FLAGS_DEBUG "-Og -g3" CACHE STRING "" FORCE)
    set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-Og -g3" CACHE STRING "" FORCE)
    set(CMAKE_ASM_FLAGS_RELEASE "-Og" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG "-Og -g3" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Og -g3" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "-Og" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Og -g3" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "-Og" CACHE STRING "" FORCE)

    # Customization of TF-M NS toolchain provided default options
    # TF-M options that are never added because the TFM_DEBUG_SYMBOLS and
    # CONFIG_TFM_WARNINGS_ARE_ERRORS variables are not defined during NS build:
    #     $<$<AND:$<COMPILE_LANGUAGE:C>,$<BOOL:${TFM_DEBUG_SYMBOLS}>>:-g>
    #     $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${TFM_DEBUG_SYMBOLS}>>:-g>
    #     $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<BOOL:${CONFIG_TFM_WARNINGS_ARE_ERRORS}>>:-Werror>
    get_property(compile_options DIRECTORY PROPERTY COMPILE_OPTIONS)
    list(REMOVE_ITEM compile_options "-specs=nano.specs")
    # TensorFlow Lite Micro is built with the toolchain default -f[no-]short-enums
    # and -f[no-]short-wchar options
    list(REMOVE_ITEM compile_options "-fshort-enums")
    # These options are used to align with previously used Open IOT SDK toolchain flags
    list(APPEND compile_options "-fomit-frame-pointer")
    list(APPEND compile_options $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
    list(APPEND compile_options $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
    set_property(DIRECTORY PROPERTY COMPILE_OPTIONS ${compile_options})

    get_property(link_options DIRECTORY PROPERTY LINK_OPTIONS)
    # These options are used to align with previously used Open IOT SDK toolchain flags
    list(REMOVE_ITEM link_options "-specs=nano.specs")
    set_property(DIRECTORY PROPERTY LINK_OPTIONS ${link_options})

endmacro()
