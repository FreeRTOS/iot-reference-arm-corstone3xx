# Copyright 2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

macro(set_compiler_and_linker_flags)
    # Clear toolchain options for all languages similar to IOTSDK as FRI uses
    # different initialization options (such as for optimization and debug symbols)
    # These variables only hold -O, -g and -DNDEBUG options originally
    set(CMAKE_ASM_FLAGS_DEBUG "-O1 -g" CACHE STRING "" FORCE)
    set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-O1 -g" CACHE STRING "" FORCE)
    set(CMAKE_ASM_FLAGS_RELEASE "-O1" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG "-O1 -g" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O1 -g" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "-O1" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "-O1 -g" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O1 -g" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O1" CACHE STRING "" FORCE)

    # Customization of TF-M NS toolchain provided default options
    # TF-M options that are never added because the TFM_DEBUG_SYMBOLS and
    # CONFIG_TFM_WARNINGS_ARE_ERRORS variables are not defined during NS build:
    #     $<$<AND:$<COMPILE_LANGUAGE:C>,$<BOOL:${TFM_DEBUG_SYMBOLS}>>:-g>
    #     $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${TFM_DEBUG_SYMBOLS}>>:-g>
    #     $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<BOOL:${CONFIG_TFM_WARNINGS_ARE_ERRORS}>>:-Werror>
    get_property(compile_options DIRECTORY PROPERTY COMPILE_OPTIONS)
    # TensorFlow Lite Micro is built with the toolchain default -f[no-]short-enums
    # and -f[no-]short-wchar options
    list(REMOVE_ITEM compile_options $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-fshort-enums>)
    list(REMOVE_ITEM compile_options $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-fshort-wchar>)
    # This option causes error when linking with TensorFlow Lite Micro
    list(REMOVE_ITEM compile_options $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-nostdlib>)
    # These options are used to align with previously used Open IOT SDK toolchain flags
    list(APPEND compile_options $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
    list(APPEND compile_options $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
    list(APPEND compile_options -mthumb)
    list(REMOVE_ITEM compile_options $<$<COMPILE_LANGUAGE:ASM>:-masm=armasm>)
    list(APPEND compile_options $<$<COMPILE_LANGUAGE:ASM>:-masm=auto>)
    list(APPEND compile_options $<$<COMPILE_LANGUAGE:ASM>:--target=arm-arm-none-eabi>)
    set_property(DIRECTORY PROPERTY COMPILE_OPTIONS ${compile_options})

endmacro()
