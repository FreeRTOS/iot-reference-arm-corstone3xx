#!/bin/bash

# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

git submodule deinit --all -f
find . -iname "*.[hc]" -exec uncrustify --replace --no-backup --if-changed -c Tools/uncrustify.cfg -l C {} +
git submodule update --init --recursive
