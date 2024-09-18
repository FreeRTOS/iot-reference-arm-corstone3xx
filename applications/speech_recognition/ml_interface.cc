/* Copyright 2021-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "ml_interface.h"
#include "AsrClassifier.hpp"
#include "AsrResult.hpp"
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
#include "Labels.hpp"
#include "OutputDecode.hpp"
#include "mqtt_agent_task.h"
#include "TensorFlowLiteMicro.hpp"
#include "Wav2LetterMfcc.hpp"
#include "Wav2LetterModel.hpp"
#include "Wav2LetterPostprocess.hpp"
#include "Wav2LetterPreprocess.hpp"
#include CMSIS_device_header
#include "dsp_interfaces.h"
#include "model_config.h"
#include "log_macros.h"

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
    #define LIBRARY_LOG_NAME     "ML_IFC"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"
}

#include "audio_config.h"
#include "app_config.h"

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

/* Define tensor arena and declare functions required to access the model */
namespace arm {
namespace app {
uint8_t tensorArena[ ACTIVATION_BUF_SZ ] ACTIVATION_BUF_ATTRIBUTE;
namespace asr {
extern uint8_t * GetModelPointer();
extern size_t GetModelLen();
} /* namespace asr */
} /* namespace app */
} /* namespace arm */

namespace {
typedef struct
{
    char * result;
} ml_mqtt_msg_t;

/* Import */
using namespace arm::app;

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

void vMlTaskInferenceStart( void )
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

void vMlTaskInferenceStop( void )
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

void vStartMlTask( void * pvParameters )
{
    if(
        xTaskCreate(
            vMlTask,
            "ML_TASK",
            appCONFIG_ML_TASK_STACK_SIZE,
            pvParameters,
            appCONFIG_ML_TASK_PRIORITY,
            NULL
            ) != pdPASS
        )
    {
        LogError( ( "Failed to create ML Task\r\n" ) );
    }
}

void vStartMlMqttTask( void )
{
    if(
        xTaskCreate(
            vMlMqttTask,
            "ML_MQTT",
            appCONFIG_ML_MQTT_TASK_STACK_SIZE,
            NULL,
            appCONFIG_ML_MQTT_TASK_PRIORITY,
            NULL
            ) != pdPASS
        )
    {
        LogError( ( "Failed to create ML Mqtt Task\r\n" ) );
    }
}
} /* extern "C" { */

static void prvSetMlProcessingstate( const char * inference_result )
{
    size_t msg_len = strlen( inference_result ) + 1;
    char * msg_result = reinterpret_cast<char *>( malloc( msg_len ) );

    if( msg_result )
    {
        if( xMlMqttQueue == NULL )
        {
            LogError( ( "xMlMqttQueue is not initialised\r\n" ) );
            free( reinterpret_cast<void *>( msg_result ) );
            return;
        }

        memcpy( msg_result, inference_result, msg_len );
        const ml_mqtt_msg_t msg = { msg_result };

        if( xQueueSendToBack( xMlMqttQueue, ( void * ) &msg, ( TickType_t ) 0 ) != pdTRUE )
        {
            LogError( ( "Failed to send message to xMlMqttQueue\r\n" ) );
            free( reinterpret_cast<void *>( msg_result ) );
        }
    }
    else
    {
        LogWarn( ( "Failed to send ml processing inference_result (alloc failure)" ) );
    }
}

/* Model */
arm::app::ApplicationContext caseContext;

/**
 * @brief           Presents inference results using the data presentation
 *                  object.
 * @param[in]       platform    Reference to the hal platform object.
 * @param[in]       results     Vector of classification results to be displayed.
 * @return          true if successful, false otherwise.
 **/
static bool prvPresentInferenceResult( const std::vector<arm::app::asr::AsrResult> &results );

static void prvProcessAudio( ApplicationContext &ctx,
                             DSPML * dspMLConnection )
{
    /* Get model reference. */
    auto &model = ctx.Get<Model &>( "model" );

    if( !model.IsInited() )
    {
        LogError( ( "Model is not initialised! Terminating processing.\n" ) );
        return;
    }

    /* Get score threshold to be applied for the classifier (post-inference). */
    auto scoreThreshold = ctx.Get<float>( "scoreThreshold" );

    /* Get tensors. Dimensions of the tensor should have been verified by
     * the callee. */
    TfLiteTensor * inputTensor = model.GetInputTensor( 0 );
    TfLiteTensor * outputTensor = model.GetOutputTensor( 0 );
    TfLiteIntArray * inputShape = model.GetInputShape( 0 );

    /* Populate MFCC related parameters. */
    auto mfccFrameLen = ctx.Get<uint32_t>( "frameLength" );
    auto mfccFrameStride = ctx.Get<uint32_t>( "frameStride" );

    /* Populate ASR inference context and inner lengths for input. */
    auto inputCtxLen = ctx.Get<uint32_t>( "ctxLen" );

    /* Get pre/post-processing objects. */
    AsrPreProcess preProcess = AsrPreProcess( inputTensor,
                                              Wav2LetterModel::ms_numMfccFeatures,
                                              inputShape->data[ Wav2LetterModel::ms_inputRowsIdx ],
                                              mfccFrameLen,
                                              mfccFrameStride );

    std::vector<ClassificationResult> singleInfResult;
    const uint32_t outputCtxLen = AsrPostProcess::GetOutputContextLen( model, inputCtxLen );
    AsrPostProcess postProcess = AsrPostProcess( outputTensor,
                                                 ctx.Get<AsrClassifier &>( "classifier" ),
                                                 ctx.Get<std::vector<std::string> &>( "labels" ),
                                                 singleInfResult,
                                                 outputCtxLen,
                                                 Wav2LetterModel::ms_blankTokenIdx,
                                                 Wav2LetterModel::ms_outputRowsIdx );

    const uint32_t inputRows = inputTensor->dims->data[ arm::app::Wav2LetterModel::ms_inputRowsIdx ];
    /* Audio data stride corresponds to inputInnerLen feature vectors. */
    const uint32_t audioParamsWinLen = inputRows * mfccFrameStride;

    auto inferenceWindow = std::vector<int16_t>( audioParamsWinLen, 0 );
    size_t inferenceWindowLen = audioParamsWinLen;

    /* Start processing audio data as it arrive */
    uint32_t inferenceIndex = 0;
    /* We do inference on 2 audio segments before reporting a result */
    /* We do not have the concept of audio clip in a streaming application */
    /* so we need to decide when a sentenced is finished to start a recognition. */
    /* It was arbitrarily chosen to be 2 inferences. */
    /* In a real app, a voice activity detector would probably be used */
    /* to detect a long silence between 2 sentences. */
    const uint32_t maxNbInference = 2;
    std::vector<arm::app::asr::AsrResult> results;

    while( true )
    {
        while( true )
        {
            EventBits_t flags = xEventGroupWaitBits(
                xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_STOP, pdTRUE, pdFAIL, 300
                );

            if( flags & EVENT_MASK_ML_STOP )
            {
                LogInfo( ( "Stopping audio processing\r\n" ) );
                break;
            }

            /* Wait for the DSP task signal to start the recognition */
            dspMLConnection->vWaitForDSPData();

            int16_t * p = inferenceWindow.data();
            dspMLConnection->vCopyFromMLBufferInto( p );

            /* This timestamp is corresponding to the time when */
            /* inference is starting and not to the time of the */
            /* beginning of the audio segment used for this inference. */
            float currentTimeStamp = xGetAudioTimestamp();
            LogInfo( ( "Inference %i/%i\n", inferenceIndex + 1, maxNbInference ) );

            /* Run the pre-processing, inference and post-processing. */
            if( !preProcess.DoPreProcess( inferenceWindow.data(), inferenceWindowLen ) )
            {
                LogError( ( "Pre-processing failed." ) );
            }

            #ifdef AUDIO_VSI
                LogInfo( ( "Start running inference on audio input from the Virtual Streaming Interface\r\n" ) );
            #else
                LogInfo( ( "Start running inference on an audio clip in local memory\r\n" ) );
            #endif

            /* Run inference over this audio clip sliding window. */
            if( !model.RunInference() )
            {
                LogError( ( "Failed to run inference" ) );
                return;
            }

            LogDebug( ( "Doing post processing\n" ) );

            /* Post processing needs to know if we are on the last audio window. */
            /* postProcess.m_lastIteration = !audioDataSlider.HasNext(); */
            if( !postProcess.DoPostProcess() )
            {
                LogError( ( "Post-processing failed." ) );
            }

            LogInfo( ( "Inference done\n" ) );

            std::vector<ClassificationResult> classificationResult;
            auto &classifier = ctx.Get<AsrClassifier &>( "classifier" );
            classifier.GetClassificationResults(
                outputTensor,
                classificationResult,
                ctx.Get<std::vector<std::string> &>( "labels" ),
                1,
                true
                );

            auto result = asr::AsrResult(
                classificationResult,
                currentTimeStamp,
                inferenceIndex,
                scoreThreshold
                );

            results.emplace_back( result );

            inferenceIndex = inferenceIndex + 1;

            if( inferenceIndex == maxNbInference )
            {
                inferenceIndex = 0;

                ctx.Set<std::vector<arm::app::asr::AsrResult> >( "results", results );

                if( !prvPresentInferenceResult( results ) )
                {
                    return;
                }

                results.clear();
            }

            /* Inference loop */
        } /* while (true) */

        EventBits_t flags = xEventGroupWaitBits(
            xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_START, pdTRUE, pdFAIL, portMAX_DELAY
            );

        if( flags & EVENT_MASK_ML_START )
        {
            LogInfo( ( "Restarting audio processing %u\r\n", flags ) );
        }
    } /* while (true) */
}

static bool prvPresentInferenceResult( const std::vector<arm::app::asr::AsrResult> &results )
{
    LogInfo( ( "Final results:\n" ) );
    LogInfo( ( "Total number of inferences: %zu\n", results.size() ) );
    /* Results from multiple inferences should be combined before processing. */
    std::vector<arm::app::ClassificationResult> combinedResults;

    for( auto &result : results )
    {
        combinedResults.insert( combinedResults.end(), result.m_resultVec.begin(), result.m_resultVec.end() );
    }

    /* Get each inference result string using the decoder. */
    for( const auto &result : results )
    {
        std::string infResultStr = audio::asr::DecodeOutput( result.m_resultVec );

        LogInfo( ( "For timestamp: %f (inference #: %" PRIu32 "); label: %s\r\n",
                   ( double ) result.m_timeStamp,
                   result.m_inferenceNumber,
                   infResultStr.c_str() ) );
    }

    /* Get the decoded result for the combined result. */
    std::string finalResultStr = audio::asr::DecodeOutput( combinedResults );

    LogInfo( ( "Complete recognition: %s\n", finalResultStr.c_str() ) );

    /* Send the inference result */
    prvSetMlProcessingstate( finalResultStr.c_str() );

    return true;
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
static int prvMlInterfaceInit( void )
{
    static arm::app::Wav2LetterModel model;    /* Model wrapper object. */
    static arm::app::AsrClassifier classifier; /* Classifier wrapper object. */
    static std::vector<std::string> labels;

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
                     ::arm::app::asr::GetModelPointer(),
                     ::arm::app::asr::GetModelLen() ) )
    {
        LogError( ( "Failed to initialise model\n" ) );
        return -1;
    }

    /* Initialise post-processing. */
    GetLabelsVector( labels );

    /* Instantiate application context. */
    caseContext.Set<arm::app::Model &>( "model", model );
    caseContext.Set<uint32_t>( "frameLength", g_FrameLength );
    caseContext.Set<uint32_t>( "frameStride", g_FrameStride );
    caseContext.Set<uint32_t>( "ctxLen", g_ctxLen );

    caseContext.Set<float>( "scoreThreshold", g_ScoreThreshold ); /* Normalised score threshold. */

    caseContext.Set<const std::vector<std::string> &>( "labels", labels );
    caseContext.Set<arm::app::AsrClassifier &>( "classifier", classifier );

    PrintTensorFlowVersion();
    LogInfo( ( "*** ML interface initialised\r\n" ) );
    return 0;
}

void vMlTask( void * pvParameters )
{
    LogInfo( ( "ML Task start\r\n" ) );
    DSPML * dspMLConnection = static_cast<DSPML *>( pvParameters );

    EventBits_t flags = xEventGroupWaitBits(
        xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_START, pdTRUE, pdFAIL, portMAX_DELAY
        );

    if( flags & EVENT_MASK_ML_START )
    {
        LogInfo( ( "Initial start of audio processing\r\n" ) );
    }

    if( prvMlInterfaceInit() < 0 )
    {
        LogError( ( "prvMlInterfaceInit failed\r\n" ) );
        return;
    }

    prvProcessAudio( caseContext, dspMLConnection );
}

void vMlMqttTask( void * pvParameters )
{
    ( void ) pvParameters;

    while( 1 )
    {
        ml_mqtt_msg_t msg;

        if( xQueueReceive( xMlMqttQueue, &msg, portMAX_DELAY ) == pdTRUE )
        {
            prvMqttSendMessage( msg.result );
            free( reinterpret_cast<void *>( msg.result ) );
        }
        else
        {
            LogError( ( "xQueueReceive ML MQTT msg queue failed\r\n" ) );
        }
    }
}
} /* extern "C" */
