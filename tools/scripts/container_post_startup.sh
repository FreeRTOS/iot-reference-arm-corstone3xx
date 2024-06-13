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

# Install the pre-commit hooks.
rm -rf /tmp/build
pip install . -t /tmp/build
sudo cp /tmp/build/bin/banned-api-hook /usr/local/bin/banned-api-hook
rm -rf iot_reference_arm_corstone3xx.egg-info
pre-commit install

# Return success
exit 0;
