/* Copyright 2022-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

/*
 *
 * Generated with CMSIS-DSP SDF Scripts.
 * The generated code is not covered by CMSIS-DSP license.
 *
 * The support classes and code is covered by CMSIS-DSP license.
 *
 */

#ifndef _SCHED_H_
    #define _SCHED_H_

    #include "queue.h"

    #ifdef   __cplusplus
        extern "C"
        {
    #endif

    extern uint32_t ulScheduler( int * error,
                                 DspAudioSource * dspAudio,
                                 DSPML * dspMLConnection );

    #ifdef   __cplusplus
    }
    #endif

#endif /* ifndef _SCHED_H_ */
