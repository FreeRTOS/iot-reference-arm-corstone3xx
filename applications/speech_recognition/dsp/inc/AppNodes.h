/* ----------------------------------------------------------------------
* Project:      CMSIS DSP Library
* Title:        AppNodes.h
* Description:  Application nodes for Example 1
*
* $Date:        29 July 2021
* $Revision:    V1.10.0
*
* Target Processor: Cortex-M and Cortex-A cores
* -------------------------------------------------------------------- */

/*
 * Copyright (C) 2010-2023 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _APPNODES_H_
#define _APPNODES_H_

#include "audio_config.h"
#include "dsp_interfaces.h"
#include "ml_interface.h"
#include <stdio.h>

#if defined( ENABLE_DSP )
    #include "speex/speex_echo.h"
    #include "speex/speex_preprocess.h"
#endif

#include "log_macros.h"
/* Include header that defines log levels. */
#include "logging_levels.h"

extern "C" {
/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "AppNodes"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"
}

template<typename OUT, int outputSize> class MicrophoneSource;

template<int outputSize>
class MicrophoneSource<int16_t, outputSize> : GenericSource<int16_t, outputSize>
{
public:
    MicrophoneSource( FIFOBase<int16_t> &dst,
                      DspAudioSource * dsp ) :
        GenericSource<int16_t, outputSize> ( dst ), mDsp( dsp )
    {
    }

    int run()
    {
        #ifdef AUDIO_VSI
            mDsp->vWaitForNewBuffer();
        #endif

        vSetAudioTimestamp( 1.0 * outputSize / SAMPLE_RATE );

        int16_t * b = this->pxGetWriteBuffer();
        const int16_t * current = mDsp->pxGetCurrentBuffer();

        memcpy( b, current, outputSize * sizeof( int16_t ) );
        return 0;
    }

    DspAudioSource * mDsp;
};

template<typename IN, int inputSize>
class ML;

template<int inputSize>
class ML<int16_t, inputSize> : public GenericSink<int16_t, inputSize>
{
public:
    ML( FIFOBase<int16_t> &src,
        DSPML * dspMLConnection ) : GenericSink<int16_t, inputSize> ( src ),
        mFrameCount( 0 ), dspMLConnection( dspMLConnection )
    {
    }

    int run()
    {
        int16_t * b = this->pxGetReadBuffer();

        /* Due to the sliding window with input of 1 audio second */
        /* we need 3 call to this node to ensure that the input is fully loaded */
        /* with the 296*160 audio samples required. */
        /* Otherwise, some part of the windows will still contain 0 */
        /* So we start our recognition only after enough data has been received. */
        if( mFrameCount < 2 )
        {
            mFrameCount++;
        }
        else
        {
            LogInfo( ( "ML Processing\r\n" ) );
            dspMLConnection->vCopyToDSPBufferFrom( b );
            dspMLConnection->vSwapBuffersAndWakeUpMLThread();
        }

        return 0;
    }

private:

    uint32_t mFrameCount;
    DSPML * dspMLConnection;
};

template<typename IN, int inputSize, typename OUT, int outputSize>
class DSP;

template<int inputSize>
class DSP<int16_t, inputSize, int16_t, inputSize> : public GenericNode<int16_t, inputSize, int16_t, inputSize>
{
public:
    DSP( FIFOBase<int16_t> &src,
         FIFOBase<int16_t> &dst ) :
        GenericNode<int16_t, inputSize, int16_t, inputSize> ( src, dst )
    {
        #if defined( ENABLE_DSP )
            /* Initialize libspeex for the noise reduction processing */
            LogInfo( ( "Init speex\r\n" ) );
            mDen = speex_preprocess_state_init( DSP_BLOCK_SIZE, SAMPLE_RATE );

            if( mDen == NULL )
            {
                LogError( ( "Not enough memory for speex\r\n" ) );
            }
            else
            {
                uint32_t noiseLevel = NOISE_LEVEL_REDUCTION;
                speex_preprocess_ctl( mDen, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, ( void * ) &noiseLevel );
            }
        #endif /* if defined( ENABLE_DSP ) */
    }

    ~DSP()
    {
        #if defined( ENABLE_DSP )
            speex_preprocess_state_destroy( mDen );
        #endif
    }

    int run()
    {
        int16_t * a = this->pxGetReadBuffer();
        int16_t * b = this->pxGetWriteBuffer();

        memcpy( b, a, sizeof( int16_t ) * inputSize );

        /* Noise reduction using libspeex */
        #if defined( ENABLE_DSP )
            if( mDen )
            {
                speex_preprocess_run( mDen, b );
            }
        #endif
        return 0;
    }

private:
    #if defined( ENABLE_DSP )
        SpeexPreprocessState * mDen;
    #endif
};

#endif /* _APPNODES_H_ */
