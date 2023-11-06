/* Copyright 2021-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "model_config.h"

#include <stdint.h>

const uint32_t g_NumMfcc = 10;
const uint32_t g_NumAudioWins = 49;

const int g_FrameLength = 512;
const int g_FrameStride = 160;
const int g_ctxLen = 98;
const float g_ScoreThreshold = 0.5;
