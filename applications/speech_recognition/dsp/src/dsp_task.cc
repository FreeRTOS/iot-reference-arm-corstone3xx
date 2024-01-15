/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "events.h"
#include "dsp_task.h"
#include "dsp_interfaces.h"
#include "model_config.h"
#include "audio_config.h"

#include "queue.h"
#include "scheduler.h"

/* Include header that defines log levels. */
#include "logging_levels.h"

extern "C" {
/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "DSP"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"
}

extern EventGroupHandle_t xSystemEvents;

#ifdef AUDIO_VSI

#include "Driver_SAI.h"

// audio constants
__attribute__((section(".bss.NoInit.vsi_audio_buffer"))) __attribute__((aligned(4)))
int16_t shared_audio_buffer[AUDIO_BUFFER_SIZE / 2];

/* Audio events definitions */
#define SEND_COMPLETED_Pos         0U                              /* Event: Send complete bit position */
#define RECEIVE_COMPLETED_Pos      1U                              /* Event: Receive complete bit position */
#define TX_UNDERFLOW_Pos           2U                              /* Event: Tx underflow bit position */
#define RX_OVERFLOW_Pos            3U                              /* Event: Rx overflow bit position */
#define FRAME_ERROR_Pos            4U                              /* Event: Frame error bit position */
#define SEND_COMPLETE_Msk          (1UL << SEND_COMPLETED_Pos)     /* Event: Send complete Mask */
#define RECEIVE_COMPLETE_Msk       (1UL << RECEIVE_COMPLETED_Pos)  /* Event: Receive complete Mask */
#define TX_UNDERFLOW_Msk           (1UL << TX_UNDERFLOW_Pos)       /* Event: Tx underflow Mask */
#define RX_OVERFLOW_Msk            (1UL << RX_OVERFLOW_Pos)        /* Event: Rx overflow Mask */
#define FRAME_ERROR_Msk            (1UL << FRAME_ERROR_Pos)        /* Event: Frame error Mask */

extern ARM_DRIVER_SAI Driver_SAI0;

extern "C" {
// Audio driver data
void (*on_event_signal)(void *);
void *on_event_ctx = nullptr;
}

// Audio driver callback function for event management
static void prvArmSaiSignalEvent(uint32_t event)
{
    if ((event & RECEIVE_COMPLETE_Msk) == ARM_SAI_EVENT_RECEIVE_COMPLETE)
    {
        if (on_event_signal)
        {
            on_event_signal(on_event_ctx);
        }
    }
}


static int prvAudioDrvSetup(void (*event_handler)(void *), void *event_handler_ptr)
{
    if (Driver_SAI0.Initialize(prvArmSaiSignalEvent) != ARM_DRIVER_OK) {
        LogError( ( "Failed to set up FVP VSI!\n" ) );
        return -1;
    }

    if (Driver_SAI0.PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK) {
        LogError( ( "Failed to set the driver to operate with full power!\n" ) );
        return -1;
    }

    if (Driver_SAI0.Control(ARM_SAI_CONTROL_RX, 1, 0) != ARM_DRIVER_OK) {
        LogError( ( "Failed to enable the VSI receiver!\n" ) );
        return -1;
    }

    if (
        Driver_SAI0.Control(
            ARM_SAI_CONFIGURE_RX | ARM_SAI_PROTOCOL_USER | ARM_SAI_DATA_SIZE(16),
            AUDIO_BLOCK_SIZE,
            static_cast<uint32_t>(SAMPLE_RATE)) != ARM_DRIVER_OK
    ) {
        LogError( ( "Failed to configure the receiver!\n" ) );
        return -1;
    }

    if (
        Driver_SAI0.Receive(
            reinterpret_cast<uint32_t *>(shared_audio_buffer), AUDIO_BLOCK_NUM
        ) != ARM_DRIVER_OK
    ) {
        LogError( ( "Failed to start receiving the data!\n" ) );
        return -1;
    }

    on_event_signal = event_handler;
    on_event_ctx = event_handler_ptr;

    return 0;
}

#else /* !defined(AUDIO_VSI) */

#include "InputFiles.hpp"

#endif // AUDIO_VSI



extern "C" {

void vDspStart(void)
{
    if(xSystemEvents == NULL)
    {
        LogError( ( "xSystemEvents is not initialised\r\n" ) );
        return;
    }

    LogInfo( ( "DSP task start\r\n" ) );

    ( void )xEventGroupSetBits( xSystemEvents, (EventBits_t)EVENT_MASK_DSP_START );
}

void vDspStop(void)
{
    if(xSystemEvents == NULL)
    {
        LogError( ( "xSystemEvents is not initialised\r\n" ) );
        return;
    }

    LogInfo( ( "DSP task stop\r\n" ) );

    ( void )xEventGroupClearBits( xSystemEvents, (EventBits_t)EVENT_MASK_DSP_START );
}
} // extern "C"

void *pvDspGetMlConnection(void)
{
   auto dspMLConnection = new DSPML(AUDIOFEATURELENGTH);
   return static_cast<void*>(dspMLConnection);
}

void vDspTask(void *pvParameters)
{
    LogInfo( ( "DSP Task start\r\n" ) );

#ifdef AUDIO_VSI
    bool first_launch = true;
    const int16_t *audioBuf = shared_audio_buffer;
    auto audioSource = DspAudioSource(audioBuf, AUDIO_BLOCK_NUM);
#else
    const int16_t *audioBuf = GetAudioArray(0);
    // This integer division for calculating the number of blocks means that,
    // any remainder data at the end of the audio clip that's smaller than a
    // block will not be accounted for. This will not have a major impact on
    // the inference result as a block is only a small fraction of a second.
    const size_t audioBlockNum = (size_t)GetAudioArraySize(0) / (AUDIO_BLOCK_SIZE / sizeof(uint16_t));
    auto audioSource = DspAudioSource(audioBuf, audioBlockNum);
#endif

    DSPML *dspMLConnection = static_cast<DSPML*>(pvParameters);

    while (1) {
        // Wait for the start message
        EventBits_t flags = xEventGroupWaitBits(xSystemEvents, (EventBits_t)EVENT_MASK_DSP_START, pdFAIL, pdFAIL, portMAX_DELAY);

        if (flags & EVENT_MASK_DSP_START)
        {
            LogInfo( ( "Initial start of audio processing\r\n" ) );
        }

#ifdef AUDIO_VSI
        if (first_launch) {
            prvAudioDrvSetup(&DspAudioSource::prvNewAudioBlockReceived, &audioSource);
            first_launch = false;
        }
#endif

        // Launch the CMSIS-DSP synchronous data flow.
        // This compute graph is defined in graph.py
        // It can be regenerated with
        // pip install cmsisdsp
        // python graph.py
        int error;
        uint32_t nbSched=ulScheduler(&error,&audioSource, dspMLConnection);
        LogInfo(
            ( "Synchronous Dataflow Scheduler ended with error %d after %i schedule loops\r\n",
            error,
            nbSched
        ) );
    }
}

void vStartDSPTask( void *pvParameters )
{
    if (
        xTaskCreate(
            vDspTask,
            "DSP_TASK",
            appCONFIG_DSP_TASK_STACK_SIZE,
            pvParameters,
            appCONFIG_DSP_TASK_PRIORITY,
            NULL
        ) != pdPASS
    ) {
        LogError( ( "Failed to create DSP Task\r\n" ) );
    }
}
