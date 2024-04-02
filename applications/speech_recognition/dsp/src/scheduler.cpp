/* Copyright 2022-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

/*
Generated with CMSIS-DSP SDF Scripts.
*/

#include "FreeRTOS.h"
#include "arm_math.h"
#include "dsp_task.h"
#include "events.h"
#include "GenericNodes.h"
#include "AppNodes.h"
#include "scheduler.h"
#include "task.h"

/***********
FIFO buffers
************/
#define FIFOSIZE0 1600
#define FIFOSIZE1 16000
#define FIFOSIZE2 47360

#define BUFFERSIZE0 1600
int16_t buf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 16000
int16_t buf1[BUFFERSIZE1]={0};

#define BUFFERSIZE2 47360
int16_t buf2[BUFFERSIZE2]={0};

extern EventGroupHandle_t xSystemEvents;

uint32_t ulScheduler(
    int *error,
    DspAudioSource *dspAudio,
    DSPML *dspMLConnection
) {
// Define CHECKERROR_OR_PAUSE
// This updated version of CHECKERROR verify if the task must be stopped or not
#define CHECKERROR_OR_PAUSE \
    if (sdfError < 0) {\
         break; \
    } else { \
        if ((xEventGroupGetBits (xSystemEvents) & EVENT_MASK_DSP_START) == 0U) { \
            break; \
        } \
    }
    int sdfError=0;
    uint32_t nbSchedule=0;

    /*
    Create FIFOs objects
    */
    FIFO<int16_t,FIFOSIZE0,0> fifo0(buf0);
    FIFO<int16_t,FIFOSIZE1,0> fifo1(buf1);
    FIFO<int16_t,FIFOSIZE2,1> fifo2(buf2);

    /*
    Create node objects
    */
    SlidingBuffer<int16_t,47360,31360> audioWin(fifo1,fifo2);
    DSP<int16_t,320,int16_t,320> dsp(fifo0,fifo1);
    MicrophoneSource<int16_t,1600> mic(fifo0,dspAudio);
    ML<int16_t,47360> ml(fifo2,dspMLConnection);

    /* Run several schedule iterations */
    while(sdfError==0)
    {
       /* Run a schedule iteration */
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = mic.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = dsp.run();
       CHECKERROR_OR_PAUSE;
       sdfError = audioWin.run();
       CHECKERROR_OR_PAUSE;
       sdfError = ml.run();
       CHECKERROR_OR_PAUSE;

       nbSchedule++;

       // Add delay to allow some time for the connectivity task
       // to send and receive messages to and from the cloud.
       vTaskDelay(120);
    }
    *error=sdfError;
    return(nbSchedule);
}
