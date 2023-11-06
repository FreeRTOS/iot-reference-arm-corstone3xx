/* Copyright 2022-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef _AUDIO_CONFIG_H_
#define _AUDIO_CONFIG_H_

#define AUDIO_BLOCK_NUM          ( 4 )
#define AUDIO_BLOCK_SIZE         ( 3200 )
#define AUDIO_BUFFER_SIZE        ( AUDIO_BLOCK_NUM * AUDIO_BLOCK_SIZE )

#define DSP_BLOCK_SIZE           320
#define SAMPLE_RATE              16000
#define CHANNELS                 1U
#define SAMPLE_BITS              16U
#define NOISE_LEVEL_REDUCTION    30

#endif /* ifndef _AUDIO_CONFIG_H_ */
