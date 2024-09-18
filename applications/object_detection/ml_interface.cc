/* Copyright 2021-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "ml_interface.h"
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
#include "DetectorPostProcessing.hpp"
#include "DetectorPreProcessing.hpp"
#include "mqtt_agent_task.h"
#include "TensorFlowLiteMicro.hpp"
#include "YoloFastestModel.hpp"
#include CMSIS_device_header
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

#include "isp_config.h"
}

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
uint8_t ucTensorArena[ ACTIVATION_BUF_SZ ] ACTIVATION_BUF_ATTRIBUTE;
namespace object_detection {
extern uint8_t * GetModelPointer();
extern size_t GetModelLen();
} /* namespace object_detection */
} /* namespace app */
} /* namespace arm */

namespace {
/* Import */
using namespace arm::app;

/* Model */
arm::app::ApplicationContext xCaseContext;

static int prvProcessImage( ApplicationContext &xApplicationContext,
                            const uint8_t * pucImage,
                            struct DetectRegion_t * pxCResults,
                            uint32_t * pulResultsNum );

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

static void prvMqttSendMessage( const char * pcMessage )
{
    static MQTTPublishInfo_t xPublishInfo = { ( MQTTQoS_t ) 0 };
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };
    static MQTTAgentCommandContext_t xCommandContext = { ( MQTTStatus_t ) 0 };
    MQTTStatus_t xMqttStatus = MQTTBadParameter;

    xPublishInfo.pTopicName = mqttexampleTOPIC;
    xPublishInfo.topicNameLength = ( uint16_t ) strlen( mqttexampleTOPIC );
    xPublishInfo.qos = MQTTQoS1;
    xPublishInfo.pPayload = pcMessage;
    xPublishInfo.payloadLength = strlen( pcMessage );

    xCommandContext.xTaskToNotify = xTaskGetCurrentTaskHandle();
    xTaskNotifyStateClear( NULL );

    xCommandParams.blockTimeMs = appMQTT_TIMEOUT_MS;
    xCommandParams.cmdCompleteCallback = prvAppPublishCommandCallback;
    xCommandParams.pCmdCompleteCallbackContext = ( MQTTAgentCommandContext_t * ) &xCommandContext;

    LogInfo( ( "Attempting to publish (%s) to the MQTT topic %s.\r\n", pcMessage, mqttexampleTOPIC ) );
    xMqttStatus = MQTTAgent_Publish( &xGlobalMqttAgentContext,
                                     &xPublishInfo,
                                     &xCommandParams );

    /* Wait for command to complete so MQTTSubscribeInfo_t remains in scope for the
     * duration of the command. */
    if( xMqttStatus == MQTTSuccess )
    {
        BaseType_t xResult = xTaskNotifyWait( 0, appMAX_UINT32, NULL, pdMS_TO_TICKS( appMQTT_TIMEOUT_MS ) );

        if( xResult != pdTRUE )
        {
            xMqttStatus = MQTTSendFailed;
        }
        else
        {
            xMqttStatus = xCommandContext.xReturnStatus;
        }
    }

    if( xMqttStatus != MQTTSuccess )
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

int32_t lMLRunInference( const uint8_t * pucImg,
                         struct DetectRegion_t * pxResults,
                         uint32_t * pulResultsNum )
{
    prvProcessImage( xCaseContext, pucImg, pxResults, pulResultsNum );
    return 0;
}
} /* extern "C" { */

static void prvSetMlProcessingstate( const char * pcInferenceResult )
{
    size_t xMsgLen = strlen( pcInferenceResult ) + 1;
    char * pcMsgResult = reinterpret_cast<char *>( malloc( xMsgLen ) );

    if( pcMsgResult )
    {
        if( xMlMqttQueue == NULL )
        {
            LogError( ( "xMlMqttQueue is not initialised\r\n" ) );
            free( reinterpret_cast<void *>( pcMsgResult ) );
            return;
        }

        memcpy( pcMsgResult, pcInferenceResult, xMsgLen );
        const MLMqttMsg_t msg = { pcMsgResult };

        if( xQueueSendToBack( xMlMqttQueue, ( void * ) &msg, ( TickType_t ) 0 ) != pdTRUE )
        {
            LogError( ( "Failed to send message to xMlMqttQueue\r\n" ) );
            free( reinterpret_cast<void *>( pcMsgResult ) );
        }
    }
    else
    {
        LogWarn( ( "Failed to send ml processing inference_result (alloc failure)" ) );
    }
}

/**
 * @brief           Presents inference results using the data presentation
 *                  object.
 * @param[in]       platform    Reference to the hal platform object.
 * @param[in]       results     Vector of classification results to be displayed.
 * @return          true if successful, false otherwise.
 **/
static bool prvPresentInferenceResult( const std::vector<object_detection::DetectionResult> &xResults );

static int prvProcessImage( ApplicationContext &xApplicationContext,
                            const uint8_t * pucImage,
                            struct DetectRegion_t * pxCResults,
                            uint32_t * pulResultsNum )
{
    /* Get the global model */
    auto &xModel = xApplicationContext.Get<Model &>( "model" );

    if( !xModel.IsInited() )
    {
        LogError( ( "Model is not initialised! Terminating processing.\n" ) );
        return -1;
    }

    TfLiteTensor * xInputTensor = xModel.GetInputTensor( 0 );
    TfLiteTensor * xOutputTensor0 = xModel.GetOutputTensor( 0 );
    TfLiteTensor * xOutputTensor1 = xModel.GetOutputTensor( 1 );

    if( !xInputTensor->dims )
    {
        LogError( ( "Invalid input tensor dims\n" ) );
        return -1;
    }
    else if( xInputTensor->dims->size < 3 )
    {
        LogError( ( "Input tensor dimension should be >= 3\n" ) );
        return -1;
    }

    TfLiteIntArray * pxInputShape = xModel.GetInputShape( 0 );

    const int lInputImgCols = pxInputShape->data[ YoloFastestModel::ms_inputColsIdx ];
    const int lInputImgRows = pxInputShape->data[ YoloFastestModel::ms_inputRowsIdx ];

    /* Set up pre and post-processing. */
    /* RGB to grayscale skipped, already done outside */
    DetectorPreProcess xPreProcess = DetectorPreProcess( xInputTensor, false, xModel.IsDataSigned() );

    std::vector<object_detection::DetectionResult> xResults;
    const object_detection::PostProcessParams xPostProcessParams{ lInputImgRows,
                                                                  lInputImgCols,
                                                                  object_detection::originalImageSize,
                                                                  object_detection::anchor1,
                                                                  object_detection::anchor2 };
    DetectorPostProcess xPostProcess = DetectorPostProcess( xOutputTensor0, xOutputTensor1, xResults, xPostProcessParams );
    /* Ensure there are no results leftover from previous inference when running all. */
    xResults.clear();

    /* Run the pre-processing, inference and post-processing. */
    if( !xPreProcess.DoPreProcess( pucImage, xInputTensor->bytes ) )
    {
        LogError( ( "Pre-processing failed." ) );
        return -1;
    }

    /* Run inference over this image. */
    info( "Running inference on image at addr 0x%x\n", ( uint32_t ) pucImage );

    if( !xModel.RunInference() )
    {
        LogError( ( "Inference failed." ) );
        return -1;
    }

    if( !xPostProcess.DoPostProcess() )
    {
        LogError( ( "Post-processing failed." ) );
        return -1;
    }

    for( uint32_t i = 0; i < xResults.size() && i < *pulResultsNum; ++i )
    {
        pxCResults[ i ].ulX = xResults[ i ].m_x0;
        pxCResults[ i ].ulY = xResults[ i ].m_y0;
        pxCResults[ i ].ulW = xResults[ i ].m_w;
        pxCResults[ i ].ulH = xResults[ i ].m_h;
    }

    if( !prvPresentInferenceResult( xResults ) )
    {
        return -1;
    }

    if( *pulResultsNum > xResults.size() )
    {
        *pulResultsNum = xResults.size();
    }

    return 0;
}

static bool prvPresentInferenceResult( const std::vector<object_detection::DetectionResult> &xResults )
{
    /* If profiling is enabled, and the time is valid. */
    LogInfo( ( "Final results:\n" ) );
    LogInfo( ( "Total number of inferences: 1\n" ) );
    LogInfo( ( "Detected faces: %d\n", xResults.size() ) );

    for( uint32_t i = 0; i < xResults.size(); ++i )
    {
        LogInfo( ( "%" PRIu32 ") (%f) -> %s {x=%d,y=%d,w=%d,h=%d}\n",
                   i,
                   xResults[ i ].m_normalisedVal,
                   "Detection box:",
                   xResults[ i ].m_x0,
                   xResults[ i ].m_y0,
                   xResults[ i ].m_w,
                   xResults[ i ].m_h ) );
    }

    std::string xFinalResultStr = "Detected faces: ";
    xFinalResultStr += std::to_string( xResults.size() );

    LogInfo( ( "Complete recognition: %s\n", xFinalResultStr.c_str() ) );

    /* Send the inference result */
    prvSetMlProcessingstate( xFinalResultStr.c_str() );

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
        int lErr = 0;

        SCB_EnableICache();
        SCB_EnableDCache();

        #if defined( ETHOS_U_NPU_TIMING_ADAPTER_ENABLED )

            /* If the platform has timing adapter blocks along with Ethos-U core
             * block, initialise them here. */
            if( 0 != ( err = arm_ethosu_timing_adapter_init() ) )
            {
                LogError( ( "Failed to init timing adapter\n" ) );
                return lErr;
            }
        #endif /* ETHOS_U_NPU_TIMING_ADAPTER_ENABLED */

        /* Initialize the ethos NPU */
        if( 0 != ( lErr = arm_ethosu_npu_init() ) )
        {
            LogError( ( "Failed to init arm npu\n" ) );
            return lErr;
        }

        LogInfo( ( "Ethos-U55 device initialised\n" ) );

        /* Get Ethos-U55 version */
        struct ethosu_driver_version xDriverVersion;
        struct ethosu_hw_info xHwInfo;

        ethosu_get_driver_version( &xDriverVersion );
        ethosu_get_hw_info( &ethosu_drv, &xHwInfo );

        LogInfo( ( "Ethos-U version info:\n" ) );
        LogInfo( ( "\tArch:       v%" PRIu32 ".%" PRIu32 ".%" PRIu32 "\n",
                   xHwInfo.version.arch_major_rev,
                   xHwInfo.version.arch_minor_rev,
                   xHwInfo.version.arch_patch_rev ) );
        LogInfo( ( "\tDriver:     v%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                   xDriverVersion.major,
                   xDriverVersion.minor,
                   xDriverVersion.patch ) );
        LogInfo( ( "\tMACs/cc:    %" PRIu32 "\n", ( uint32_t ) ( 1 << xHwInfo.cfg.macs_per_cc ) ) );
        LogInfo( ( "\tCmd stream: v%" PRIu32 "\n", xHwInfo.cfg.cmd_stream_version ) );

        return 0;
    }
#endif /* USE_ETHOS */

extern "C" {
static int prvMlInterfaceInit( void )
{
    static arm::app::YoloFastestModel xModel; /* Model wrapper object. */

    #ifdef USE_ETHOS
        /* Initialize the ethos U55 */
        if( prvArmNpuInit() != 0 )
        {
            LogError( ( "Failed to arm npu\n" ) );
            return -1;
        }
    #endif /* USE_ETHOS */

    /* Load the model. */
    if( !xModel.Init( arm::app::ucTensorArena,
                      sizeof( arm::app::ucTensorArena ),
                      arm::app::object_detection::GetModelPointer(),
                      arm::app::object_detection::GetModelLen() ) )
    {
        LogError( ( "Failed to initialise model\n" ) );
        return -1;
    }

    xModel.ShowModelInfoHandler();

    /* Instantiate application context. */
    xCaseContext.Set<arm::app::Model &>( "model", xModel );

    PrintTensorFlowVersion();
    LogInfo( ( "*** ML interface initialised\r\n" ) );
    return 0;
}

void vMlTask( void * pvParameters )
{
    LogInfo( ( "ML Task start\r\n" ) );

    EventBits_t xFlags = xEventGroupWaitBits(
        xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_START, pdTRUE, pdFAIL, portMAX_DELAY
        );

    if( xFlags & EVENT_MASK_ML_START )
    {
        LogInfo( ( "Initial start of image processing\r\n" ) );
    }

    if( prvMlInterfaceInit() < 0 )
    {
        LogError( ( "prvMlInterfaceInit failed\r\n" ) );
        return;
    }

    vStartISPDemo();

    while( 1 )
    {
        xFlags = xEventGroupWaitBits(
            xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_STOP, pdTRUE, pdFAIL, 300
            );

        if( xFlags & EVENT_MASK_ML_STOP )
        {
            LogInfo( ( "Stopping image processing\r\n" ) );
            break;
        }
    }
}

void vMlMqttTask( void * pvParameters )
{
    ( void ) pvParameters;

    while( 1 )
    {
        MLMqttMsg_t xMsg;

        if( xQueueReceive( xMlMqttQueue, &xMsg, portMAX_DELAY ) == pdTRUE )
        {
            prvMqttSendMessage( xMsg.pcResult );
            free( reinterpret_cast<void *>( xMsg.pcResult ) );
        }
        else
        {
            LogError( ( "xQueueReceive ML MQTT msg queue failed\r\n" ) );
        }
    }
}
} /* extern "C" */
