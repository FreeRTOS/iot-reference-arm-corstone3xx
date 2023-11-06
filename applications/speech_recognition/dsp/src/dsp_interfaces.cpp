/* Copyright 2022-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "FreeRTOS.h"
#include "log_macros.h"
#include "dsp_interfaces.h"
#include "audio_config.h"
#include "model_config.h"
#include "task.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" {
/* Include header that defines log levels. */
#include "logging_levels.h"

/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "DSP"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"
}

static float audio_timestamp = 0.0;

void vSetAudioTimestamp(float timestamp) {
    taskENTER_CRITICAL();
    audio_timestamp = timestamp;
    taskEXIT_CRITICAL();
}

float xGetAudioTimestamp() {
    taskENTER_CRITICAL();
    float timestamp = audio_timestamp;
    taskEXIT_CRITICAL();
    return timestamp;
}

DspAudioSource::DspAudioSource(const int16_t* audiobuffer, size_t block_count ):
        block_count{block_count},
        audiobuffer{audiobuffer}
{

}

const int16_t *DspAudioSource::pxGetCurrentBuffer()
{
#ifndef AUDIO_VSI
    // Update block ID
    current_block = (current_block + 1) % block_count;
#endif

    return(audiobuffer + current_block*(AUDIO_BLOCK_SIZE/2));
}

#ifdef AUDIO_VSI

void DspAudioSource::vWaitForNewBuffer()
{
    xSemaphoreTake( this->semaphore, portMAX_DELAY );
}

void DspAudioSource::prvNewAudioBlockReceived(void* ptr)
{
    auto* self = reinterpret_cast<DspAudioSource*>(ptr);

    // Update block ID
    self->current_block = self->block_under_write;
    self->block_under_write = ((self->block_under_write + 1) % self->block_count);

    if ( self->semaphore != NULL )
    {
        BaseType_t yield = pdFALSE;
        // Wakeup task waiting
        if(xSemaphoreGiveFromISR(self->semaphore, &yield) == pdTRUE)
        {
            portYIELD_FROM_ISR (yield);
        }
    }
};

#endif

static bool prvDspMlLock(SemaphoreHandle_t ml_fifo_mutex)
{
    if ( ml_fifo_mutex == NULL ) {
        return false;
    }

    if ( xSemaphoreTake( ml_fifo_mutex, portMAX_DELAY ) != pdTRUE ) {
        LogError( ( "Failed to acquire ml_fifo_mutex" ) );
        return false;
    }

    return true;
}

static bool prvDspMlUnlock(SemaphoreHandle_t ml_fifo_mutex)
{
    if ( ml_fifo_mutex == NULL ) {
        return false;
    }

    if ( xSemaphoreGive( ml_fifo_mutex ) != pdTRUE ) {
        LogError( ( "Failed to release ml_fifo_mutex" ) );
        return false;
    }

    return true;
}


DSPML::DSPML(size_t bufferLengthInSamples ):nbSamples(bufferLengthInSamples)
{
    bufferA=static_cast<int16_t*>(malloc(bufferLengthInSamples*sizeof(int16_t)));
    bufferB=static_cast<int16_t*>(malloc(bufferLengthInSamples*sizeof(int16_t)));

    dspBuffer = bufferA;
    mlBuffer = bufferB;
}

DSPML::~DSPML()
{
    free(bufferA);
    free(bufferB);
}

void DSPML::vCopyToDSPBufferFrom(int16_t * buf)
{
    prvDspMlLock(mutex);
    memcpy(dspBuffer,buf,sizeof(int16_t)*nbSamples);
    prvDspMlUnlock(mutex);

}

void DSPML::vCopyFromMLBufferInto(int16_t * buf)
{
    prvDspMlLock(mutex);
    memcpy(buf,mlBuffer,sizeof(int16_t)*nbSamples);
    prvDspMlUnlock(mutex);
}

void DSPML::vSwapBuffersAndWakeUpMLThread()
{
    int16_t* tmp;

    prvDspMlLock(mutex);
    tmp=dspBuffer;
    dspBuffer=mlBuffer;
    mlBuffer=tmp;
    prvDspMlUnlock(mutex);

    BaseType_t yield = pdFALSE;
    if (xSemaphoreGiveFromISR(semaphore, &yield) == pdTRUE)
    {
        portYIELD_FROM_ISR (yield);
    }
}

void DSPML::vWaitForDSPData()
{
    xSemaphoreTake( semaphore, portMAX_DELAY );
}
