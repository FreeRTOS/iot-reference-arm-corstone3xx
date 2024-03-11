#!/bin/bash

# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Change the ownership of /workspace to user `ubuntu`
sudo chown -R ubuntu /workspaces

# Check if the submodules have been initialised
git submodule status | grep -E "\-[0-f]{10}"
ERROR_CODE=$?
if [ $ERROR_CODE == 0 ]; then
    git submodule update --init --recursive
fi

# Return success
exit 0;
