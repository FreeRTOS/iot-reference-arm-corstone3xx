/*
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2016-2024, Arm Limited. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 * - Neither the name of ARM nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "acamera_command_api.h"
#include "acamera_firmware_settings.h"

/* ------------ 3A & iridix */
static uint8_t _calibration_evtolux_probability_enable[] = { 1 };

static uint8_t _calibration_awb_avg_coef[] = { 30 };

static uint8_t _calibration_iridix_avg_coef[] = { 30 }; /* 7 */

static uint16_t _calibration_ccm_one_gain_threshold[] = { 256 * 10 };

static uint8_t _calibration_iridix_strength_maximum[] = { 255 };

static uint16_t _calibration_iridix_min_max_str[] = { 0 };

static uint32_t _calibration_iridix_ev_lim_full_str[] = { 1000000 };        /* 100 */

static uint32_t _calibration_iridix_ev_lim_no_str[] = { 5792559, 2997049 }; /* 5792559, 2997049 */

static uint8_t _calibration_ae_correction[] = { 128 };

static uint32_t _calibration_ae_exposure_correction[] = { 500 };

/* ------------ Sinter */
static uint16_t _calibration_sinter_strength[][ 2 ] =
{
    { 0 * 256, 35 }, /* 30 */
    { 1 * 256, 36 }, /* 30 */
    { 2 * 256, 38 }, /* 45 */
    { 3 * 256, 55 }, /* 60 */
    { 4 * 256, 18 }, /* 73 */
    { 5 * 256, 25 }, /* 74 */
    { 6 * 256, 35 }, /* 74 */
    { 7 * 256, 55 }, /* 82 */
    { 8 * 256, 60 }, /* 82 */
    { 9 * 256, 80 } /* 82 */
};

static uint16_t _calibration_sinter_strength_MC_contrast[][ 2 ] = { { 10, 0 } };

static uint16_t _calibration_sinter_strength1[][ 2 ] = { { 0 * 256,  0   },   /* 255 */
                                                         { 1 * 256,  0   },   /* 255 */
                                                         { 2 * 256,  10  },   /* 255 */
                                                         { 3 * 256,  20  },   /* 255 */
                                                         { 4 * 256,  45  },   /* 255 4 int */
                                                         { 5 * 256,  50  },   /* 255 4 int */
                                                         { 6 * 256,  55  },   /* 255 4 int */
                                                         { 7 * 256,  65  },   /* 255 4 int */
                                                         { 8 * 256,  65  },   /* 255 4 int */
                                                         { 9 * 256,  85  },   /* 255 4 int */
                                                         { 10 * 256, 110 } }; /* 255 4 int */

static uint16_t _calibration_sinter_thresh1[][ 2 ] =
{
    { 0 * 256, 0 }, { 1 * 256, 0 }, { 2 * 256, 2 }, { 3 * 256, 3 }, { 4 * 256, 4 }, { 5 * 256, 4 }, { 6 * 256, 5 }
};

static uint16_t _calibration_sinter_thresh4[][ 2 ] =
{
    { 0 * 256, 0 }, { 1 * 256, 0 }, { 2 * 256, 0 }, { 3 * 256, 5 }, { 4 * 256, 64 }, { 5 * 256, 64 }, { 6 * 256, 64 }
};

static uint16_t _calibration_sinter_motion_offset_0[][ 2 ] = { { 0 * 256, 36 },
                                                               { 1 * 256, 36 },
                                                               { 2 * 256, 36 },
                                                               { 3 * 256, 36 },
                                                               { 4 * 256, 36 },
                                                               { 5 * 256, 36 },
                                                               { 6 * 256, 36 },
                                                               { 7 * 256, 36 },
                                                               { 8 * 256, 36 } };

static uint16_t _calibration_sinter_motion_offset_1[][ 2 ] = { { 0 * 256, 28 },
                                                               { 1 * 256, 28 },
                                                               { 2 * 256, 28 },
                                                               { 3 * 256, 28 },
                                                               { 4 * 256, 28 },
                                                               { 5 * 256, 28 },
                                                               { 6 * 256, 28 },
                                                               { 7 * 256, 28 },
                                                               { 8 * 256, 28 } };

static uint16_t _calibration_sinter_motion_offset_2[][ 2 ] = { { 0 * 256, 20 },
                                                               { 1 * 256, 20 },
                                                               { 2 * 256, 20 },
                                                               { 3 * 256, 20 },
                                                               { 4 * 256, 20 },
                                                               { 5 * 256, 20 },
                                                               { 6 * 256, 20 },
                                                               { 7 * 256, 20 },
                                                               { 8 * 256, 20 } };

static uint16_t _calibration_sinter_intConfig[][ 2 ] = { { 0 * 256, 10 } };

static uint8_t _calibration_sinter_radial_lut[] =
{
    0,  0,  0,  0,  0,  0,  1,  3,  4,  6,  7,  9,  10, 12, 13, 15, 16,
    18, 19, 21, 22, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

static uint16_t _calibration_sinter_radial_params[] =
{
    0,        /* rm_enable */
    1920 / 2, /* rm_centre_x */
    1080 / 2, /* rm_centre_y */
    1770      /* rm_off_centre_mult: round((2^31)/((540^2)+(960^2))) */
};

static uint16_t _calibration_sinter_sad[][ 2 ] =
{
    { 0, 8 }, { 1 * 256, 8 }, { 2 * 256, 5 }, { 3 * 256, 5 }, { 4 * 256, 9 }, { 5 * 256, 11 }, { 6 * 256, 13 }
};

static uint8_t _calibration_sinter_noise_profile_config[] =
{
    0, /* 0 sinter noise profile/add_lut (0) */
    1  /* 1 sinter noise profile/motion_offset */
};

/* ------------ Sharpening */
static uint16_t _calibration_sharp_fr_config[] =
{
    10,   /*  0 sharpen/alpha undershoot */
    280,  /*  1 sharpen/clip_str_min */
    280,  /*  2 sharpen/clip_str_max */
    300,  /*  3 sharpen/luma_thresh_low */
    1000, /*  4 sharpen/luma_thresh_high */
    1000, /*  5 sharpen/luma_slope_low */
    1700  /*  6 sharpen/luma_slope_high */
};

static uint16_t _calibration_sharp_ds1_config[] =
{
    10,   /*  0 sharpen/alpha undershoot */
    280,  /*  1 sharpen/clip_str_min */
    280,  /*  2 sharpen/clip_str_max */
    300,  /*  3 sharpen/luma_thresh_low */
    1000, /*  4 sharpen/luma_thresh_high */
    1000, /*  5 sharpen/luma_slope_low */
    1700  /*  6 sharpen/luma_slope_high */
};

static uint16_t _calibration_sharp_alt_d[][ 2 ] = { { 0 * 256,  36 },
                                                    { 1 * 256,  36 },
                                                    { 2 * 256,  36 },
                                                    { 3 * 256,  36 },
                                                    { 4 * 256,  36 },
                                                    { 5 * 256,  15 },
                                                    { 6 * 256,  15 },
                                                    { 7 * 256,  15 },
                                                    { 8 * 256,  10 },
                                                    { 9 * 256,  10 },
                                                    { 10 * 256, 5  } };

static uint16_t _calibration_sharp_alt_ud[][ 2 ] = { { 0 * 256,  20 }, /* 12 */
                                                     { 1 * 256,  12 },
                                                     { 2 * 256,  12 },
                                                     { 3 * 256,  10 },
                                                     { 4 * 256,  9  },
                                                     { 5 * 256,  7  },
                                                     { 6 * 256,  5  },
                                                     { 7 * 256,  5  },
                                                     { 8 * 256,  5  },
                                                     { 9 * 256,  5  },
                                                     { 10 * 256, 5  } };

static uint16_t _calibration_sharp_alt_du[][ 2 ] = { { 0 * 256,  70 }, /* 45 */
                                                     { 1 * 256,  70 }, /* 36 */
                                                     { 2 * 256,  68 }, /* 36 */
                                                     { 3 * 256,  65 }, /* 36 */
                                                     { 4 * 256,  60 }, /* 30 */
                                                     { 5 * 256,  50 },
                                                     { 6 * 256,  45 },
                                                     { 7 * 256,  15 },
                                                     { 8 * 256,  10 },
                                                     { 9 * 256,  10 },
                                                     { 10 * 256, 5  } };

static uint16_t _calibration_sharpen_fr[][ 2 ] = { { 0 * 256, 72 },
                                                   { 1 * 256, 70 },
                                                   { 2 * 256, 70 },
                                                   { 3 * 256, 70 },
                                                   { 4 * 256, 70 },
                                                   { 5 * 256, 65 },
                                                   { 6 * 256, 55 },
                                                   { 7 * 256, 50 },
                                                   { 8 * 256, 45 },
                                                   { 9 * 256, 55 } };

/* ------------ CNR */
static uint16_t _calibration_cnr_config[] =
{
    /* Commmon registers */
    1,     /* square_root_enable (1) */
    1,     /* enable (1) */
    0,     /* debug_reg (16) */
    150,   /* delta_factor (12) */
    525,   /* uv_seg1_offset (12) */
    63636, /* uv_seg1_slope (16) */
    1,     /* Enable gamma RGB fr (1) */
    1,     /* Enable gamma RGB ds (1) */
    /* Smith registers */
    819,   /* uv_seg1_min (12) */
    275,   /* uv_mean_offset (12) */
    62000, /* uv_mean_slope (16) */
    1023,  /* uv_var_offset (12) */
    64845, /* uv_var_slope (16) */
    275,   /* uv_delta_offset (12) */
    1536,  /* uv_delta_slope (16) */
    /* C32/52 registers */
    0,     /* mode (8) */
    63,    /* effective_kernel (6) */
    512,   /* u_center (12) */
    512,   /* v_center (12) */
    818,   /* global_offset (12) */
    205,   /* global_slope (16) */
    0,     /* uv_seg1_threshold (12) */
    0,     /* umean1_threshold (12) */
    62,    /* umean1_offset (12) */
    59316, /* umean1_slope (16) */
    0,     /* umean2_threshold (12) */
    62,    /* umean2_offset (12) */
    59316, /* umean2_slope (16) */
    0,     /* vmean1_threshold (12) */
    62,    /* vmean1_offset (12) */
    59316, /* vmean1_slope (16) */
    0,     /* vmean2_threshold (12) */
    62,    /* vmean2_offset (12) */
    59316, /* vmean2_slope (16) */
    0,     /* uv_var1_threshold (12) */
    1023,  /* uv_var1_offset (12) */
    65535, /* uv_var1_slope (16) */
    0,     /* uv_var2_threshold (12) */
    1023,  /* uv_var2_offset (12) */
    65535, /* uv_var2_slope (16) */
    16,    /* uv_var1_scale (6) */
    16,    /* uv_var2_scale (6) */
    0,     /* uv_delta1_threshold (12) */
    185,   /* uv_delta1_offset (12) */
    1536,  /* uv_delta1_slope (16) */
    0,     /* uv_delta2_threshold (12) */
    185,   /* uv_delta2_offset (12) */
    1536   /* uv_delta2_slope (16) */
};

/* ------------ Demosaic */
static uint16_t _calibration_demosaic_config[] =
{
    8075, /*  0 demosaic/min_d_strength */
    8100, /*  1 demosaic/min_ud_strength */
    819,  /*  2 demosaic/max_d_strength */
    819,  /*  3 demosaic/max_ud_strength */
    85,   /*  4 demosaic/fc alias slope */
    240,  /*  5 demosaic/uu thresh */
    240,  /*  6 demosaic/uu sh thresh */
    8     /*  7 demosaic/sad_amp */
};

static uint16_t _calibration_demosaic_fc_slope[][ 2 ] = { { 0 * 256, 150 },
                                                          { 1 * 256, 150 },
                                                          { 2 * 256, 150 },
                                                          { 3 * 256, 150 },
                                                          { 4 * 256, 150 },
                                                          { 5 * 256, 150 },
                                                          { 6 * 256, 150 },
                                                          { 7 * 256, 150 },
                                                          { 8 * 256, 150 } };

static uint16_t _calibration_demosaic_uu_slope[][ 2 ] = { { 0 * 256, 160 }, { 6 * 256, 90 } };

static uint16_t _calibration_demosaic_uu_sh_slope[][ 2 ] = { { 0 * 256, 160 }, { 6 * 256, 90 } };

static uint16_t _calibration_mesh_shading_strength[][ 2 ] = { { 0 * 256, 4096 } };

static uint16_t _calibration_saturation_strength[][ 2 ] = { { 0 * 256,  128 },
                                                            { 1 * 256,  128 },
                                                            { 2 * 256,  128 },
                                                            { 3 * 256,  128 },
                                                            { 4 * 256,  110 },
                                                            { 5 * 256,  100 },
                                                            { 6 * 256,  100 },
                                                            { 7 * 256,  90  },
                                                            { 8 * 256,  60  },
                                                            { 9 * 256,  60  },
                                                            { 10 * 256, 60  } };

/* ----------- Frame stitching motion */
static uint16_t _calibration_stitching_lm_np[][ 2 ] =
{
    { 0,       16  },
    { 1 * 256, 16  },
    { 2 * 256, 16  },
    { 3 * 256, 55  },
    { 4 * 256, 55  },
    { 5 * 256, 55  },
    { 6 * 256, 100 },
    { 1664,    100 }, /* 6.5 */
    { 7 * 256, 150 },
};

static uint16_t _calibration_stitching_lm_mov_mult[][ 2 ] =
{
    /*     {0,0} // 0 will disable motion */
    { 0 * 256, 450 }, /* */
    { 1 * 256, 500 }, /* */
    { 2 * 256, 550 }, /* */
    { 3 * 256, 600 }, /* */
    { 4 * 256, 550 }, /* */
    { 5 * 256, 550 }, /* */
    { 6 * 256, 450 }, /* */
    { 1664,    350 }, /* 6.5 */
    { 7 * 256, 250 }, /* */
    { 8 * 256, 135 }, /* */
};

static uint16_t _calibration_stitching_lm_med_noise_intensity_thresh[][ 2 ] =
{
    { 0,       32   },
    { 6 * 256, 32   },
    { 8 * 256, 4095 },
};

static uint16_t _calibration_stitching_ms_np[][ 2 ] = { { 0, 3680 }, { 1 * 256, 3680 }, { 2 * 256, 2680 } };

static uint16_t _calibration_stitching_ms_mov_mult[][ 2 ] =
{
    /*{0,0} // 0 will disable motion */
    { 0,       128 },
    { 1 * 256, 128 },
    { 2 * 256, 128 }
};

static uint16_t _calibration_stitching_svs_np[][ 2 ] = { { 0, 3680 }, { 1 * 256, 3680 }, { 2 * 256, 2680 } };

static uint16_t _calibration_stitching_svs_mov_mult[][ 2 ] =
{
    /*{0,0} // 0 will disable motion */
    { 0,       128 },
    { 1 * 256, 128 },
    { 2 * 256, 128 }
};

static uint16_t _calibration_dp_slope[][ 2 ] =
{
    { 0 * 256, 268  },
    { 1 * 256, 1700 },
    { 2 * 256, 1700 },
    { 3 * 256, 1800 },
    { 4 * 256, 1911 },
    { 5 * 256, 2200 },
    { 6 * 256, 2400 },
    { 7 * 256, 2400 },
    { 8 * 256, 2400 },
};

static uint16_t _calibration_dp_threshold[][ 2 ] = { { 0 * 256, 4095 },
                                                     { 1 * 256, 100  },
                                                     { 2 * 256, 100  },
                                                     { 3 * 256, 64   },
                                                     { 4 * 256, 64   },
                                                     { 5 * 256, 64   },
                                                     { 6 * 256, 64   },
                                                     { 7 * 256, 64   },
                                                     { 8 * 256, 60   },
                                                     { 9 * 256, 57   } };

static uint16_t _calibration_AWB_bg_max_gain[][ 2 ] =
{
    { 0 * 256, 100 },
    { 1 * 256, 100 },
    { 7 * 256, 200 },
};

/* *** NOTE: to add/remove items in partition luts, please also update SYSTEM_EXPOSURE_PARTITION_VALUE_COUNT. */
static uint16_t _calibration_cmos_exposure_partition_luts[][ 10 ] =
{
    /* {integration time, gain } */
    /* value: for integration time - milliseconds, for gains - multiplier. */
    /*        Zero value means maximum. */

    /* lut partitions_balanced */
    {
        10,
        2,
        30,
        4,
        60,
        6,
        100,
        8,
        0,
        0,
    },

    /* lut partition_int_priority */
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    },
};

static uint32_t _calibration_cmos_control[] =
{
    0,   /* enable antiflicker */
    50,  /* antiflicker frequency */
    0,   /* manual integration time */
    0,   /* manual sensor analog gain */
    0,   /* manual sensor digital gain */
    0,   /* manual isp digital gain */
    0,   /* manual max integration time */
    0,   /* max integration time */
    166, /* max sensor AG */
    0,   /* max sensor DG */
    158, /* 159 max isp DG */
    255, /* max exposure ratio */
    0,   /* integration time. */
    0,   /* sensor analog gain. log2 fixed - 5 bits */
    0,   /* sensor digital gain. log2 fixed - 5 bits */
    0,   /* isp digital gain. log2 fixed - 5 bits */
    1,   /* analog_gain_last_priority */
    4    /* analog_gain_reserve */
};

static uint32_t _calibration_status_info[] =
{
    0xFFFFFFFF, /* sys.total_gain_log2 */
    0xFFFFFFFF, /* sys.exposure_log2 */
    0xFFFFFFFF, /* awb.mix_light_contrast */
    0xFFFFFFFF, /* af.cur_lens_pos */
    0xFFFFFFFF  /* af.cur_focus_value */
};

static uint32_t _calibration_iridix8_strength_dk_enh_control[] =
{
    20,          /* dark_prc //20 */
    98,          /* bright_prc */
    1000,        /* min_dk: minimum dark enhancement //1100 */
    4000,        /* max_dk: maximum dark enhancement //4250 */
    8,           /* pD_cut_min: minimum intensity cut for dark regions in which dk_enh will be applied */
    30,          /* pD_cut_max: maximum intensity cut for dark regions in which dk_enh will be applied */
    16 << 8,     /* dark contrast min */
        60 << 8, /* dark contrast max */
        0,       /* min_str: iridix strength in percentage */
        50,      /* max_str: iridix strength in percentage: 50 = 1x gain. 100 = 2x gain */
        40,      /* dark_prc_gain_target: target in histogram (percentage) for dark_prc after iridix is applied */
        15 << 8, /* 1382,   // 5.4<<8,  // contrast_min: clip factor of strength for LDR scenes. //20 */
        5760,    /* 30<<8,  // contrast_max: clip factor of strength for HDR scenes. //50 */
        32,      /* max iridix gain */
        0        /* print debug */
};

static uint32_t _calibration_ae_control[] =
{
    30,  /* AE convergence */
    236, /* LDR AE target -> this should match the 18% grey of teh output gamma */
    16,  /* AE tail weight */
    77,  /* WDR mode only: Max percentage of clipped pixels for long exposure: WDR mode only: 256 = 100% clipped pixels */
    15,  /* WDR mode only: Time filter for exposure ratio */
    100, /* control for clipping: bright percentage of pixels that should be below hi_target_prc */
    99,  /* control for clipping: highlights percentage (hi_target_prc): target for tail of histogram */
    1,   /* 1:0 enable | disable iridix global gain. */
    10,  /* AE tolerance */
};

static uint16_t _calibration_ae_control_HDR_target[][ 2 ] =
{
    { 0 * 256, 11 }, /* HDR AE target should not be higher than LDR target */
    { 1 * 256, 11 },
    { 4 * 256, 25 },
    { 5 * 256, 25 },
    { 6 * 256, 25 },
    { 7 * 256, 25 },
    { 8 * 256, 30 },
    { 9 * 256, 40 },
};

/* ------------ Purple Fringe */
static uint16_t _calibration_pf_config[] =
{
    1,    /*  0 use_color_corrected_rgb (1) */
    768,  /*  1 hue_strength (12) */
    512,  /*  2 sat_strength (12) */
    1024, /*  3 luma_strength (12) */
    768,  /*  4 purple_strength (12) */
    512,  /*  5 saturation_strength (8) */
    0,    /*  6 sad_offset (12) */
    284,  /*  7 hue_low_slope (12) */
    0,    /*  8 hue_low_offset (12) */
    1690, /*  9 hue_low_thresh (12) */
    1422, /* 10 hue_high_slope (12) */
    0,    /* 11 hue_high_offset (12) */
    2150, /* 12 hue_high_thresh (12) */
    123,  /* 13 sat_low_slope (12) */
    0,    /* 14 sat_low_offset (12) */
    164,  /* 15 sat_low_thresh (12) */
    0,    /* 16 sat_high_slope (12) */
    4095, /* 17 sat_high_offset (12) */
    0,    /* 18 sat_high_thresh (12) */
    110,  /* 19 luma1_low_slope (12) */
    0,    /* 20 luma1_low_offset (12) */
    205,  /* 21 luma1_low_thresh (12) */
    70,   /* 22 luma1_high_slope (12) */
    0,    /* 23 luma1_high_offset (12) */
    1500, /* 24 luma1_high_thresh (12) */
    400,  /* 25 luma2_low_slope (12) */
    0,    /* 26 luma2_low_offset (12) */
    3450, /* 27 luma2_low_thresh (12) */
    1500, /* 28 luma2_high_slope (12) */
    0,    /* 29 luma2_high_offset (12) */
    3900, /* 30 luma2_high_thresh (12) */
    36,   /* 31 hsl_slope (12) */
    0,    /* 32 hsl_offset (12) */
    0,    /* 33 hsl_thresh (12) */
    0     /* 34 debug_sel (8) */
};

static uint16_t _calibration_pf_sad_slope[][ 2 ] = { { 0 * 256, 1024 },
                                                     { 1 * 256, 1024 },
                                                     { 2 * 256, 1024 },
                                                     { 3 * 256, 1024 },
                                                     { 4 * 256, 1024 },
                                                     { 5 * 256, 1024 },
                                                     { 6 * 256, 1024 },
                                                     { 7 * 256, 1024 },
                                                     { 8 * 256, 1024 } };

static uint16_t _calibration_pf_sad_thresh[][ 2 ] = { { 0 * 256, 1200 },
                                                      { 1 * 256, 1200 },
                                                      { 2 * 256, 1200 },
                                                      { 3 * 256, 1200 },
                                                      { 4 * 256, 1200 },
                                                      { 5 * 256, 1200 },
                                                      { 6 * 256, 1200 },
                                                      { 7 * 256, 1200 },
                                                      { 8 * 256, 1200 } };

static uint8_t _calibration_pf_radial_lut[] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static uint16_t _calibration_pf_radial_params[] =
{
    1920 / 2, /* rm_centre_x */
    1080 / 2, /* rm_centre_y */
    1770      /* rm_off_centre_mult: round((2^31)/((540^2)+(960^2))) */
};

static uint32_t _calibration_auto_level_control[] =
{
    1,  /* black_percentage */
    99, /* white_percentage */
    0,  /* auto_black_min */
    50, /* auto_black_max */
    75, /* auto_white_prc */
    5,  /* avg_coeff */
    1   /* enable_auto_level */
};

static uint16_t _calibration_exposure_ratio_adjustment[][ 2 ] =
{
    /* contrast u8.8, adjustment u8.8 */
    { 1 * 256, 256 },
    /* {16 * 256, 256}, */
    /* {34 * 256, 256}, */
    /* {48 * 256, 400},  //500 */
    /* {60 * 256, 400},  //450 */
    /* {70 * 256, 700},  //750 */
    /* {128 * 256, 768}, //600 */
};

static uint16_t _calibration_cnr_uv_delta12_slope[][ 2 ] =
{
    { 0 * 256, 1500 }, /* 3800 */
    { 1 * 256, 2000 },
    { 2 * 256, 2100 },
    { 3 * 256, 2100 },
    { 4 * 256, 2100 },
    { 5 * 256, 3500 },
    { 6 * 256, 3500 },
    { 7 * 256, 3500 },
    { 8 * 256, 2700 },
    { 9 * 256, 2500 },
};

static uint32_t _calibration_af_lms[] =
{
    70 << 6,                           /* Down_FarEnd */
        70 << 6,                       /* Hor_FarEnd */
        70 << 6,                       /* Up_FarEnd */
        112 << 6,                      /* Down_Infinity */
        112 << 6,                      /* Hor_Infinity */
        112 << 6,                      /* Up_Infinity */
                                       /* 50<<6, // Down_FarEnd */
                                       /* 50<<6, // Hor_FarEnd */
                                       /* 50<<6, // Up_FarEnd */
                                       /* 167<<6, // Down_Infinity */
                                       /* 167<<6, // Hor_Infinity */
                                       /* 167<<6, // Up_Infinity */
        832 << 6,                      /* Down_Macro */
        832 << 6,                      /* Hor_Macro */
        832 << 6,                      /* Up_Macro */
        915 << 6,                      /* Down_NearEnd */
        915 << 6,                      /* Hor_NearEnd */
        915 << 6,                      /* Up_NearEnd */
        11,                            /* step_num */
        6,                             /* skip_frames_init */
        2,                             /* skip_frames_move */
        30,                            /* dynamic_range_th */
        2 << ( LOG2_GAIN_SHIFT - 2 ),  /* spot_tolerance */
        1 << ( LOG2_GAIN_SHIFT - 1 ),  /* exit_th */
        16 << ( LOG2_GAIN_SHIFT - 4 ), /* caf_trigger_th */
        4 << ( LOG2_GAIN_SHIFT - 4 ),  /* caf_stable_th */
        0                              /* print_debug */
};

static uint16_t _calibration_af_input_roi[] =
{
    20, /* AF input ROI left border, pixels */
    20, /* AF input ROI right border, pixels */
    10, /* AF input ROI top border, pixels */
    10  /* AF input ROI bottom border, pixels */
};

static uint16_t _calibration_af_lms_exit_threshold[][ 2 ] =
{
    { 0 * 256, 120 }, /* 120 = 20% threshold (1.20) */
    { 1 * 256, 120 },
    { 2 * 256, 120 },
    { 3 * 256, 120 },
    { 4 * 256, 120 },
    { 5 * 256, 120 },
    { 6 * 256, 120 },
    { 7 * 256, 120 },
};

static uint16_t _calibration_af_zone_wght_hor[] = { 0, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 0 };
static uint16_t _calibration_af_zone_wght_ver[] = { 0, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 0 };

static uint16_t _calibration_fs_mc_off[] =
{
    8 * 256, /* gain_log2 threshold. if gain is higher than the current gain_log2. mc off mode will be enabled. */
};

static int16_t _calibration_awb_colour_preference[] = { 7500, 6000, 4700, 2800 };

static uint16_t _calibration_rgb2yuv_conversion[] =
{
    76, 150, 29, 0x8025, 0x8049, 111, 157, 0x8083, 0x8019, 0, 512, 512
};

static uint16_t _calibration_awb_zone_wght_hor[] =
{
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};
static uint16_t _calibration_awb_zone_wght_ver[] =
{
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

static uint16_t _calibration_sharpen_ds1[][ 2 ] = { { 0 * 256, 70 },
                                                    { 1 * 256, 70 },
                                                    { 2 * 256, 70 },
                                                    { 3 * 256, 70 },
                                                    { 4 * 256, 70 },
                                                    { 5 * 256, 50 },
                                                    { 6 * 256, 40 },
                                                    { 7 * 256, 25 },
                                                    { 8 * 256, 10 } };

/* ------------ Temper */
static uint16_t _calibration_temper_strength[][ 2 ] =
{
    { 0 * 256, 115 },
    { 1 * 256, 115 },
    { 2 * 256, 115 },
    { 3 * 256, 125 },
    { 4 * 256, 125 },
    { 5 * 256, 125 },
    { 6 * 256, 125 },
    { 7 * 256, 127 },
    { 8 * 256, 127 },
    { 9 * 256, 147 },
};

static uint8_t _calibration_temper_noise_profile_config[] =
{
    61, /* 0 temper noise profile/motion_thresh_0 */
    58, /* 1 temper noise profile/motion_thresh_1 */
    53  /* 2 temper noise profile/motion_thresh_2 */
};

static uint32_t _calibration_custom_settings_context[][ 4 ] =
{
    /* stop sequence - address is 0x0000 */
    { 0x0000, 0x0000, 0x0000, 0x0000 }
};

static int32_t _calibration_gamma_threshold[] = { 100000, 2000000 };

/* CALIBRATION_GAMMA_EV1 */
static uint16_t _calibration_gamma_ev1[] =

/*sRGB highcontrast{0, 150, 261, 359, 452, 541, 623, 702, 781, 859, 937, 1014, 1087, 1158, 1224, 1288, 1348, 1407,
 * 1464, 1519, 1572, 1625, 1676, 1727, 1775, 1823, 1869, 1913, 1956, 1999, 2041, 2082, 2123, 2162, 2201, 2238, 2276,
 * 2312, 2348, 2383, 2417, 2451, 2485, 2516, 2549, 2580, 2611, 2641, 2671, 2701, 2730, 2759, 2787, 2816, 2843, 2871,
 * 2897, 2923, 2950, 2975, 3000, 3025, 3048, 3071, 3095, 3118, 3139, 3161, 3182, 3204, 3224, 3244, 3263, 3283, 3302,
 * 3322, 3340, 3358, 3377, 3394, 3412, 3429, 3447, 3464, 3481, 3497, 3514, 3530, 3546, 3562, 3579, 3594, 3610, 3625,
 * 3641, 3656, 3671, 3686, 3701, 3716, 3731, 3745, 3759, 3774, 3788, 3802, 3816, 3830, 3843, 3857, 3871, 3884, 3898,
 * 3911, 3924, 3936, 3949, 3962, 3974, 3987, 4000, 4011, 4024, 4036, 4048, 4060, 4072, 4083, 4095}; */

/*sRGB
 * 65{0,192,318,419,511,596,675,749,820,887,950,1012,1070,1126,1180,1231,1282,1332,1380,1428,1475,1521,1568,1614,1660,1706,1751,1796,1842,1890,1938,1988,2037,2085,2133,2180,2228,2273,2319,2363,2406,2447,2489,2528,2566,2603,2638,2671,2703,2734,2762,2790,2818,2845,2871,2897,2921,2946,2970,2993,3016,3038,3060,3081,3103,3123,3143,3163,3183,3203,3222,3241,3259,3278,3296,3315,3333,3351,3369,3386,3403,3420,3438,3455,3472,3489,3506,3522,3539,3555,3572,3588,3604,3620,3635,3651,3666,3681,3696,3712,3726,3741,3755,3770,3784,3798,3813,3827,3840,3854,3868,3881,3895,3908,3921,3934,3947,3960,3972,3985,3998,4010,4023,4035,4048,4060,4071,4083,4095};
 */
{
    0,    250,  406,  527,  630,  723,  807,  889,  969,  1045, 1120, 1192, 1260, 1327, 1391, 1453, 1512, 1570, 1625,
    1677, 1728, 1777, 1824, 1870, 1913, 1955, 1995, 2033, 2069, 2105, 2140, 2174, 2207, 2238, 2270, 2300, 2330, 2359,
    2387, 2415, 2443, 2469, 2496, 2522, 2548, 2573, 2598, 2622, 2646, 2671, 2694, 2717, 2740, 2763, 2786, 2809, 2830,
    2852, 2875, 2896, 2918, 2939, 2960, 2981, 3002, 3023, 3043, 3064, 3084, 3105, 3125, 3145, 3164, 3184, 3203, 3223,
    3242, 3262, 3281, 3299, 3318, 3336, 3355, 3373, 3391, 3409, 3427, 3445, 3463, 3480, 3498, 3515, 3532, 3549, 3567,
    3584, 3600, 3617, 3633, 3650, 3667, 3683, 3699, 3715, 3731, 3748, 3764, 3780, 3795, 3811, 3827, 3842, 3858, 3873,
    3888, 3904, 3919, 3934, 3948, 3964, 3979, 3993, 4008, 4023, 4038, 4052, 4066, 4081, 4095
};

/* CALIBRATION_GAMMA_EV2 */
static uint16_t _calibration_gamma_ev2[] =

/*sRGB highcontrast{0, 150, 261, 359, 452, 541, 623, 702, 781, 859, 937, 1014, 1087, 1158, 1224, 1288, 1348, 1407,
 * 1464, 1519, 1572, 1625, 1676, 1727, 1775, 1823, 1869, 1913, 1956, 1999, 2041, 2082, 2123, 2162, 2201, 2238, 2276,
 * 2312, 2348, 2383, 2417, 2451, 2485, 2516, 2549, 2580, 2611, 2641, 2671, 2701, 2730, 2759, 2787, 2816, 2843, 2871,
 * 2897, 2923, 2950, 2975, 3000, 3025, 3048, 3071, 3095, 3118, 3139, 3161, 3182, 3204, 3224, 3244, 3263, 3283, 3302,
 * 3322, 3340, 3358, 3377, 3394, 3412, 3429, 3447, 3464, 3481, 3497, 3514, 3530, 3546, 3562, 3579, 3594, 3610, 3625,
 * 3641, 3656, 3671, 3686, 3701, 3716, 3731, 3745, 3759, 3774, 3788, 3802, 3816, 3830, 3843, 3857, 3871, 3884, 3898,
 * 3911, 3924, 3936, 3949, 3962, 3974, 3987, 4000, 4011, 4024, 4036, 4048, 4060, 4072, 4083, 4095}; */

/*sRGB
 * 65{0,192,318,419,511,596,675,749,820,887,950,1012,1070,1126,1180,1231,1282,1332,1380,1428,1475,1521,1568,1614,1660,1706,1751,1796,1842,1890,1938,1988,2037,2085,2133,2180,2228,2273,2319,2363,2406,2447,2489,2528,2566,2603,2638,2671,2703,2734,2762,2790,2818,2845,2871,2897,2921,2946,2970,2993,3016,3038,3060,3081,3103,3123,3143,3163,3183,3203,3222,3241,3259,3278,3296,3315,3333,3351,3369,3386,3403,3420,3438,3455,3472,3489,3506,3522,3539,3555,3572,3588,3604,3620,3635,3651,3666,3681,3696,3712,3726,3741,3755,3770,3784,3798,3813,3827,3840,3854,3868,3881,3895,3908,3921,3934,3947,3960,3972,3985,3998,4010,4023,4035,4048,4060,4071,4083,4095};
 */
{
    0,    250,  406,  527,  630,  723,  807,  889,  969,  1045, 1120, 1192, 1260, 1327, 1391, 1453, 1512, 1570, 1625,
    1677, 1728, 1777, 1824, 1870, 1913, 1955, 1995, 2033, 2069, 2105, 2140, 2174, 2207, 2238, 2270, 2300, 2330, 2359,
    2387, 2415, 2443, 2469, 2496, 2522, 2548, 2573, 2598, 2622, 2646, 2671, 2694, 2717, 2740, 2763, 2786, 2809, 2830,
    2852, 2875, 2896, 2918, 2939, 2960, 2981, 3002, 3023, 3043, 3064, 3084, 3105, 3125, 3145, 3164, 3184, 3203, 3223,
    3242, 3262, 3281, 3299, 3318, 3336, 3355, 3373, 3391, 3409, 3427, 3445, 3463, 3480, 3498, 3515, 3532, 3549, 3567,
    3584, 3600, 3617, 3633, 3650, 3667, 3683, 3699, 3715, 3731, 3748, 3764, 3780, 3795, 3811, 3827, 3842, 3858, 3873,
    3888, 3904, 3919, 3934, 3948, 3964, 3979, 3993, 4008, 4023, 4038, 4052, 4066, 4081, 4095
};

static LookupTable calibration_gamma_threshold =
{
    .ptr   = _calibration_gamma_threshold,
    .rows  = 1,
    .cols  = sizeof( _calibration_gamma_threshold )
             / sizeof( _calibration_gamma_threshold[ 0 ] ),
    .width = sizeof( _calibration_gamma_threshold[ 0 ] )
};
static LookupTable calibration_gamma_ev1 =
{
    .ptr   = _calibration_gamma_ev1,
    .rows  = 1,
    .cols  = sizeof( _calibration_gamma_ev1 ) / sizeof( _calibration_gamma_ev1[ 0 ] ),
    .width = sizeof( _calibration_gamma_ev1[ 0 ] )
};
static LookupTable calibration_gamma_ev2 =
{
    .ptr   = _calibration_gamma_ev2,
    .rows  = 1,
    .cols  = sizeof( _calibration_gamma_ev2 ) / sizeof( _calibration_gamma_ev2[ 0 ] ),
    .width = sizeof( _calibration_gamma_ev2[ 0 ] )
};
static LookupTable calibration_fs_mc_off =
{
    .ptr   = _calibration_fs_mc_off,
    .rows  = 1,
    .cols  = sizeof( _calibration_fs_mc_off ) / sizeof( _calibration_fs_mc_off[ 0 ] ),
    .width = sizeof( _calibration_fs_mc_off[ 0 ] )
};
static LookupTable calibration_exposure_ratio_adjustment =
{
    .ptr   = _calibration_exposure_ratio_adjustment,
    .rows  = sizeof( _calibration_exposure_ratio_adjustment ) / sizeof( _calibration_exposure_ratio_adjustment[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_exposure_ratio_adjustment[ 0 ][ 0 ] )
};
static LookupTable calibration_awb_colour_preference =
{
    .ptr   = _calibration_awb_colour_preference,
    .rows  = 1,
    .cols  = sizeof( _calibration_awb_colour_preference )
             / sizeof( _calibration_awb_colour_preference[ 0 ] ),
    .width = sizeof( _calibration_awb_colour_preference[ 0 ] )
};
static LookupTable calibration_sinter_strength_MC_contrast =
{
    .ptr   = _calibration_sinter_strength_MC_contrast,
    .rows  = sizeof( _calibration_sinter_strength_MC_contrast ) / sizeof( _calibration_sinter_strength_MC_contrast[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_strength_MC_contrast[ 0 ][ 0 ] )
};
static LookupTable calibration_pf_config =
{
    .ptr   = _calibration_pf_config,
    .rows  = 1,
    .cols  = sizeof( _calibration_pf_config ) / sizeof( _calibration_pf_config[ 0 ] ),
    .width = sizeof( _calibration_pf_config[ 0 ] )
};
static LookupTable calibration_pf_radial_lut =
{
    .ptr   = _calibration_pf_radial_lut,
    .rows  = 1,
    .cols  = sizeof( _calibration_pf_radial_lut )
             / sizeof( _calibration_pf_radial_lut[ 0 ] ),
    .width = sizeof( _calibration_pf_radial_lut[ 0 ] )
};
static LookupTable calibration_pf_radial_params =
{
    .ptr   = _calibration_pf_radial_params,
    .rows  = 1,
    .cols  = sizeof( _calibration_pf_radial_params )
             / sizeof( _calibration_pf_radial_params[ 0 ] ),
    .width = sizeof( _calibration_pf_radial_params[ 0 ] )
};
static LookupTable calibration_sinter_radial_lut =
{
    .ptr   = _calibration_sinter_radial_lut,
    .rows  = 1,
    .cols  = sizeof( _calibration_sinter_radial_lut )
             / sizeof( _calibration_sinter_radial_lut[ 0 ] ),
    .width = sizeof( _calibration_sinter_radial_lut[ 0 ] )
};
static LookupTable calibration_sinter_radial_params =
{
    .ptr   = _calibration_sinter_radial_params,
    .rows  = 1,
    .cols  = sizeof( _calibration_sinter_radial_params )
             / sizeof( _calibration_sinter_radial_params[ 0 ] ),
    .width = sizeof( _calibration_sinter_radial_params[ 0 ] )
};
static LookupTable calibration_sinter_noise_profile_config =
{
    .ptr   = _calibration_sinter_noise_profile_config,
    .rows  = 1,
    .cols  = sizeof( _calibration_sinter_noise_profile_config ) / sizeof( _calibration_sinter_noise_profile_config[ 0 ] ),
    .width = sizeof( _calibration_sinter_noise_profile_config[ 0 ] )
};
static LookupTable calibration_AWB_bg_max_gain =
{
    .ptr   = _calibration_AWB_bg_max_gain,
    .rows  = sizeof( _calibration_AWB_bg_max_gain )
             / sizeof( _calibration_AWB_bg_max_gain[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_AWB_bg_max_gain[ 0 ][ 0 ] )
};
static LookupTable calibration_iridix8_strength_dk_enh_control =
{
    .ptr   = _calibration_iridix8_strength_dk_enh_control,
    .rows  = 1,
    .cols  =
        sizeof( _calibration_iridix8_strength_dk_enh_control ) / sizeof( _calibration_iridix8_strength_dk_enh_control[ 0 ] ),
    .width = sizeof( _calibration_iridix8_strength_dk_enh_control[ 0 ] )
};
static LookupTable calibration_auto_level_control =
{
    .ptr   = _calibration_auto_level_control,
    .rows  = 1,
    .cols  = sizeof( _calibration_auto_level_control )
             / sizeof( _calibration_auto_level_control[ 0 ] ),
    .width = sizeof( _calibration_auto_level_control[ 0 ] )
};
static LookupTable calibration_dp_threshold =
{
    .ptr   = _calibration_dp_threshold,
    .rows  = sizeof( _calibration_dp_threshold )
             / sizeof( _calibration_dp_threshold[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_dp_threshold[ 0 ][ 0 ] )
};
static LookupTable calibration_stitching_lm_np =
{
    .ptr   = _calibration_stitching_lm_np,
    .rows  = sizeof( _calibration_stitching_lm_np )
             / sizeof( _calibration_stitching_lm_np[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_stitching_lm_np[ 0 ][ 0 ] )
};
static LookupTable calibration_stitching_lm_med_noise_intensity_thresh =
{
    .ptr   = _calibration_stitching_lm_med_noise_intensity_thresh,
    .rows  = sizeof( _calibration_stitching_lm_med_noise_intensity_thresh )
             / sizeof( _calibration_stitching_lm_med_noise_intensity_thresh[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_stitching_lm_med_noise_intensity_thresh[ 0 ][ 0 ] )
};
static LookupTable calibration_stitching_lm_mov_mult =
{
    .ptr   = _calibration_stitching_lm_mov_mult,
    .rows  = sizeof( _calibration_stitching_lm_mov_mult )
             / sizeof( _calibration_stitching_lm_mov_mult[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_stitching_lm_mov_mult[ 0 ][ 0 ] )
};
static LookupTable calibration_stitching_ms_np =
{
    .ptr   = _calibration_stitching_ms_np,
    .rows  = sizeof( _calibration_stitching_ms_np )
             / sizeof( _calibration_stitching_ms_np[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_stitching_ms_np[ 0 ][ 0 ] )
};
static LookupTable calibration_stitching_ms_mov_mult =
{
    .ptr   = _calibration_stitching_ms_mov_mult,
    .rows  = sizeof( _calibration_stitching_ms_mov_mult )
             / sizeof( _calibration_stitching_ms_mov_mult[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_stitching_ms_mov_mult[ 0 ][ 0 ] )
};
static LookupTable calibration_stitching_svs_np =
{
    .ptr   = _calibration_stitching_svs_np,
    .rows  = sizeof( _calibration_stitching_svs_np )
             / sizeof( _calibration_stitching_svs_np[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_stitching_svs_np[ 0 ][ 0 ] )
};
static LookupTable calibration_stitching_svs_mov_mult =
{
    .ptr   = _calibration_stitching_svs_mov_mult,
    .rows  = sizeof( _calibration_stitching_svs_mov_mult )
             / sizeof( _calibration_stitching_svs_mov_mult[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_stitching_svs_mov_mult[ 0 ][ 0 ] )
};
static LookupTable calibration_evtolux_probability_enable =
{
    .ptr   = _calibration_evtolux_probability_enable,
    .rows  = 1,
    .cols  = sizeof( _calibration_evtolux_probability_enable ) / sizeof( _calibration_evtolux_probability_enable[ 0 ] ),
    .width = sizeof( _calibration_evtolux_probability_enable[ 0 ] )
};
static LookupTable calibration_awb_avg_coef =
{
    .ptr   = _calibration_awb_avg_coef,
    .rows  = 1,
    .cols  = sizeof( _calibration_awb_avg_coef )
             / sizeof( _calibration_awb_avg_coef[ 0 ] ),
    .width = sizeof( _calibration_awb_avg_coef[ 0 ] )
};
static LookupTable calibration_iridix_avg_coef =
{
    .ptr   = _calibration_iridix_avg_coef,
    .rows  = 1,
    .cols  = sizeof( _calibration_iridix_avg_coef )
             / sizeof( _calibration_iridix_avg_coef[ 0 ] ),
    .width = sizeof( _calibration_iridix_avg_coef[ 0 ] )
};
static LookupTable calibration_iridix_strength_maximum =
{
    .ptr   = _calibration_iridix_strength_maximum,
    .rows  = 1,
    .cols  = sizeof( _calibration_iridix_strength_maximum )
             / sizeof( _calibration_iridix_strength_maximum[ 0 ] ),
    .width = sizeof( _calibration_iridix_strength_maximum[ 0 ] )
};
static LookupTable calibration_iridix_min_max_str =
{
    .ptr   = _calibration_iridix_min_max_str,
    .rows  = 1,
    .cols  = sizeof( _calibration_iridix_min_max_str )
             / sizeof( _calibration_iridix_min_max_str[ 0 ] ),
    .width = sizeof( _calibration_iridix_min_max_str[ 0 ] )
};
static LookupTable calibration_iridix_ev_lim_full_str =
{
    .ptr   = _calibration_iridix_ev_lim_full_str,
    .rows  = 1,
    .cols  = sizeof( _calibration_iridix_ev_lim_full_str )
             / sizeof( _calibration_iridix_ev_lim_full_str[ 0 ] ),
    .width = sizeof( _calibration_iridix_ev_lim_full_str[ 0 ] )
};
static LookupTable calibration_iridix_ev_lim_no_str =
{
    .ptr   = _calibration_iridix_ev_lim_no_str,
    .rows  = 1,
    .cols  = sizeof( _calibration_iridix_ev_lim_no_str )
             / sizeof( _calibration_iridix_ev_lim_no_str[ 0 ] ),
    .width = sizeof( _calibration_iridix_ev_lim_no_str[ 0 ] )
};
static LookupTable calibration_ae_correction =
{
    .ptr   = _calibration_ae_correction,
    .rows  = 1,
    .cols  = sizeof( _calibration_ae_correction )
             / sizeof( _calibration_ae_correction[ 0 ] ),
    .width = sizeof( _calibration_ae_correction[ 0 ] )
};
static LookupTable calibration_ae_exposure_correction =
{
    .ptr   = _calibration_ae_exposure_correction,
    .rows  = 1,
    .cols  = sizeof( _calibration_ae_exposure_correction )
             / sizeof( _calibration_ae_exposure_correction[ 0 ] ),
    .width = sizeof( _calibration_ae_exposure_correction[ 0 ] )
};
static LookupTable calibration_sinter_strength =
{
    .ptr   = _calibration_sinter_strength,
    .rows  = sizeof( _calibration_sinter_strength )
             / sizeof( _calibration_sinter_strength[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_strength[ 0 ][ 0 ] )
};
static LookupTable calibration_sinter_strength1 =
{
    .ptr   = _calibration_sinter_strength1,
    .rows  = sizeof( _calibration_sinter_strength1 )
             / sizeof( _calibration_sinter_strength1[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_strength1[ 0 ][ 0 ] )
};
static LookupTable calibration_sinter_thresh1 =
{
    .ptr   = _calibration_sinter_thresh1,
    .rows  = sizeof( _calibration_sinter_thresh1 )
             / sizeof( _calibration_sinter_thresh1[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_thresh1[ 0 ][ 0 ] )
};
static LookupTable calibration_sinter_thresh4 =
{
    .ptr   = _calibration_sinter_thresh4,
    .rows  = sizeof( _calibration_sinter_thresh4 )
             / sizeof( _calibration_sinter_thresh4[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_thresh4[ 0 ][ 0 ] )
};
static LookupTable calibration_sinter_intConfig =
{
    .ptr   = _calibration_sinter_intConfig,
    .rows  = sizeof( _calibration_sinter_intConfig )
             / sizeof( _calibration_sinter_intConfig[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_intConfig[ 0 ][ 0 ] )
};
static LookupTable calibration_sharp_fr_config =
{
    .ptr   = _calibration_sharp_fr_config,
    .rows  = 1,
    .cols  = sizeof( _calibration_sharp_fr_config )
             / sizeof( _calibration_sharp_fr_config[ 0 ] ),
    .width = sizeof( _calibration_sharp_fr_config[ 0 ] )
};
static LookupTable calibration_sharp_ds1_config =
{
    .ptr   = _calibration_sharp_ds1_config,
    .rows  = 1,
    .cols  = sizeof( _calibration_sharp_ds1_config )
             / sizeof( _calibration_sharp_ds1_config[ 0 ] ),
    .width = sizeof( _calibration_sharp_ds1_config[ 0 ] )
};
static LookupTable calibration_cnr_config =
{
    .ptr   = _calibration_cnr_config,
    .rows  = 1,
    .cols  =
        sizeof( _calibration_cnr_config ) / sizeof( _calibration_cnr_config[ 0 ] ),
    .width = sizeof( _calibration_cnr_config[ 0 ] )
};
static LookupTable calibration_sharp_alt_d =
{
    .ptr   = _calibration_sharp_alt_d,
    .rows  = sizeof( _calibration_sharp_alt_d )
             / sizeof( _calibration_sharp_alt_d[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sharp_alt_d[ 0 ][ 0 ] )
};
static LookupTable calibration_sharp_alt_ud =
{
    .ptr   = _calibration_sharp_alt_ud,
    .rows  = sizeof( _calibration_sharp_alt_ud )
             / sizeof( _calibration_sharp_alt_ud[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sharp_alt_ud[ 0 ][ 0 ] )
};
static LookupTable calibration_sharp_alt_du =
{
    .ptr   = _calibration_sharp_alt_du,
    .rows  = sizeof( _calibration_sharp_alt_du )
             / sizeof( _calibration_sharp_alt_du[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sharp_alt_du[ 0 ][ 0 ] )
};
static LookupTable calibration_sharpen_fr =
{
    .ptr   = _calibration_sharpen_fr,
    .rows  =
        sizeof( _calibration_sharpen_fr ) / sizeof( _calibration_sharpen_fr[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sharpen_fr[ 0 ][ 0 ] )
};
static LookupTable calibration_demosaic_config =
{
    .ptr   = _calibration_demosaic_config,
    .rows  = 1,
    .cols  = sizeof( _calibration_demosaic_config )
             / sizeof( _calibration_demosaic_config[ 0 ] ),
    .width = sizeof( _calibration_demosaic_config[ 0 ] )
};
static LookupTable calibration_demosaic_uu_slope =
{
    .ptr   = _calibration_demosaic_uu_slope,
    .rows  = sizeof( _calibration_demosaic_uu_slope )
             / sizeof( _calibration_demosaic_uu_slope[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_demosaic_uu_slope[ 0 ][ 0 ] )
};
static LookupTable calibration_demosaic_uu_sh_slope =
{
    .ptr   = _calibration_demosaic_uu_sh_slope,
    .rows  = sizeof( _calibration_demosaic_uu_sh_slope )
             / sizeof( _calibration_demosaic_uu_sh_slope[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_demosaic_uu_sh_slope[ 0 ][ 0 ] )
};
static LookupTable calibration_mesh_shading_strength =
{
    .ptr   = _calibration_mesh_shading_strength,
    .rows  = sizeof( _calibration_mesh_shading_strength )
             / sizeof( _calibration_mesh_shading_strength[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_mesh_shading_strength[ 0 ][ 0 ] )
};
static LookupTable calibration_saturation_strength =
{
    .ptr   = _calibration_saturation_strength,
    .rows  = sizeof( _calibration_saturation_strength )
             / sizeof( _calibration_saturation_strength[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_saturation_strength[ 0 ][ 0 ] )
};
static LookupTable calibration_ccm_one_gain_threshold =
{
    .ptr   = _calibration_ccm_one_gain_threshold,
    .cols  = sizeof( _calibration_ccm_one_gain_threshold )
             / sizeof( _calibration_ccm_one_gain_threshold[ 0 ] ),
    .rows  = 1,
    .width = sizeof( _calibration_ccm_one_gain_threshold[ 0 ] )
};
static LookupTable calibration_cmos_exposure_partition_luts =
{
    .ptr   = _calibration_cmos_exposure_partition_luts,
    .rows  = sizeof( _calibration_cmos_exposure_partition_luts ) / sizeof( _calibration_cmos_exposure_partition_luts[ 0 ] ),
    .cols  = 10,
    .width = sizeof( _calibration_cmos_exposure_partition_luts[ 0 ][ 0 ] )
};
static LookupTable calibration_cmos_control =
{
    .ptr   = _calibration_cmos_control,
    .rows  = 1,
    .cols  = sizeof( _calibration_cmos_control )
             / sizeof( _calibration_cmos_control[ 0 ] ),
    .width = sizeof( _calibration_cmos_control[ 0 ] )
};
static LookupTable calibration_status_info =
{
    .ptr   = _calibration_status_info,
    .rows  = 1,
    .cols  = sizeof( _calibration_status_info )
             / sizeof( _calibration_status_info[ 0 ] ),
    .width = sizeof( _calibration_status_info[ 0 ] )
};
static LookupTable calibration_ae_control =
{
    .ptr   = _calibration_ae_control,
    .rows  = 1,
    .cols  =
        sizeof( _calibration_ae_control ) / sizeof( _calibration_ae_control[ 0 ] ),
    .width = sizeof( _calibration_ae_control[ 0 ] )
};
static LookupTable calibration_ae_control_HDR_target =
{
    .ptr   = _calibration_ae_control_HDR_target,
    .rows  = sizeof( _calibration_ae_control_HDR_target )
             / sizeof( _calibration_ae_control_HDR_target[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_ae_control_HDR_target[ 0 ][ 0 ] )
};
static LookupTable calibration_rgb2yuv_conversion =
{
    .ptr   = _calibration_rgb2yuv_conversion,
    .rows  = 1,
    .cols  = sizeof( _calibration_rgb2yuv_conversion )
             / sizeof( _calibration_rgb2yuv_conversion[ 0 ] ),
    .width = sizeof( _calibration_rgb2yuv_conversion[ 0 ] )
};
static LookupTable calibration_calibration_af_lms =
{
    .ptr   = _calibration_af_lms,
    .rows  = 1,
    .cols  =
        sizeof( _calibration_af_lms ) / sizeof( _calibration_af_lms[ 0 ] ),
    .width = sizeof( _calibration_af_lms[ 0 ] )
};
static LookupTable calibration_af_input_roi =
{
    .ptr   = _calibration_af_input_roi,
    .rows  = 1,
    .cols  = sizeof( _calibration_af_input_roi )
             / sizeof( _calibration_af_input_roi[ 0 ] ),
    .width = sizeof( _calibration_af_input_roi[ 0 ] )
};
static LookupTable calibration_af_lms_exit_threshold =
{
    .ptr   = _calibration_af_lms_exit_threshold,
    .rows  = sizeof( _calibration_af_lms_exit_threshold )
             / sizeof( _calibration_af_lms_exit_threshold[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_af_lms_exit_threshold[ 0 ][ 0 ] )
};
static LookupTable calibration_calibration_af_zone_wght_hor =
{
    .ptr   = _calibration_af_zone_wght_hor,
    .rows  = 1,
    .cols  = sizeof( _calibration_af_zone_wght_hor )
             / sizeof( _calibration_af_zone_wght_hor[ 0 ] ),
    .width = sizeof( _calibration_af_zone_wght_hor[ 0 ] )
};
static LookupTable calibration_calibration_af_zone_wght_ver =
{
    .ptr   = _calibration_af_zone_wght_ver,
    .rows  = 1,
    .cols  = sizeof( _calibration_af_zone_wght_ver )
             / sizeof( _calibration_af_zone_wght_ver[ 0 ] ),
    .width = sizeof( _calibration_af_zone_wght_ver[ 0 ] )
};
static LookupTable calibration_calibration_awb_zone_wght_hor =
{
    .ptr   = _calibration_awb_zone_wght_hor,
    .rows  = 1,
    .cols  = sizeof( _calibration_awb_zone_wght_hor )
             / sizeof( _calibration_awb_zone_wght_hor[ 0 ] ),
    .width = sizeof( _calibration_awb_zone_wght_hor[ 0 ] )
};
static LookupTable calibration_calibration_awb_zone_wght_ver =
{
    .ptr   = _calibration_awb_zone_wght_ver,
    .rows  = 1,
    .cols  = sizeof( _calibration_awb_zone_wght_ver )
             / sizeof( _calibration_awb_zone_wght_ver[ 0 ] ),
    .width = sizeof( _calibration_awb_zone_wght_ver[ 0 ] )
};
static LookupTable calibration_dp_slope =
{
    .ptr   = _calibration_dp_slope,
    .rows  = sizeof( _calibration_dp_slope ) / sizeof( _calibration_dp_slope[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_dp_slope[ 0 ][ 0 ] )
};
static LookupTable calibration_cnr_uv_delta12_slope =
{
    .ptr   = _calibration_cnr_uv_delta12_slope,
    .rows  = sizeof( _calibration_cnr_uv_delta12_slope )
             / sizeof( _calibration_cnr_uv_delta12_slope[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_cnr_uv_delta12_slope[ 0 ][ 0 ] )
};
static LookupTable calibration_sinter_sad =
{
    .ptr   = _calibration_sinter_sad,
    .rows  =
        sizeof( _calibration_sinter_sad ) / sizeof( _calibration_sinter_sad[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_sad[ 0 ][ 0 ] )
};
static LookupTable calibration_sharpen_ds1 =
{
    .ptr   = _calibration_sharpen_ds1,
    .rows  = sizeof( _calibration_sharpen_ds1 )
             / sizeof( _calibration_sharpen_ds1[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sharpen_ds1[ 0 ][ 0 ] )
};
static LookupTable calibration_temper_strength =
{
    .ptr   = _calibration_temper_strength,
    .rows  = sizeof( _calibration_temper_strength )
             / sizeof( _calibration_temper_strength[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_temper_strength[ 0 ][ 0 ] )
};
static LookupTable calibration_temper_noise_profile_config =
{
    .ptr   = _calibration_temper_noise_profile_config,
    .rows  = 1,
    .cols  = sizeof( _calibration_temper_noise_profile_config ) / sizeof( _calibration_temper_noise_profile_config[ 0 ] ),
    .width = sizeof( _calibration_temper_noise_profile_config[ 0 ] )
};
static LookupTable calibration_custom_settings_context =
{
    .ptr   = _calibration_custom_settings_context,
    .rows  = sizeof( _calibration_custom_settings_context )
             / sizeof( _calibration_custom_settings_context[ 0 ] ),
    .cols  = 4,
    .width = sizeof( _calibration_custom_settings_context[ 0 ][ 0 ] )
};
static LookupTable calibration_demosaic_fc_slope =
{
    .ptr   = _calibration_demosaic_fc_slope,
    .rows  = sizeof( _calibration_demosaic_fc_slope )
             / sizeof( _calibration_demosaic_fc_slope[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_demosaic_fc_slope[ 0 ][ 0 ] )
};
static LookupTable calibration_pf_sad_thresh =
{
    .ptr   = _calibration_pf_sad_thresh,
    .rows  = sizeof( _calibration_pf_sad_thresh )
             / sizeof( _calibration_pf_sad_thresh[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_pf_sad_thresh[ 0 ][ 0 ] )
};
static LookupTable calibration_pf_sad_slope =
{
    .ptr   = _calibration_pf_sad_slope,
    .rows  = sizeof( _calibration_pf_sad_slope )
             / sizeof( _calibration_pf_sad_slope[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_pf_sad_slope[ 0 ][ 0 ] )
};
static LookupTable calibration_sinter_motion_offset_0 =
{
    .ptr   = _calibration_sinter_motion_offset_0,
    .rows  = sizeof( _calibration_sinter_motion_offset_0 )
             / sizeof( _calibration_sinter_motion_offset_0[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_motion_offset_0[ 0 ][ 0 ] )
};
static LookupTable calibration_sinter_motion_offset_1 =
{
    .ptr   = _calibration_sinter_motion_offset_1,
    .rows  = sizeof( _calibration_sinter_motion_offset_1 )
             / sizeof( _calibration_sinter_motion_offset_1[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_motion_offset_1[ 0 ][ 0 ] )
};
static LookupTable calibration_sinter_motion_offset_2 =
{
    .ptr   = _calibration_sinter_motion_offset_2,
    .rows  = sizeof( _calibration_sinter_motion_offset_2 )
             / sizeof( _calibration_sinter_motion_offset_2[ 0 ] ),
    .cols  = 2,
    .width = sizeof( _calibration_sinter_motion_offset_2[ 0 ][ 0 ] )
};

uint32_t get_calibrations_dynamic_native_dummy( ACameraCalibrations * c )
{
    uint32_t result = 0;

    if( c != 0 )
    {
        c->calibrations[ CALIBRATION_STITCHING_LM_MED_NOISE_INTENSITY ] =
            &calibration_stitching_lm_med_noise_intensity_thresh;
        c->calibrations[ CALIBRATION_EXPOSURE_RATIO_ADJUSTMENT ] = &calibration_exposure_ratio_adjustment;
        c->calibrations[ CALIBRATION_SINTER_STRENGTH_MC_CONTRAST ] = &calibration_sinter_strength_MC_contrast;
        c->calibrations[ CALIBRATION_AWB_COLOUR_PREFERENCE ] = &calibration_awb_colour_preference;
        c->calibrations[ CALIBRATION_PF_CONFIG ] = &calibration_pf_config;
        c->calibrations[ CALIBRATION_PF_RADIAL_LUT ] = &calibration_pf_radial_lut;
        c->calibrations[ CALIBRATION_PF_RADIAL_PARAMS ] = &calibration_pf_radial_params;
        c->calibrations[ CALIBRATION_SINTER_RADIAL_LUT ] = &calibration_sinter_radial_lut;
        c->calibrations[ CALIBRATION_SINTER_RADIAL_PARAMS ] = &calibration_sinter_radial_params;
        c->calibrations[ CALIBRATION_SINTER_NOISE_PROFILE_CONFIG ] = &calibration_sinter_noise_profile_config;
        c->calibrations[ CALIBRATION_AWB_BG_MAX_GAIN ] = &calibration_AWB_bg_max_gain;
        c->calibrations[ CALIBRATION_IRIDIX8_STRENGTH_DK_ENH_CONTROL ] = &calibration_iridix8_strength_dk_enh_control;
        c->calibrations[ CALIBRATION_CMOS_EXPOSURE_PARTITION_LUTS ] = &calibration_cmos_exposure_partition_luts;
        c->calibrations[ CALIBRATION_CMOS_CONTROL ] = &calibration_cmos_control;
        c->calibrations[ CALIBRATION_STATUS_INFO ] = &calibration_status_info;
        c->calibrations[ CALIBRATION_AUTO_LEVEL_CONTROL ] = &calibration_auto_level_control;
        c->calibrations[ CALIBRATION_DP_SLOPE ] = &calibration_dp_slope;
        c->calibrations[ CALIBRATION_DP_THRESHOLD ] = &calibration_dp_threshold;
        c->calibrations[ CALIBRATION_STITCHING_LM_MOV_MULT ] = &calibration_stitching_lm_mov_mult;
        c->calibrations[ CALIBRATION_STITCHING_LM_NP ] = &calibration_stitching_lm_np;
        c->calibrations[ CALIBRATION_STITCHING_MS_MOV_MULT ] = &calibration_stitching_ms_mov_mult;
        c->calibrations[ CALIBRATION_STITCHING_MS_NP ] = &calibration_stitching_ms_np;
        c->calibrations[ CALIBRATION_STITCHING_SVS_MOV_MULT ] = &calibration_stitching_svs_mov_mult;
        c->calibrations[ CALIBRATION_STITCHING_SVS_NP ] = &calibration_stitching_svs_np;
        c->calibrations[ CALIBRATION_EVTOLUX_PROBABILITY_ENABLE ] = &calibration_evtolux_probability_enable;
        c->calibrations[ CALIBRATION_AWB_AVG_COEF ] = &calibration_awb_avg_coef;
        c->calibrations[ CALIBRATION_IRIDIX_AVG_COEF ] = &calibration_iridix_avg_coef;
        c->calibrations[ CALIBRATION_IRIDIX_STRENGTH_MAXIMUM ] = &calibration_iridix_strength_maximum;
        c->calibrations[ CALIBRATION_IRIDIX_MIN_MAX_STR ] = &calibration_iridix_min_max_str;
        c->calibrations[ CALIBRATION_IRIDIX_EV_LIM_FULL_STR ] = &calibration_iridix_ev_lim_full_str;
        c->calibrations[ CALIBRATION_IRIDIX_EV_LIM_NO_STR ] = &calibration_iridix_ev_lim_no_str;
        c->calibrations[ CALIBRATION_AE_CORRECTION ] = &calibration_ae_correction;
        c->calibrations[ CALIBRATION_AE_EXPOSURE_CORRECTION ] = &calibration_ae_exposure_correction;
        c->calibrations[ CALIBRATION_SINTER_STRENGTH ] = &calibration_sinter_strength;
        c->calibrations[ CALIBRATION_SINTER_STRENGTH1 ] = &calibration_sinter_strength1;
        c->calibrations[ CALIBRATION_SINTER_THRESH1 ] = &calibration_sinter_thresh1;
        c->calibrations[ CALIBRATION_SINTER_THRESH4 ] = &calibration_sinter_thresh4;
        c->calibrations[ CALIBRATION_SINTER_INTCONFIG ] = &calibration_sinter_intConfig;
        c->calibrations[ CALIBRATION_SHARP_FR_CONFIG ] = &calibration_sharp_fr_config;
        c->calibrations[ CALIBRATION_SHARP_DS1_CONFIG ] = &calibration_sharp_ds1_config;
        c->calibrations[ CALIBRATION_CNR_CONFIG ] = &calibration_cnr_config;
        c->calibrations[ CALIBRATION_SHARP_ALT_D ] = &calibration_sharp_alt_d;
        c->calibrations[ CALIBRATION_SHARP_ALT_UD ] = &calibration_sharp_alt_ud;
        c->calibrations[ CALIBRATION_SHARP_ALT_DU ] = &calibration_sharp_alt_du;
        c->calibrations[ CALIBRATION_SHARPEN_FR ] = &calibration_sharpen_fr;
        c->calibrations[ CALIBRATION_DEMOSAIC_CONFIG ] = &calibration_demosaic_config;
        c->calibrations[ CALIBRATION_DEMOSAIC_UU_SLOPE ] = &calibration_demosaic_uu_slope;
        c->calibrations[ CALIBRATION_DEMOSAIC_UU_SH_SLOPE ] = &calibration_demosaic_uu_sh_slope;
        c->calibrations[ CALIBRATION_MESH_SHADING_STRENGTH ] = &calibration_mesh_shading_strength;
        c->calibrations[ CALIBRATION_SATURATION_STRENGTH ] = &calibration_saturation_strength;
        c->calibrations[ CALIBRATION_CCM_ONE_GAIN_THRESHOLD ] = &calibration_ccm_one_gain_threshold;
        c->calibrations[ CALIBRATION_AE_CONTROL ] = &calibration_ae_control;
        c->calibrations[ CALIBRATION_AE_CONTROL_HDR_TARGET ] = &calibration_ae_control_HDR_target;
        c->calibrations[ CALIBRATION_RGB2YUV_CONVERSION ] = &calibration_rgb2yuv_conversion;
        c->calibrations[ CALIBRATION_AF_LMS ] = &calibration_calibration_af_lms;
        c->calibrations[ CALIBRATION_AF_INPUT_ROI ] = &calibration_af_input_roi;
        c->calibrations[ CALIBRATION_AF_LMS_EXIT_THRESHOLD ] = &calibration_af_lms_exit_threshold;
        c->calibrations[ CALIBRATION_AF_ZONE_WGHT_HOR ] = &calibration_calibration_af_zone_wght_hor;
        c->calibrations[ CALIBRATION_AF_ZONE_WGHT_VER ] = &calibration_calibration_af_zone_wght_ver;
        c->calibrations[ CALIBRATION_AWB_ZONE_WGHT_HOR ] = &calibration_calibration_awb_zone_wght_hor;
        c->calibrations[ CALIBRATION_AWB_ZONE_WGHT_VER ] = &calibration_calibration_awb_zone_wght_ver;
        c->calibrations[ CALIBRATION_CNR_UV_DELTA12_SLOPE ] = &calibration_cnr_uv_delta12_slope;
        c->calibrations[ CALIBRATION_FS_MC_OFF ] = &calibration_fs_mc_off;
        c->calibrations[ CALIBRATION_SINTER_SAD ] = &calibration_sinter_sad;
        c->calibrations[ CALIBRATION_SHARPEN_DS1 ] = &calibration_sharpen_ds1;
        c->calibrations[ CALIBRATION_TEMPER_STRENGTH ] = &calibration_temper_strength;
        c->calibrations[ CALIBRATION_TEMPER_NOISE_PROFILE_CONFIG ] = &calibration_temper_noise_profile_config;
        c->calibrations[ CALIBRATION_CUSTOM_SETTINGS_CONTEXT ] = &calibration_custom_settings_context;
        c->calibrations[ CALIBRATION_GAMMA_EV1 ] = &calibration_gamma_ev1;
        c->calibrations[ CALIBRATION_GAMMA_EV2 ] = &calibration_gamma_ev2;
        c->calibrations[ CALIBRATION_GAMMA_THRESHOLD ] = &calibration_gamma_threshold;
        c->calibrations[ CALIBRATION_DEMOSAIC_FC_SLOPE ] = &calibration_demosaic_fc_slope;
        c->calibrations[ CALIBRATION_PF_SAD_THRESH ] = &calibration_pf_sad_thresh;
        c->calibrations[ CALIBRATION_PF_SAD_SLOPE ] = &calibration_pf_sad_slope;
        c->calibrations[ CALIBRATION_SINTER_MOTION_OFFSET_0 ] = &calibration_sinter_motion_offset_0;
        c->calibrations[ CALIBRATION_SINTER_MOTION_OFFSET_1 ] = &calibration_sinter_motion_offset_1;
        c->calibrations[ CALIBRATION_SINTER_MOTION_OFFSET_2 ] = &calibration_sinter_motion_offset_2;
    }
    else
    {
        result = -1;
    }

    return result;
}
