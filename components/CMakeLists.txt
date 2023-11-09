# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# FreeRTOS must be first as other components depend on it
add_subdirectory(freertos_kernel)
add_subdirectory(ai EXCLUDE_FROM_ALL)
add_subdirectory(aws_iot EXCLUDE_FROM_ALL)
add_subdirectory(connectivity EXCLUDE_FROM_ALL)
add_subdirectory(security EXCLUDE_FROM_ALL)
add_subdirectory(tools EXCLUDE_FROM_ALL)
