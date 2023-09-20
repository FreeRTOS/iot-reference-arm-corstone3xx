/*
 * FreeRTOS Kernel V10.4.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

/* From the "Fast Models Reference Guide" (https://developer.arm.com/documentation/100964/1123/About-the-models),
 * "Programmer's View (PV) models of processors and devices work at a level
 * where functional behavior is equivalent to what a programmer would see using
 * the hardware.
 *
 * They sacrifice timing accuracy to achieve fast simulation execution speeds:
 * you can use the PV models for confirming software functionality, but you
 * must not rely on the accuracy of cycle counts, low-level component
 * interactions, or other hardware-specific behavior."
 *
 * As described above, FVPs sacrifice timing accuracy to achieve fast
 * simulation execution speeds. Therefore, we need this work around of setting
 * `configTICK_RATE_HZ` to `60` to simulate scheduler polling rate of
 * `1000 Hz` or 1 tick per second.
 *
 * In addition, the macro `pdMS_TO_TICKS` is defined here to match the 1 tick
 * per second instead of using the macro defined in
 * `FreeRTOS-kernel/include/projdefs.h`
 */
#define configTICK_RATE_HZ    ( ( uint32_t ) 60 )
#define pdMS_TO_TICKS( xTimeInMs )    ( ( TickType_t ) xTimeInMs )
#define TICKS_TO_pdMS( xTicks )       ( ( uint32_t ) xTicks )
