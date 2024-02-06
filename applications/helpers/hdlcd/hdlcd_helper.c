/* Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "hdlcd_helper.h"

#include "hdlcd_drv.h"

/* RGB565 format
 *  Pixel information:
 *  <--NOT USED--> <--  RED   --> <-- GREEN --> <-- BLUE -->
 *  31 .. .. .. 16 15 .. .. .. 11 10 .. .. .. 5 4 .. .. .. 0
 */
static const struct hdlcd_pixel_cfg_t hdlcd_pixel_cfg_rgb565 =
{
    .red.default_color   = 0xff,
    .red.bit_size        = 0x5,
    .red.offset          = 0xb,
    .green.default_color = 0x0,
    .green.bit_size      = 0x6,
    .green.offset        = 0x5,
    .blue.default_color  = 0x0,
    .blue.bit_size       = 0x5,
    .blue.offset         = 0x0
};
/* RGB32 format */
static const struct hdlcd_pixel_cfg_t hdlcd_pixel_cfg_rgb32 =
{
    .red.default_color   = 0xff,
    .red.bit_size        = 0x8,
    .red.offset          = 0x10,
    .green.default_color = 0xff,
    .green.bit_size      = 0x8,
    .green.offset        = 0x8,
    .blue.default_color  = 0xff,
    .blue.bit_size       = 0x8,
    .blue.offset         = 0x0
};
/* A2R10G10B10 format. HDLCD only handles top 8 bits. */
static const struct hdlcd_pixel_cfg_t hdlcd_pixel_cfg_a2r10g10b10 =
{
    .red.default_color   = 0xff,
    .red.bit_size        = 0x8,
    .red.offset          = 0x16,
    .green.default_color = 0xff,
    .green.bit_size      = 0x8,
    .green.offset        = 0xc,
    .blue.default_color  = 0xff,
    .blue.bit_size       = 0x8,
    .blue.offset         = 0x2
};

const hdlcd_mode_t HDLCD_MODES[] = { { .pixel_format = BYTESPERPIXEL2 << PIXEL_FORMAT_BYTES_PER_PIXEL_Pos,
                                             .default_highlight_color = RGB565_YELLOW,
                                             .default_mask_color = RGB565_RED,
                                             .bytes_per_pixel = 2,
                                             .pixel_cfg = &hdlcd_pixel_cfg_rgb565 },
                                     { .pixel_format = BYTESPERPIXEL4 << PIXEL_FORMAT_BYTES_PER_PIXEL_Pos,
                                             .default_highlight_color = RGB32_YELLOW,
                                             .default_mask_color = RGB32_RED,
                                             .bytes_per_pixel = 4,
                                             .pixel_cfg = &hdlcd_pixel_cfg_rgb32 },
                                     { .pixel_format = BYTESPERPIXEL4 << PIXEL_FORMAT_BYTES_PER_PIXEL_Pos,
                                             .default_highlight_color = A2R10G10B10_YELLOW,
                                             .default_mask_color = A2R10G10B10_RED,
                                             .bytes_per_pixel = 4,
                                             .pixel_cfg = &hdlcd_pixel_cfg_a2r10g10b10 } };
