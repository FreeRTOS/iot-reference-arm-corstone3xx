/* Copyright 2021-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "ml_interface.h"
#include "AudioUtils.hpp"
#include "AppContext.hpp"
#include "BufAttributes.hpp"
#include "demo_config.h"
extern "C" {
#include "events.h"
#ifdef USE_ETHOS
#include "ethosu_driver.h"
#include "ethosu_npu_init.h"
#endif
}
#include "KwsClassifier.hpp"
#include "KwsProcessing.hpp"
#include "KwsResult.hpp"
#include "Labels.hpp"
#include "log_macros.h"
#include "MicroNetKwsMfcc.hpp"
#include "MicroNetKwsModel.hpp"
#include "mqtt_agent_task.h"
#include "TensorFlowLiteMicro.hpp"
#include CMSIS_device_header

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <stdbool.h>
#include <string>
#include <utility>
#include <vector>

extern "C" {
/* Include header that defines log levels. */
#include "logging_levels.h"
/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "ML_Interface"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"
}

/**
 * @brief The topic to subscribe and publish to in the example.
 *
 * The topic name starts with the client identifier to ensure that each demo
 * interacts with a unique topic name.
 */
#define mqttexampleTOPIC    democonfigCLIENT_IDENTIFIER "/ml/inference"

/**
 * @brief The MQTT agent manages the MQTT contexts.  This set the handle to the
 * context used by this demo.
 */
extern MQTTAgentContext_t xGlobalMqttAgentContext;

/**
 * @brief The maximum time for which application waits for an MQTT operation to be complete.
 * This involves receiving an acknowledgment for broker for SUBSCRIBE, UNSUBSCRIBE and non
 * QOS0 publishes.
 */
#define appMQTT_TIMEOUT_MS    ( 5000U )

/**
 * @brief Used to clear bits in a task's notification value.
 */
#define appMAX_UINT32         ( 0xffffffff )

extern EventGroupHandle_t xSystemEvents;
extern QueueHandle_t xMlMqttQueue;

#ifdef AUDIO_VSI

    #include "Driver_SAI.h"

    #define AUDIO_BLOCK_NUM      ( 4 )
    #define AUDIO_BLOCK_SIZE     ( 3200 )
    #define AUDIO_BUFFER_SIZE    ( AUDIO_BLOCK_NUM * AUDIO_BLOCK_SIZE )

/* audio constants */
    __attribute__( ( section( ".bss.NoInit.vsi_audio_buffer" ) ) ) __attribute__( ( aligned( 4 ) ) )
    int16_t shared_audio_buffer[ AUDIO_BUFFER_SIZE / 2 ];
    const int kAudioSampleFrequency = 16000;

    extern ARM_DRIVER_SAI Driver_SAI0;
    extern TaskHandle_t xVsiTaskHandle;

    uint32_t ulVsiEvent;

#else /* !defined(AUDIO_VSI) */

    #include "InputFiles.hpp"

#endif /* AUDIO_VSI */


/* Define tensor arena and declare functions required to access the model */
namespace arm {
namespace app {
uint8_t tensorArena[ ACTIVATION_BUF_SZ ] ACTIVATION_BUF_ATTRIBUTE;
namespace kws {
extern uint8_t * GetModelPointer();
extern size_t GetModelLen();
} /* namespace kws */
} /* namespace app */
} /* namespace arm */

namespace {
typedef struct
{
    ml_processing_state_t state;
} ml_mqtt_msg_t;

/* Import */
using namespace arm::app;

ml_processing_change_handler_t ml_processing_change_handler = NULL;
void * ml_processing_change_ptr = NULL;
const std::array<std::pair<const char *, ml_processing_state_t>, 12> label_to_state{
    std::pair<const char *, ml_processing_state_t>{ "_silence_", ML_SILENCE },
    std::pair<const char *, ml_processing_state_t>{ "_unknown_", ML_UNKNOWN },
    std::pair<const char *, ml_processing_state_t>{ "yes", ML_HEARD_YES },
    std::pair<const char *, ml_processing_state_t>{ "no", ML_HEARD_NO },
    std::pair<const char *, ml_processing_state_t>{ "up", ML_HEARD_UP },
    std::pair<const char *, ml_processing_state_t>{ "down", ML_HEARD_DOWN },
    std::pair<const char *, ml_processing_state_t>{ "left", ML_HEARD_LEFT },
    std::pair<const char *, ml_processing_state_t>{ "right", ML_HEARD_RIGHT },
    std::pair<const char *, ml_processing_state_t>{ "on", ML_HEARD_ON },
    std::pair<const char *, ml_processing_state_t>{ "off", ML_HEARD_OFF },
    std::pair<const char *, ml_processing_state_t>{ "go", ML_HEARD_GO },
    std::pair<const char *, ml_processing_state_t>{ "stop", ML_HEARD_STOP },
};

extern "C" {
static void prvAppPublishCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                          MQTTAgentReturnInfo_t * pxReturnInfo )
{
    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

    if( pxCommandContext->xTaskToNotify != NULL )
    {
        xTaskNotify( pxCommandContext->xTaskToNotify, ( uint32_t ) ( pxReturnInfo->returnCode ), eSetValueWithOverwrite );
    }
}

static void prvMqttSendMessage( const char * message )
{
    static MQTTPublishInfo_t publishInfo = { ( MQTTQoS_t ) 0 };
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };
    static MQTTAgentCommandContext_t xCommandContext = { ( MQTTStatus_t ) 0 };
    MQTTStatus_t mqttStatus = MQTTBadParameter;

    publishInfo.pTopicName = mqttexampleTOPIC;
    publishInfo.topicNameLength = ( uint16_t ) strlen( mqttexampleTOPIC );
    publishInfo.qos = MQTTQoS1;
    publishInfo.pPayload = message;
    publishInfo.payloadLength = strlen( message );

    xCommandContext.xTaskToNotify = xTaskGetCurrentTaskHandle();
    xTaskNotifyStateClear( NULL );

    xCommandParams.blockTimeMs = appMQTT_TIMEOUT_MS;
    xCommandParams.cmdCompleteCallback = prvAppPublishCommandCallback;
    xCommandParams.pCmdCompleteCallbackContext = ( MQTTAgentCommandContext_t * ) &xCommandContext;

    LogInfo( ( "Attempting to publish (%s) to the MQTT topic %s.\r\n", message, mqttexampleTOPIC ) );
    mqttStatus = MQTTAgent_Publish( &xGlobalMqttAgentContext,
                                    &publishInfo,
                                    &xCommandParams );

    /* Wait for command to complete so MQTTSubscribeInfo_t remains in scope for the
     * duration of the command. */
    if( mqttStatus == MQTTSuccess )
    {
        BaseType_t result = xTaskNotifyWait( 0, appMAX_UINT32, NULL, pdMS_TO_TICKS( appMQTT_TIMEOUT_MS ) );

        if( result != pdTRUE )
        {
            mqttStatus = MQTTSendFailed;
        }
        else
        {
            mqttStatus = xCommandContext.xReturnStatus;
        }
    }

    if( mqttStatus != MQTTSuccess )
    {
        LogError( ( "Failed to publish result over MQTT" ) );
    }
    else
    {
        LogInfo( ( "Sent PUBLISH packet to broker %.*s to broker.\n",
                   strlen( mqttexampleTOPIC ),
                   mqttexampleTOPIC ) );
    }
}

static const char * prvGetInferenceResultString( ml_processing_state_t ref_state )
{
    return( label_to_state[ ref_state ].first );
}

void vMlTaskInferenceStart()
{
    if( xSystemEvents == NULL )
    {
        LogError( ( "xSystemEvents is not initialised\r\n" ) );
        return;
    }

    LogInfo( ( "Signal task inference start\r\n" ) );

    ( void ) xEventGroupClearBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_STOP );

    ( void ) xEventGroupSetBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_START );
}

void vMlTaskInferenceStop()
{
    if( xSystemEvents == NULL )
    {
        LogError( ( "xSystemEvents is not initialised\r\n" ) );
        return;
    }

    LogInfo( ( "Signal task inference stop\r\n" ) );

    ( void ) xEventGroupClearBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_START );

    ( void ) xEventGroupSetBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_STOP );
}

void vStartMlTask( void )
{
    if( xTaskCreate( vMlTask,
                     "ML_TASK",
                     appCONFIG_ML_TASK_STACK_SIZE,
                     NULL,
                     appCONFIG_ML_TASK_PRIORITY,
                     NULL ) != pdPASS )
    {
        LogError( ( "Failed to create Ml Task\r\n" ) );
    }
}

void vStartMlMqttTask( void )
{
    if( xTaskCreate( vMlMqttTask,
                     "ML_MQTT",
                     appCONFIG_ML_MQTT_TASK_STACK_SIZE,
                     NULL,
                     appCONFIG_ML_MQTT_TASK_PRIORITY,
                     NULL ) != pdPASS )
    {
        LogError( ( "Failed to create Ml Mqtt Task\r\n" ) );
    }
}
} /* extern "C" */

static void prvSetMlProcessingState( ml_processing_state_t new_state )
{
    /* In this use case, only changes in state are relevant. Additionally, */
    /* this avoids reporting the same keyword detected twice in adjacent, */
    /* overlapping inference windows. */
    static ml_processing_state_t ml_processing_state{ ML_SILENCE };

    if( new_state != ml_processing_state )
    {
        if( xMlMqttQueue == NULL )
        {
            LogError( ( "xMlMqttQueue is not initialised\r\n" ) );
            return;
        }

        const ml_mqtt_msg_t msg = { new_state };

        if( xQueueSendToBack( xMlMqttQueue, ( void * ) &msg, ( TickType_t ) 0 ) != pdPASS )
        {
            LogError( ( "Failed to send message to xMlMqttQueue\r\n" ) );
        }

        ml_processing_state = new_state;

        if( ml_processing_change_handler )
        {
            ml_processing_change_handler_t handler = ml_processing_change_handler;
            void * handler_instance = ml_processing_change_ptr;

            handler( handler_instance, new_state );
        }
    }
}

/* Model */
arm::app::ApplicationContext caseContext;

#ifdef AUDIO_VSI
extern "C" {
/* Audio driver data */
void (* pxOnVsiEvent)( void * );
void * pvVsiContext = nullptr;
}

/* Audio driver callback function for event management */
/* Note: This function cannot contain any logging function */
/* because it would be called in an ISR and it is not permitted */
/* to use logging calls inside the ISR. */
static void prvArmSaiSignalEvent( uint32_t event )
{
    if( xVsiTaskHandle == NULL )
    {
        LogError( ( "VSI Task is not created\r\n" ) );
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ulVsiEvent = event;

    vTaskNotifyGiveFromISR( xVsiTaskHandle, &xHigherPriorityTaskWoken );

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static int prvAudioDrvSetup( void ( * event_handler )( void * ),
                             void * event_handler_ptr )
{
    if( Driver_SAI0.Initialize( prvArmSaiSignalEvent ) != ARM_DRIVER_OK )
    {
        LogError( ( "Failed to set up FVP VSI!\n" ) );
        return -1;
    }

    if( Driver_SAI0.PowerControl( ARM_POWER_FULL ) != ARM_DRIVER_OK )
    {
        LogError( ( "Failed to set the driver to operate with full power!\n" ) );
        return -1;
    }

    if( Driver_SAI0.Control( ARM_SAI_CONTROL_RX, 1, 0 ) != ARM_DRIVER_OK )
    {
        LogError( ( "Failed to enable the VSI receiver!\n" ) );
        return -1;
    }

    if( Driver_SAI0.Control( ARM_SAI_CONFIGURE_RX | ARM_SAI_PROTOCOL_USER | ARM_SAI_DATA_SIZE( 16 ),
                             AUDIO_BLOCK_SIZE,
                             static_cast<uint32_t>( kAudioSampleFrequency ) )
        != ARM_DRIVER_OK )
    {
        LogError( ( "Failed to configure the receiver!\n" ) );
        return -1;
    }

    if( Driver_SAI0.Receive( reinterpret_cast<uint32_t *>( shared_audio_buffer ), AUDIO_BLOCK_NUM ) != ARM_DRIVER_OK )
    {
        LogError( ( "Failed to start receiving the data!\n" ) );
        return -1;
    }

    pxOnVsiEvent = event_handler;
    pvVsiContext = event_handler_ptr;

    return 0;
}

/*
 * Access synchronously data from the audio driver.
 *
 * If data is not available, the audio processing thread goes to sleep until it
 * is woken up by the audio driver.
 */
template<typename T> struct CircularSlidingWindow
{
    CircularSlidingWindow( const T * buffer,
                           size_t block_size,
                           size_t block_count,
                           size_t window_size,
                           size_t stride_size )
        : buffer{ buffer }, block_size{ block_size }, block_count{ block_count }, window_size{ window_size }, stride_size{
                                                                                                                          stride_size }
    {
        /* These are the requirements for the algorithm. */
        assert( stride_size < block_size );
        assert( window_size > stride_size );
        assert( block_size > window_size );
        assert( block_size % stride_size == 0 );
        assert( window_size % stride_size == 0 );
        prvCreateBinarySemaphore( &xSlidingWindowSemaphore );
    }

    ~CircularSlidingWindow()
    {
        if( xSlidingWindowSemaphore != NULL )
        {
            vSemaphoreDelete( xSlidingWindowSemaphore );
        }
    }

    void next( T * dest )
    {
        /* Compute the block that contains the stride */
        size_t first_block = current_stride / prvStridesPerBlock();
        auto last_block = ( ( current_stride * stride_size + window_size - 1 ) / block_size ) % block_count;

        /* Go to sleep if one of the block that contains the next stride is being written. */
        /* If the stride is already loaded, copy it into the destination buffer. */
        while( first_block == prvGetBlockUnderWrite() || last_block == prvGetBlockUnderWrite() )
        {
            if( xSlidingWindowSemaphore != NULL )
            {
                BaseType_t ret = xSemaphoreTake( xSlidingWindowSemaphore, portMAX_DELAY );

                if( ret != pdTRUE )
                {
                    LogError( ( "xSemaphoreTake xSlidingWindowSemaphore failed %ld\r\n", ret ) );
                }
            }
        }

        /* Copy the data into the destination buffer */
        auto begin = buffer + ( current_stride * stride_size );

        /* Memory to copy may not be seqquential if a window span on two blocks. */
        if( last_block < first_block )
        {
            /* Copy end of the buffer */
            auto buffer_end = buffer + ( block_size * block_count );
            std::copy( begin, buffer_end, dest );
            /* Copy remaining from the begining */
            auto offset = buffer_end - begin;
            std::copy( buffer, buffer + ( window_size - offset ), dest + offset );
        }
        else
        {
            std::copy( begin, begin + window_size, dest );
        }

        /* Compute the next stride */
        ++current_stride;
        current_stride %= prvStrideCount();
    }

    /* This is called from ISR */
    static void prvSignalBlockWritten( void * ptr )
    {
        auto * self = reinterpret_cast<CircularSlidingWindow<T> *>( ptr );

        /* Update block ID */
        self->block_under_write = ( ( self->block_under_write + 1 ) % self->block_count );

        if( self->xSlidingWindowSemaphore != NULL )
        {
            BaseType_t yield = pdFALSE;

            /* Wakeup task waiting */
            if( xSemaphoreGiveFromISR( self->xSlidingWindowSemaphore, &yield ) == pdTRUE )
            {
                portYIELD_FROM_ISR( yield );
            }
        }

        /* safe to return as this can signal multiple times before the reader acquires the semaphore. */
    }

    static void prvCreateBinarySemaphore( SemaphoreHandle_t * xSemaphore )
    {
        * xSemaphore = xSemaphoreCreateBinary();

        if( *xSemaphore == NULL )
        {
            LogError( ( "xSemaphoreCreateBinary failed \r\n" ) );
        }

        if( xSemaphoreGive( *xSemaphore ) != pdPASS )
        {
            LogError( ( "xSemaphoreGive xSemaphore failed \r\n" ) );
            vSemaphoreDelete( *xSemaphore );
            * xSemaphore = NULL;
        }
    }

private:
    size_t prvStrideCount() const
    {
        return( ( block_size * block_count ) / stride_size );
    }

    size_t prvStridesPerBlock() const
    {
        return block_size / stride_size;
    }

    size_t prvGetBlockUnderWrite() const
    {
        taskENTER_CRITICAL();
        auto result = block_under_write;
        taskEXIT_CRITICAL();
        return result;
    }

    const T * buffer;
    size_t block_size; /* write size */
    size_t block_count;
    size_t window_size;
    size_t stride_size; /* read size, smaller than write size */
    size_t block_under_write = 0;
    size_t current_stride = 0;
    SemaphoreHandle_t xSlidingWindowSemaphore;
};
#endif /* AUDIO_VSI */

/**
 * @brief           Presents inference results using the data presentation
 *                  object.
 * @param[in]       platform    Reference to the hal platform object.
 * @param[in]       results     Vector of classification results to be displayed.
 * @return          true if successful, false otherwise.
 **/
static bool prvPresentInferenceResult( const arm::app::kws::KwsResult &result );

/**
 * @brief Returns a function to perform feature calculation and populates input tensor data with
 * MFCC data.
 *
 * Input tensor data type check is performed to choose correct MFCC feature data type.
 * If tensor has an integer data type then original features are quantised.
 *
 * Warning: MFCC calculator provided as input must have the same life scope as returned function.
 *
 * @param[in]       mfcc          MFCC feature calculator.
 * @param[in,out]   inputTensor   Input tensor pointer to store calculated features.
 * @param[in]       cacheSize     Size of the feature vectors cache (number of feature vectors).
 * @return          Function to be called providing audio sample and sliding window index.
 */
static std::function<void( std::vector<int16_t> &, int, bool, size_t )> prvGetFeatureCalculator( audio::MicroNetKwsMFCC &mfcc,
                                                                                                 TfLiteTensor * inputTensor,
                                                                                                 size_t cacheSize );

/* Convert labels into ml_processing_state_t */
static ml_processing_state_t prvConvertInferenceResult( const std::string &label )
{
    for( const auto &label_to_state_pair : label_to_state )
    {
        if( label == label_to_state_pair.first )
        {
            return label_to_state_pair.second;
        }
    }

    return ML_UNKNOWN;
}

static void prvProcessAudio( ApplicationContext &ctx )
{
    /* Constants */
    constexpr int minTensorDims =
        static_cast<int>( ( arm::app::MicroNetKwsModel::ms_inputRowsIdx > arm::app::MicroNetKwsModel::ms_inputColsIdx )
                          ? arm::app::MicroNetKwsModel::ms_inputRowsIdx
                          : arm::app::MicroNetKwsModel::ms_inputColsIdx );

    /* Get the global model */
    auto &model = ctx.Get<Model &>( "model" );

    if( !model.IsInited() )
    {
        LogError( ( "Model is not initialised! Terminating processing.\n" ) );
        return;
    }

    const auto frameLength = ctx.Get<int>( "frameLength" );         /* 640 */
    const auto frameStride = ctx.Get<int>( "frameStride" );         /* 320 */
    const auto scoreThreshold = ctx.Get<float>( "scoreThreshold" ); /* 0.8 */

    /* Input and output tensors */
    TfLiteTensor * outputTensor = model.GetOutputTensor( 0 );
    TfLiteTensor * inputTensor = model.GetInputTensor( 0 );

    if( !inputTensor->dims )
    {
        LogError( ( "Invalid input tensor dims\n" ) );
        return;
    }
    else if( inputTensor->dims->size < minTensorDims )
    {
        LogError( ( "Input tensor dimension should be >= %d\n", minTensorDims ) );
        return;
    }

    TfLiteIntArray * inputShape = model.GetInputShape( 0 );
    const uint32_t kNumCols = inputShape->data[ arm::app::MicroNetKwsModel::ms_inputColsIdx ];
    const uint32_t kNumRows = inputShape->data[ arm::app::MicroNetKwsModel::ms_inputRowsIdx ];

    audio::MicroNetKwsMFCC mfcc = audio::MicroNetKwsMFCC( kNumCols, frameLength );
    mfcc.Init();

    /* Deduce the data length required for 1 inference from the network parameters. */
    auto audioDataWindowSize = kNumRows * frameStride + ( frameLength - frameStride ); /* 16000 */
    #ifdef AUDIO_VSI
        auto mfccWindowSize = frameLength;                                             /* 640 */
    #endif /* AUDIO_VSI */
    auto mfccWindowStride = frameStride;                                               /* 320 */

    /* We choose to move by half the window size => for a 1 second window size
     * there is an overlap of 0.5 seconds. */
    auto audioDataStride = audioDataWindowSize / 2;

    /* To have the previously calculated features re-usable, stride must be multiple
     * of MFCC features window stride. */
    if( 0 != audioDataStride % mfccWindowStride )
    {
        /* Reduce the stride. */
        audioDataStride -= audioDataStride % mfccWindowStride; /* 8000 */
    }

    auto nMfccVectorsInAudioStride = audioDataStride / mfccWindowStride; /* 25 */

    /* We expect to be sampling 1 second worth of data at a time.
     * NOTE: This is only used for time stamp calculation. */
    const float secondsPerSample = 1.0 / audio::MicroNetKwsMFCC::ms_defaultSamplingFreq;

    /* Calculate number of the feature vectors in the window overlap region.
     * These feature vectors will be reused.*/
    auto numberOfReusedFeatureVectors = nMfccVectorsInAudioStride;

    /* Construct feature calculation function. */
    auto mfccFeatureCalc = prvGetFeatureCalculator( mfcc, inputTensor, numberOfReusedFeatureVectors );

    if( !mfccFeatureCalc )
    {
        LogError( ( "No feature calculator available" ) );
        return;
    }

    #ifdef AUDIO_VSI
        /* Initialize the sliding window */
        auto circularSlider = CircularSlidingWindow<int16_t>(
            shared_audio_buffer, AUDIO_BLOCK_SIZE / sizeof( int16_t ), AUDIO_BLOCK_NUM, mfccWindowSize, mfccWindowStride );

        /* Initialize the audio driver. It is delayed until that point to avoid drop */
        /* of starting frames. */
        prvAudioDrvSetup( &decltype( circularSlider )::prvSignalBlockWritten, &circularSlider );

        bool first_iteration = true;
        auto mfccAudioData = std::vector<int16_t>( mfccWindowSize, 0 );
        size_t audio_index = 0;
    #endif /* AUDIO_VSI */

    while( true )
    {
        #ifdef AUDIO_VSI
            LogInfo( ( "Running inference as audio input is received from the Virtual Streaming Interface\r\n" ) );

            while( true )
            {
                EventBits_t flags = xEventGroupWaitBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_STOP, pdTRUE, pdFAIL, 10 );

                if( flags & EVENT_MASK_ML_STOP )
                {
                    /* jump out to outer loop */
                    LogInfo( ( "Stopping audio processing\r\n" ) );
                    break;
                }

                /* The first window does not have cache ready. */
                bool useCache = first_iteration == false && numberOfReusedFeatureVectors > 0;
                size_t stride_index = 0;

                while( stride_index < ( audioDataWindowSize / mfccWindowStride ) )
                {
                    if( !useCache || ( stride_index >= numberOfReusedFeatureVectors ) )
                    {
                        circularSlider.next( mfccAudioData.data() );
                    }

                    /* Compute features for this window and write them to input tensor. */
                    mfccFeatureCalc( mfccAudioData, stride_index, useCache, nMfccVectorsInAudioStride );
                    ++stride_index;
                }

                /* Run inference over this audio clip sliding window. */
                if( !model.RunInference() )
                {
                    LogError( ( "Failed to run inference" ) );
                    return;
                }

                std::vector<ClassificationResult> classificationResult;
                auto &classifier = ctx.Get<KwsClassifier &>( "classifier" );
                classifier.GetClassificationResults(
                    outputTensor, classificationResult, ctx.Get<std::vector<std::string> &>( "labels" ), 1, true );

                auto result = kws::KwsResult(
                    classificationResult, audio_index * secondsPerSample * audioDataStride, audio_index, scoreThreshold );

                if( result.m_resultVec.empty() )
                {
                    prvSetMlProcessingState( ML_UNKNOWN );
                }
                else
                {
                    prvSetMlProcessingState( prvConvertInferenceResult( result.m_resultVec[ 0 ].m_label ) );
                }

                if( prvPresentInferenceResult( result ) != true )
                {
                    LogError( ( "Failed to present inference result" ) );
                    return;
                }

                first_iteration = false;
                ++audio_index;
            } /* while (true) */
        #else /* !defined(AUDIO_VSI) */
            LogInfo( ( "Running inference on an audio clip in local memory\r\n" ) );

            const uint32_t numMfccFeatures = inputShape->data[ MicroNetKwsModel::ms_inputColsIdx ];
            const uint32_t numMfccFrames = inputShape->data[ arm::app::MicroNetKwsModel::ms_inputRowsIdx ];

            KwsPreProcess preProcess = KwsPreProcess(
                inputTensor, numMfccFeatures, numMfccFrames, ctx.Get<int>( "frameLength" ), ctx.Get<int>( "frameStride" ) );

            std::vector<ClassificationResult> singleInfResult;
            KwsPostProcess postProcess = KwsPostProcess( outputTensor,
                                                         ctx.Get<arm::app::KwsClassifier &>( "classifier" ),
                                                         ctx.Get<std::vector<std::string> &>( "labels" ),
                                                         singleInfResult );

            /* Creating a sliding window through the whole audio clip. */
            auto audioDataSlider = audio::SlidingWindow<const int16_t>(
                GetAudioArray( 0 ), GetAudioArraySize( 0 ), preProcess.m_audioDataWindowSize, preProcess.m_audioDataStride );

            /* Start sliding through audio clip. */
            while( audioDataSlider.HasNext() )
            {
                EventBits_t flags = xEventGroupWaitBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_STOP, pdTRUE, pdFAIL, 10 );

                if( flags & EVENT_MASK_ML_STOP )
                {
                    /* Jump out to the outer loop, which may restart inference on an EVENT_MASK_ML_START signal */
                    LogInfo( ( "Inference stopped by a signal.\r\n" ) );
                    break;
                }

                const int16_t * inferenceWindow = audioDataSlider.Next();

                if( !preProcess.DoPreProcess( inferenceWindow, audioDataSlider.Index() ) )
                {
                    LogError( ( "Pre-processing failed." ) );
                    return;
                }

                if( !model.RunInference() )
                {
                    LogError( ( "Inference failed." ) );
                    return;
                }

                if( !postProcess.DoPostProcess() )
                {
                    LogError( ( "Post-processing failed." ) );
                    return;
                }

                auto result = kws::KwsResult( singleInfResult,
                                              audioDataSlider.Index() * secondsPerSample * preProcess.m_audioDataStride,
                                              audioDataSlider.Index(),
                                              scoreThreshold );

                if( result.m_resultVec.empty() )
                {
                    prvSetMlProcessingState( ML_UNKNOWN );
                }
                else
                {
                    prvSetMlProcessingState( prvConvertInferenceResult( result.m_resultVec[ 0 ].m_label ) );
                }

                if( prvPresentInferenceResult( result ) != true )
                {
                    LogError( ( "Failed to present inference result" ) );
                    return;
                }
            } /* while (audioDataSlider.HasNext()) */
        #endif /* AUDIO_VSI */

        EventBits_t flags = xEventGroupWaitBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_START, pdTRUE, pdFAIL, portMAX_DELAY );

        if( flags & EVENT_MASK_ML_START )
        {
            LogInfo( ( "Restarting audio processing %u\r\n", flags ) );
        }
    } /* while (true) */
}

static bool prvPresentInferenceResult( const arm::app::kws::KwsResult &result )
{
    /* Display each result */
    if( result.m_resultVec.empty() )
    {
        LogInfo( ( "For timestamp: %f (inference #: %" PRIu32 "); label: %s; threshold: %f\n",
                   ( double ) result.m_timeStamp,
                   result.m_inferenceNumber,
                   "<none>",
                   0. ) );
    }
    else
    {
        for( uint32_t i = 0; i < result.m_resultVec.size(); ++i )
        {
            LogInfo( ( "For timestamp: %f (inference #: %" PRIu32 "); label: %s, score: %f; threshold: %f\n",
                       ( double ) result.m_timeStamp,
                       result.m_inferenceNumber,
                       result.m_resultVec[ i ].m_label.c_str(),
                       result.m_resultVec[ i ].m_normalisedVal,
                       ( double ) result.m_threshold ) );
        }
    }

    return true;
}

/**
 * @brief Generic feature calculator factory.
 *
 * Returns lambda function to compute features using features cache.
 * Real features math is done by a lambda function provided as a parameter.
 * Features are written to input tensor memory.
 *
 * @tparam T                Feature vector type.
 * @param[in] inputTensor   Model input tensor pointer.
 * @param[in] cacheSize     Number of feature vectors to cache. Defined by the sliding window overlap.
 * @param[in] compute       Features calculator function.
 * @return                  Lambda function to compute features.
 */
template<class T>
std::function<void( std::vector<int16_t> &, size_t, bool, size_t )> FeatureCalc( TfLiteTensor * inputTensor,
                                                                                 size_t cacheSize,
                                                                                 std::function<std::vector<T>( std::vector<int16_t> & )> compute )
{
    /* Feature cache to be captured by lambda function. */
    static std::vector<std::vector<T> > featureCache = std::vector<std::vector<T> >( cacheSize );

    return [ = ]( std::vector<int16_t> &audioDataWindow, size_t index, bool useCache, size_t featuresOverlapIndex ) {
               T * tensorData = tflite::GetTensorData<T>( inputTensor );
               std::vector<T> features;

               /* Reuse features from cache if cache is ready and sliding windows overlap.
                * Overlap is in the beginning of sliding window with a size of a feature cache. */
               if( useCache && ( index < featureCache.size() ) )
               {
                   features = std::move( featureCache[ index ] );
               }
               else
               {
                   features = std::move( compute( audioDataWindow ) );
               }

               auto size = features.size();
               auto sizeBytes = sizeof( T ) * size;
               std::memcpy( tensorData + ( index * size ), features.data(), sizeBytes );

               /* Start renewing cache as soon iteration goes out of the windows overlap. */
               if( index >= featuresOverlapIndex )
               {
                   featureCache[ index - featuresOverlapIndex ] = std::move( features );
               }
    };
}

template std::function<void( std::vector<int16_t> &, size_t, bool, size_t )> FeatureCalc<int8_t>( TfLiteTensor * inputTensor,
                                                                                                  size_t cacheSize,
                                                                                                  std::function<std::vector<int8_t>( std::vector<int16_t> & )> compute );

template std::function<void( std::vector<int16_t> &, size_t, bool, size_t )> FeatureCalc<uint8_t>( TfLiteTensor * inputTensor,
                                                                                                   size_t cacheSize,
                                                                                                   std::function<std::vector<uint8_t>( std::vector<int16_t> & )> compute );

template std::function<void( std::vector<int16_t> &, size_t, bool, size_t )> FeatureCalc<int16_t>( TfLiteTensor * inputTensor,
                                                                                                   size_t cacheSize,
                                                                                                   std::function<std::vector<int16_t>( std::vector<int16_t> & )> compute );

template std::function<void( std::vector<int16_t> &, size_t, bool, size_t )> FeatureCalc<float>( TfLiteTensor * inputTensor,
                                                                                                 size_t cacheSize,
                                                                                                 std::function<std::vector<float>( std::vector<int16_t> & )> compute );

static std::function<void( std::vector<int16_t> &, int, bool, size_t )> prvGetFeatureCalculator( audio::MicroNetKwsMFCC &mfcc,
                                                                                                 TfLiteTensor * inputTensor,
                                                                                                 size_t cacheSize )
{
    std::function<void( std::vector<int16_t> &, size_t, bool, size_t )> mfccFeatureCalc;
    TfLiteQuantization quant = inputTensor->quantization;

    if( kTfLiteAffineQuantization == quant.type )
    {
        auto * quantParams = static_cast<TfLiteAffineQuantization *>( quant.params );
        const float quantScale = quantParams->scale->data[ 0 ];
        const int quantOffset = quantParams->zero_point->data[ 0 ];

        switch( inputTensor->type )
        {
            case kTfLiteInt8:
                mfccFeatureCalc =
                    FeatureCalc<int8_t>( inputTensor, cacheSize, [ =, &mfcc ]( std::vector<int16_t> &audioDataWindow ) {
                    return mfcc.MfccComputeQuant<int8_t>( audioDataWindow, quantScale, quantOffset );
                } );
                break;

            case kTfLiteUInt8:
                mfccFeatureCalc =
                    FeatureCalc<uint8_t>( inputTensor, cacheSize, [ =, &mfcc ]( std::vector<int16_t> &audioDataWindow ) {
                    return mfcc.MfccComputeQuant<uint8_t>( audioDataWindow, quantScale, quantOffset );
                } );
                break;

            case kTfLiteInt16:
                mfccFeatureCalc =
                    FeatureCalc<int16_t>( inputTensor, cacheSize, [ =, &mfcc ]( std::vector<int16_t> &audioDataWindow ) {
                    return mfcc.MfccComputeQuant<int16_t>( audioDataWindow, quantScale, quantOffset );
                } );
                break;

            default:
                LogError( ( "Tensor type %s not supported\n", TfLiteTypeGetName( inputTensor->type ) ) );
        }
    }
    else
    {
        mfccFeatureCalc = FeatureCalc<float>( inputTensor, cacheSize, [ &mfcc ]( std::vector<int16_t> &audioDataWindow ) {
                return mfcc.MfccCompute( audioDataWindow );
            } );
    }

    return mfccFeatureCalc;
}
} /* anonymous namespace */

#ifdef USE_ETHOS
    extern struct ethosu_driver ethosu_drv; /* Default Ethos-U55 device driver */

/**
 * @brief   Initialises the Arm Ethos-U55 NPU
 * @return  0 if successful, error code otherwise
 **/
    static int prvArmNpuInit( void );

    static int prvArmNpuInit( void )
    {
        int err = 0;

        SCB_EnableICache();
        SCB_EnableDCache();

        #if defined( ETHOS_U_NPU_TIMING_ADAPTER_ENABLED )

            /* If the platform has timing adapter blocks along with Ethos-U core
             * block, initialise them here. */
            if( 0 != ( err = arm_ethosu_timing_adapter_init() ) )
            {
                LogError( ( "Failed to init timing adapter\n" ) );
                return err;
            }
        #endif /* ETHOS_U_NPU_TIMING_ADAPTER_ENABLED */

        /* Initialize the ethos NPU */
        if( 0 != ( err = arm_ethosu_npu_init() ) )
        {
            LogError( ( "Failed to init arm npu\n" ) );
            return err;
        }

        LogInfo( ( "Ethos-U55 device initialised\n" ) );

        /* Get Ethos-U55 version */
        struct ethosu_driver_version driver_version;
        struct ethosu_hw_info hw_info;

        ethosu_get_driver_version( &driver_version );
        ethosu_get_hw_info( &ethosu_drv, &hw_info );

        LogInfo( ( "Ethos-U version info:\n" ) );
        LogInfo( ( "\tArch:       v%" PRIu32 ".%" PRIu32 ".%" PRIu32 "\n",
                   hw_info.version.arch_major_rev,
                   hw_info.version.arch_minor_rev,
                   hw_info.version.arch_patch_rev ) );
        LogInfo( ( "\tDriver:     v%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                   driver_version.major,
                   driver_version.minor,
                   driver_version.patch ) );
        LogInfo( ( "\tMACs/cc:    %" PRIu32 "\n", ( uint32_t ) ( 1 << hw_info.cfg.macs_per_cc ) ) );
        LogInfo( ( "\tCmd stream: v%" PRIu32 "\n", hw_info.cfg.cmd_stream_version ) );

        return 0;
    }
#endif /* USE_ETHOS */

extern "C" {
void vRegisterMlProcessingChangeCb( ml_processing_change_handler_t handler,
                                    void * ctx )
{
    ml_processing_change_handler = handler;
    ml_processing_change_ptr = ctx;
}

static int prvMlInterfaceInit()
{
    static arm::app::MicroNetKwsModel model; /* Model wrapper object. */

    #ifdef USE_ETHOS
        /* Initialize the ethos U55 */
        if( prvArmNpuInit() != 0 )
        {
            LogError( ( "Failed to arm npu\n" ) );
            return -1;
        }
    #endif /* USE_ETHOS */

    /* Load the model. */
    if( !model.Init( ::arm::app::tensorArena,
                     sizeof( ::arm::app::tensorArena ),
                     ::arm::app::kws::GetModelPointer(),
                     ::arm::app::kws::GetModelLen() ) )
    {
        LogError( ( "Failed to initialise model\n" ) );
        return -1;
    }

    /* Instantiate application context. */
    caseContext.Set<arm::app::Model &>( "model", model );
    caseContext.Set<int>( "frameLength", arm::app::kws::g_FrameLength );
    caseContext.Set<int>( "frameStride", arm::app::kws::g_FrameStride );
    caseContext.Set<float>( "scoreThreshold", arm::app::kws::g_ScoreThreshold ); /* Normalised score threshold. */

    static KwsClassifier classifier;                                             /* classifier wrapper object. */
    caseContext.Set<arm::app::KwsClassifier &>( "classifier", classifier );

    static std::vector<std::string> labels;
    GetLabelsVector( labels );

    caseContext.Set<const std::vector<std::string> &>( "labels", labels );

    PrintTensorFlowVersion();
    LogInfo( ( "*** ML interface initialised\r\n" ) );
    return 0;
}

void vMlTask( void * arg )
{
    ( void ) arg;

    EventBits_t flags = xEventGroupWaitBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_START, pdTRUE, pdFAIL, portMAX_DELAY );

    if( flags & EVENT_MASK_ML_START )
    {
        LogInfo( ( "Initial start of audio processing\r\n" ) );
    }

    if( prvMlInterfaceInit() < 0 )
    {
        LogError( ( "prvMlInterfaceInit failed\r\n" ) );
        return;
    }

    prvProcessAudio( caseContext );
}

void vMlMqttTask( void * arg )
{
    ( void ) arg;

    while( 1 )
    {
        ml_mqtt_msg_t msg;

        if( xQueueReceive( xMlMqttQueue, &msg, portMAX_DELAY ) == pdPASS )
        {
            prvMqttSendMessage( prvGetInferenceResultString( msg.state ) );
        }
        else
        {
            LogError( ( "xQueueReceive Ml Mqtt Queue failed\r\n" ) );
            return;
        }
    }
}
} /* extern "C" */
