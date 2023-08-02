;/* Copyright 2023 Arm Limited and/or its affiliates
; * <open-source-office@arm.com>
; * SPDX-License-Identifier: MIT
; */

#include "provisioning_config.h"

MEMORY
{
    CODE (rx) : ORIGIN = PROVISIONING_DATA_START, LENGTH = PROVISIONING_DATA_LEN
}

SECTIONS
{
    RO_DATA :
    {
        *(.rodata.provisioningBundle)
        *(.rodata*)
    } > CODE
}
