/* Copyright 2022-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <stdint.h>

extern const uint32_t g_NumMfcc;
extern const uint32_t g_NumAudioWins;

extern const int g_FrameLength;
extern const int g_FrameStride;
extern const int g_ctxLen;
extern const float g_ScoreThreshold;

/* Audio samples required to generate MFCC features */
/* 296 windows of 160 samples are required for inference to be processed. */
#define AUDIOFEATURELENGTH    ( 296 * 160 )

#endif /* ifndef MODEL_CONFIG_H */
