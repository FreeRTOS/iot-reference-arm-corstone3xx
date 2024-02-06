/* Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef __HDLCD_HELPER_H__
#define __HDLCD_HELPER_H__

#include "hdlcd_drv.h"
#include <stdint.h>

#define RGB565_YELLOW         0xFFE0U
#define RGB565_RED            0xF800U
#define RGB32_YELLOW          0xFFFF00U
#define RGB32_RED             0xFF0000U
#define A2R10G10B10_YELLOW    0x3FFFFC00U
#define A2R10G10B10_RED       0x3FF00000U

typedef struct hdlcd_mode
{
    uint32_t pixel_format;
    uint32_t bytes_per_pixel;
    uint32_t default_highlight_color;
    uint32_t default_mask_color;
    const struct hdlcd_pixel_cfg_t * const pixel_cfg;
} hdlcd_mode_t;

enum hdlcd_pixel_format
{
    HDLCD_PIXEL_FORMAT_RGB565 = 0,
    HDLCD_PIXEL_FORMAT_RGB32 = 1,
    HDLCD_PIXEL_FORMAT_A2R10G10B10 = 2,
    HDLCD_PIXEL_FORMAT_NOT_SUPPORTED
};

extern const hdlcd_mode_t HDLCD_MODES[ HDLCD_PIXEL_FORMAT_NOT_SUPPORTED ];

#endif /* __HDLCD_HELPER_H__ */
