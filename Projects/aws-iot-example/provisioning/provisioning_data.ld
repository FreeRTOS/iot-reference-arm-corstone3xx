;/* Copyright 2023 Arm Limited and/or its affiliates
; * <open-source-office@arm.com>
; * SPDX-License-Identifier: MIT
; */


MEMORY
{
    CODE (rx) : ORIGIN = 0x210FF000, LENGTH = 0x1000
}

SECTIONS
{
    RO_DATA :
    {
        *(.rodata.provisioningBundle)
        *(.rodata*)
    } > CODE
}
