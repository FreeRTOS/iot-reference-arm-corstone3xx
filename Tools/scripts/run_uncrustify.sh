#!/bin/bash

# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

find Projects Config -iname "*.[hc]" -exec uncrustify --check -c Tools/uncrustify.cfg {} +
