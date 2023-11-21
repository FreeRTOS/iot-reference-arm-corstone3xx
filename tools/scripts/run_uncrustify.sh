#!/bin/bash

# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Ensure the new line is used to split the output of the `git submodule status`
# command into an array
IFS=$'\n'

submodules_status=($(git submodule status))

exclude_pattern="-E "
for submodule_status in "${submodules_status[@]}"
do
    # Ensure the space is used to split the submodule status into an array
    IFS=" "
    submodule_status_parts=($submodule_status)
    submodule_path="${submodule_status_parts[1]}"
    exclude_pattern+="${submodule_path} -E "
done
exclude_pattern+="./build"

fdfind -E $exclude_pattern -e c -e h --exec uncrustify --no-backup --replace --if-changed -c tools/uncrustify.cfg
