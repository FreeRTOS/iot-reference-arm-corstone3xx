# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_executable(provisioning_data
    provisioning_data.c
)

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    target_link_options(provisioning_data
        PRIVATE
            "--specs=nosys.specs"
            "-T" "${CMAKE_CURRENT_SOURCE_DIR}/provisioning_data.ld"
            "-nostartfiles"
            "-Wl,-Map=${CMAKE_CURRENT_SOURCE_DIR}/provisioning_data.map"
    )
    set(provisioning_elf_to_bin_cmd ${GCC_ELF2BIN} -O binary ${CMAKE_CURRENT_BINARY_DIR}/provisioning_data${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/provisioning_data.bin)
else()
    target_link_options(provisioning_data
        PRIVATE
            "--map"
            "--scatter=${CMAKE_CURRENT_SOURCE_DIR}/provisioning_data.sct"
    )
    set(provisioning_elf_to_bin_cmd ${ARM_ELF2BIN} --bin --output ${CMAKE_CURRENT_BINARY_DIR}/provisioning_data.bin ${CMAKE_CURRENT_BINARY_DIR}/provisioning_data${CMAKE_EXECUTABLE_SUFFIX} --bincombined)
endif()

add_custom_command(OUTPUT provisioning_data.bin
    DEPENDS $<TARGET_FILE_DIR:provisioning_data>/provisioning_data${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND ${provisioning_elf_to_bin_cmd}
)

add_custom_target(provisioning_data_bin
    SOURCES provisioning_data.bin
)

