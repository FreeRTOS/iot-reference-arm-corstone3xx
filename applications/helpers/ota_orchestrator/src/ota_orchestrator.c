/*
 * Copyright Amazon.com, Inc. and its affiliates. All Rights Reserved.
 * Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 *
 * SPDX-License-Identifier: MIT
 * Licensed under the MIT License. See the LICENSE accompanying this file
 * for the specific language governing permissions and limitations under
 * the License.
 */

/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "app_config.h"

#include "app_strnlen.h"

#include "mqtt_agent_task.h"
#include "events.h"

/* Includes for TF-M */
#include "psa/update.h"

/* Includes for OTA PAL PSA */
#include "version/application_version.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Demo config includes. */
#include "demo_config.h"
#include "iot_default_root_certificates.h"
#include "MQTTFileDownloader_config.h"

/* Library config includes. */
#include "ota_config.h"

/* Subscription manager header include. */
#include "subscription_manager.h"

/* Jobs Library includes. */
#include "jobs.h"
#include "ota_job_processor.h"

/* MQTT File Streams Library includes */
#include "MQTTFileDownloader.h"
#include "MQTTFileDownloader_base64.h"

/* OTA Library Interface include. */
#include "ota_os_freertos.h"

/* Include firmware version struct definition. */
#include "ota_appversion32.h"

/* Include platform abstraction header. */
#include "ota_pal.h"

/* OTA orchestrator includes*/
#include "mqtt_helpers.h"
#include "ota_config.h"
#include "ota_orchestrator_helpers.h"
#include "ota_register_callback.h"
#include "ota_types_definitions.h"

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Configure name and log level for the OTA library. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "OTA"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

#define NUM_OF_BLOCKS_REQUESTED    1U
#define START_JOB_MSG_LENGTH       147U
#define MAX_JOB_ID_LENGTH          64U
#define UPDATE_JOB_MSG_LENGTH      128U

extern void vOtaNotActiveHook( void );
extern void vOtaActiveHook( void );

/* Provides external linkage only when running unit test */
#ifdef UNIT_TESTING
    #define STATIC    /* as nothing */
#else /* ifdef UNIT_TESTING */
    #define STATIC    static
#endif /* UNIT_TESTING */

/* -------------------- Demo configurations ------------------------- */

/**
 * @brief The name of the AWS Thing which will be updated with the new firmware
 * image.
 */
#define OTA_THING_NAME                                   clientcredentialIOT_THING_NAME

/**
 * @brief The common prefix for all OTA topics.
 *
 * Thing name is substituted with a wildcard symbol `+`. OTA agent
 * registers with MQTT broker with the thing name in the topic. This topic
 * filter is used to match incoming packet received and route them to OTA.
 * Thing name is not needed for this matching.
 */
#define OTA_TOPIC_PREFIX                                 "$aws/things/+/"

/**
 * @brief Wildcard topic filter for job notification.
 * The filter is used to match the constructed job notify topic filter from OTA agent and register
 * appropriate callback for it.
 */
#define OTA_JOB_NOTIFY_TOPIC_FILTER                      OTA_TOPIC_PREFIX "jobs/start-next"

/**
 * @brief Length of job notification topic filter.
 */
#define OTA_JOB_NOTIFY_TOPIC_FILTER_LENGTH               ( ( uint16_t ) ( sizeof( OTA_JOB_NOTIFY_TOPIC_FILTER ) - 1 ) )

/**
 * @brief Wildcard topic filter for matching job response messages.
 * This topic filter is used to match the responses from OTA service for OTA agent job requests. THe
 * topic filter is a reserved topic which is not subscribed with MQTT broker.
 *
 */
#define OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER           OTA_TOPIC_PREFIX "jobs/start-next/accepted"

/**
 * @brief Length of job accepted response topic filter.
 */
#define OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER_LENGTH    ( ( uint16_t ) ( sizeof( OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER ) - 1 ) )

/**
 * @brief Wildcard topic filter for matching OTA data packets.
 *  The filter is used to match the constructed data stream topic filter from OTA agent and register
 * appropriate callback for it.
 */
#define OTA_DATA_STREAM_TOPIC_FILTER                     OTA_TOPIC_PREFIX  "streams/#"

/**
 * @brief Length of data stream topic filter.
 */
#define OTA_DATA_STREAM_TOPIC_FILTER_LENGTH              ( ( uint16_t ) ( sizeof( OTA_DATA_STREAM_TOPIC_FILTER ) - 1 ) )

/**
 * @brief Default topic filter for OTA.
 * This is used to route all the packets for OTA reserved topics which OTA agent has not subscribed for.
 */
#define OTA_DEFAULT_TOPIC_FILTER                         OTA_TOPIC_PREFIX "jobs/#"

/**
 * @brief Length of default topic filter.
 */
#define OTA_DEFAULT_TOPIC_FILTER_LENGTH                  ( ( uint16_t ) ( sizeof( OTA_DEFAULT_TOPIC_FILTER ) - 1 ) )

/* -------------------------------------------------------------------------- */

/**
 * @brief Structure used to store the topic filter to ota callback mappings.
 */
typedef struct OtaTopicFilterCallback
{
    const char * pTopicFilter;
    uint16_t topicFilterLength;
    IncomingPubCallback_t callback;
} OtaTopicFilterCallback_t;

/* -------------------------------------------------------------------------- */

/**
 * @brief The current state of the OTA agent.
 */
static OtaState_t otaAgentState = OtaAgentStateInit;

/**
 * @brief The job ID of the OTA job currently being processed.
 */
char globalJobId[ MAX_JOB_ID_LENGTH ] = { 0 };

/**
 * @brief Structure used to hold parameters of a received job document.
 */
static AfrOtaJobDocumentFields_t jobFields = { 0 };

static AfrOtaJobDocumentStatusDetails_t jobStatusDetails = { 0 };

/**
 * @brief Structure used to hold data from a job document.
 */
static OtaJobEventData_t jobDocBuffer = { 0 };

/**
 * @brief Structure used to hold information about a MQTT file stream.
 */
MqttFileDownloaderContext_t mqttFileDownloaderContext = { 0 };

/**
 * @brief The file ID of the current file being downloaded.
 */
static uint8_t currentFileId = 0;


/**
 * @brief Index of the current file block that is being streamed.
 */
static uint32_t currentBlockOffset = 0;

/**
 * @brief Number of file blocks left to be streamed.
 */
static uint32_t numOfBlocksRemaining = 0;

/**
 * @brief Number of bytes that have been received of the file being downloaded.
 */
static uint32_t totalBytesReceived = 0;

/**
 * @brief A statically allocated array of data buffers used by the OTA agent.
 * Maximum number of buffers are determined by how many chunks are requested
 * by OTA agent at a time along with an extra buffer to handle the control
 * message. The size of each buffer is determined by the maximum size of the
 * firmware image chunk, with space for other metadata sent along with the
 * chunk.
 */
static OtaDataEvent_t dataBuffers[ otaconfigMAX_NUM_OTA_DATA_BUFFERS ] = { 0 };

/**
 * @brief Mutex used to manage thread safe access of OTA event buffers.
 */
static SemaphoreHandle_t xBufferSemaphore;

/**
 * @brief Buffer to hold the decoded signature from the job document.
 */
static uint8_t OtaImageSignatureDecoded[ OTA_MAX_SIGNATURE_SIZE ] = { 0 };

/* -------------------------------------------------------------------------- */

/**
 * @brief Put the OTA agent into the Stopped state.
 */
STATIC void otaAgentShutdown( void );

/**
 * @brief Close the file that is currently being downloaded.
 *
 * @return true if the file was closed successfully, otherwise returns false.
 */
STATIC bool closeFile( void );

/**
 * @brief Activate the downloaded firmware image.
 *
 * @return true if the image was activated successfully, otherwise returns
 * false.
 */
STATIC bool activateImage( void );

/**
 * @brief Send a message to the cloud to update the statusDetails field of the
 * job.
 */
STATIC void sendStatusDetailsMessage( void );

/**
 * @brief Send a message to notify the cloud of the job's final status (i.e.
 * accepted or failed).
 */
STATIC void sendFinalJobStatusMessage( JobCurrentStatus_t status );

/**
 * @brief Print the OTA job document metadata.
 *
 * @param[in] jobId String containing the job ID.
 * @param[in] jobFields Pointer to the parameters extracted from the OTA job
 * document.
 */
STATIC void printJobParams( const char * jobId,
                            AfrOtaJobDocumentFields_t jobFields );

/**
 * @brief Send the necessary message to request a job document.
 */
STATIC void requestJobDocumentHandler( void );

/**
 * @brief Process a job document by parsing its parameters and beginning the
 * file streaming.
 *
 * @param[in] jobDoc Pointer to the job document data.
 *
 * @return The OTA platform image state after parsing the job document.
 */
STATIC OtaPalJobDocProcessingResult_t receivedJobDocumentHandler( OtaJobEventData_t * jobDoc );

/**
 * @brief Initialize the MQTT streams downloader.
 *
 * @param[in] jobFields Pointer to the parameters extracted from the OTA job
 * document.
 */
STATIC void initMqttDownloader( AfrOtaJobDocumentFields_t * jobFields );

/**
 * @brief Free an event buffer back to pool
 *
 * OTA demo uses a statically allocated array of fixed size event buffers . The
 * number of event buffers is configured by the param otaconfigMAX_NUM_OTA_DATA_BUFFERS
 * within ota_config.h. The function is used by the OTA application callback to free a buffer,
 * after OTA agent has completed processing with the event. The access to the pool is made thread safe
 * using a mutex.
 *
 * @param[in] pxBuffer Pointer to the buffer to be freed.
 */
STATIC void freeOtaDataEventBuffer( OtaDataEvent_t * const pxBuffer );

/**
 * @brief
 *
 * @param[in] pData Pointer to the received data block.
 * @param[in] dataLength Length of the received data block.
 *
 * @return The number of bytes written successfully, or a negative error code from the platform
 * abstraction layer.
 */
STATIC int16_t handleMqttStreamsBlockArrived( uint8_t * pData,
                                              size_t dataLength );

/**
 * @brief Request the next data block from the file being streamed.
 */
STATIC void requestDataBlock( void );

/**
 * @brief Fetch an unused OTA event buffer from the pool.
 *
 * Demo uses a simple statically allocated array of fixed size event buffers. The
 * number of event buffers is configured by the param otaconfigMAX_NUM_OTA_DATA_BUFFERS
 * within ota_config.h. This function is used to fetch a free buffer from the pool for processing
 * by the OTA agent task. It uses a mutex for thread safe access to the pool.
 *
 * @return A pointer to an unusued buffer. NULL if there are no buffers available.
 */
STATIC OtaDataEvent_t * getOtaDataEventBuffer( void );

/**
 * @brief Receive an OTA event from the event queue and process it based on the
 * type of event.
 */
STATIC void processOTAEvents( void );

/**
 * @brief The function which runs the OTA agent task.
 *
 * The function runs the OTA Agent Event processing loop, which waits for
 * any events for OTA agent and process them. The loop never returns until the OTA agent
 * is shutdown. The tasks exits gracefully by freeing up all resources in the event of an
 *  OTA agent shutdown.
 *
 * @param[in] pvParam Any parameters to be passed to OTA agent task.
 */
STATIC void prvOTAAgentTask( void * pvParam );

/**
 * @brief The function which runs the OTA demo task.
 *
 * The demo task initializes the OTA agent an loops until OTA agent is shutdown.
 * It reports OTA update statistics (which includes number of blocks received, processed and dropped),
 * at regular intervals.
 *
 * @param[in] pvParam Any parameters to be passed to OTA Demo task.
 */
STATIC void vOtaDemoTask( void * pvParam );

/**
 * @brief The function which implements the flow for OTA demo.
 *
 * @return pdPASS if success or pdFAIL.
 */
STATIC BaseType_t prvRunOTADemo( void );

/**
 * @brief Callback registered with the OTA library that notifies the OTA agent
 * of an incoming PUBLISH containing a job document.
 *
 * @param[in] pContext MQTT context which stores the connection.
 * @param[in] pPublishInfo MQTT packet information which stores details of the
 * job document.
 */
STATIC void prvMqttJobCallback( void * pContext,
                                MQTTPublishInfo_t * pPublish );


/**
 * @brief Callback that notifies the OTA library when a data block is received.
 *
 * @param[in] pContext MQTT context which stores the connection.
 * @param[in] pPublishInfo MQTT packet that stores the information of the file block.
 */
STATIC void prvMqttDataCallback( void * pContext,
                                 MQTTPublishInfo_t * pPublish );

/**
 * @brief Default callback used to receive unsolicited messages for OTA.
 *
 * The callback is not subscribed with MQTT broker, but only with local subscription manager.
 * A wildcard OTA job topic is used for subscription so that all unsolicited messages related to OTA is
 * forwarded to this callback for filtration. Right now the callback is used to filter responses to job requests
 * from the OTA service.
 *
 * @param[in] pvIncomingPublishCallbackContext MQTT context which stores the connection.
 * @param[in] pPublishInfo MQTT packet that stores the information of the file block.
 */
STATIC void prvMqttDefaultCallback( void * pvIncomingPublishCallbackContext,
                                    MQTTPublishInfo_t * pxPublishInfo );

/**
 * @brief Registry for all MQTT topic filters to their corresponding callbacks for OTA.
 */
static OtaTopicFilterCallback_t otaTopicFilterCallbacks[] =
{
    {
        .pTopicFilter = OTA_JOB_NOTIFY_TOPIC_FILTER,
        .topicFilterLength = OTA_JOB_NOTIFY_TOPIC_FILTER_LENGTH,
        .callback = prvMqttJobCallback
    },
    {
        .pTopicFilter = OTA_DATA_STREAM_TOPIC_FILTER,
        .topicFilterLength = OTA_DATA_STREAM_TOPIC_FILTER_LENGTH,
        .callback = prvMqttDataCallback
    },
    {
        .pTopicFilter = OTA_DEFAULT_TOPIC_FILTER,
        .topicFilterLength = OTA_DEFAULT_TOPIC_FILTER_LENGTH,
        .callback = prvMqttDefaultCallback
    }
};

/* -------------------------------------------------------------------------- */

/*
 * Helper functions
 */

STATIC void otaAgentShutdown( void )
{
    OtaEventMsg_t eventMsg = { 0 };

    eventMsg.eventId = OtaAgentEventShutdown;

    OtaSendEvent_FreeRTOS( &eventMsg );
}

STATIC bool closeFile( void )
{
    return otaPal_CloseFile( &jobFields );
}

STATIC bool activateImage( void )
{
    return otaPal_ActivateNewImage( &jobFields );
}

STATIC void sendStatusDetailsMessage( void )
{
    char topicBuffer[ TOPIC_BUFFER_SIZE + 1 ] = { 0 };
    size_t topicBufferLength = 0U;
    char messageBuffer[ UPDATE_JOB_MSG_LENGTH ] = { 0 };

    /*
     * AWS IoT Jobs library:
     * Creating the MQTT topic to update the status of OTA job.
     */
    Jobs_Update( topicBuffer,
                 TOPIC_BUFFER_SIZE,
                 OTA_THING_NAME,
                 ( uint16_t ) app_strnlen( OTA_THING_NAME, 1000U ),
                 globalJobId,
                 ( uint16_t ) app_strnlen( globalJobId, 1000U ),
                 &topicBufferLength );

    /*
     * Convert the current app firmware version to a string and send it to the
     * the cloud
     */

    /*
     * Calling snprintf() with NULL and 0 as first two parameters gives the
     * required length of the destination string.
     */
    int updatedByBufferLength = snprintf( NULL,
                                          0,
                                          "%u",
                                          appFirmwareVersion.u.x.build );

    char updatedByBuffer[ updatedByBufferLength + 1 ];

    ( void ) snprintf( updatedByBuffer,
                       updatedByBufferLength + 1,
                       "%u",
                       appFirmwareVersion.u.x.build );

    /*
     * AWS IoT Jobs library:
     * Creating the message which contains the status of OTA job.
     * It will be published on the topic created in the previous step.
     */
    size_t messageBufferLength = Jobs_UpdateMsg( InProgress,
                                                 "2",
                                                 1U,
                                                 updatedByBuffer,
                                                 updatedByBufferLength,
                                                 messageBuffer,
                                                 UPDATE_JOB_MSG_LENGTH );

    prvMQTTPublish( topicBuffer,
                    topicBufferLength,
                    messageBuffer,
                    messageBufferLength,
                    0 );
}

STATIC void sendFinalJobStatusMessage( JobCurrentStatus_t status )
{
    char topicBuffer[ TOPIC_BUFFER_SIZE + 1 ] = { 0 };
    size_t topicBufferLength = 0U;
    char messageBuffer[ UPDATE_JOB_MSG_LENGTH ] = { 0 };

    /*
     * AWS IoT Jobs library:
     * Creating the MQTT topic to update the status of OTA job.
     */
    Jobs_Update( topicBuffer,
                 TOPIC_BUFFER_SIZE,
                 OTA_THING_NAME,
                 ( uint16_t ) app_strnlen( OTA_THING_NAME, 1000U ),
                 globalJobId,
                 ( uint16_t ) app_strnlen( globalJobId, 1000U ),
                 &topicBufferLength );

    /*
     * AWS IoT Jobs library:
     * Creating the message which contains the status of OTA job.
     * It will be published on the topic created in the previous step.
     */
    size_t messageBufferLength = Jobs_UpdateMsg( status,
                                                 "3",
                                                 1U,
                                                 jobStatusDetails.updatedBy,
                                                 jobStatusDetails.updatedByLen,
                                                 messageBuffer,
                                                 UPDATE_JOB_MSG_LENGTH );

    prvMQTTPublish( topicBuffer, topicBufferLength, messageBuffer, messageBufferLength, 0 );
    globalJobId[ 0 ] = 0U;

    if( status == Succeeded )
    {
        LogInfo( ( "OTA update completed successfully.\n" ) );
    }
}

STATIC void printJobParams( const char * jobId,
                            AfrOtaJobDocumentFields_t jobFields )
{
    char streamName[ jobFields.imageRefLen ];
    char filePath[ jobFields.filepathLen ];
    char certFile[ jobFields.certfileLen ];
    char sig[ jobFields.signatureLen ];

    LogInfo( ( "Extracted parameter: [jobid: %s]\n", jobId ) );

    /*
     * Strings in the jobFields structure are not null terminated so copy them
     * to a buffer and null terminate them for printing.
     */
    ( void ) memcpy( streamName, jobFields.imageRef, jobFields.imageRefLen );
    streamName[ jobFields.imageRefLen ] = '\0';
    LogInfo( ( "Extracted parameter: [streamname: %s]\n", streamName ) );

    ( void ) memcpy( filePath, jobFields.filepath, jobFields.filepathLen );
    filePath[ jobFields.filepathLen ] = '\0';
    LogInfo( ( "Extracted parameter: [filepath: %s]\n", filePath ) );

    LogInfo( ( "Extracted parameter: [filesize: %u]\n", jobFields.fileSize ) );
    LogInfo( ( "Extracted parameter: [fileid: %u]\n", jobFields.fileId ) );

    ( void ) memcpy( certFile, jobFields.certfile, jobFields.certfileLen );
    certFile[ jobFields.certfileLen ] = '\0';
    LogInfo( ( "Extracted parameter: [certfile: %s]\n", certFile ) );

    ( void ) memcpy( sig, jobFields.signature, jobFields.signatureLen );
    sig[ jobFields.signatureLen ] = '\0';
    LogInfo( ( "Extracted parameter: [sig-sha256-rsa: %s]\n", sig ) );
}

/* -------------------------------------------------------------------------- */

/*
 * Functions related to callbacks, which are called after an event is received.
 */

STATIC void prvMqttJobCallback( void * pvIncomingPublishCallbackContext,
                                MQTTPublishInfo_t * pxPublishInfo )
{
    OtaEventMsg_t eventMsg = { 0 };

    configASSERT( pxPublishInfo != NULL );
    ( void ) pvIncomingPublishCallbackContext;

    LogInfo( ( "Received job message callback, size %ld.\n", pxPublishInfo->payloadLength ) );

    bool handled = Jobs_IsStartNextAccepted( pxPublishInfo->pTopicName,
                                             pxPublishInfo->topicNameLength,
                                             OTA_THING_NAME,
                                             strlen( OTA_THING_NAME ) );

    if( handled )
    {
        memcpy( jobDocBuffer.jobData, pxPublishInfo->pPayload, pxPublishInfo->payloadLength );
        eventMsg.jobEvent = &jobDocBuffer;
        jobDocBuffer.jobDataLength = pxPublishInfo->payloadLength;
        eventMsg.eventId = OtaAgentEventReceivedJobDocument;
        OtaSendEvent_FreeRTOS( &eventMsg );
    }
}

STATIC void prvMqttDefaultCallback( void * pvIncomingPublishCallbackContext,
                                    MQTTPublishInfo_t * pxPublishInfo )
{
    bool isMatch = false;

    ( void ) MQTT_MatchTopic( pxPublishInfo->pTopicName,
                              pxPublishInfo->topicNameLength,
                              OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER,
                              OTA_JOB_ACCEPTED_RESPONSE_TOPIC_FILTER_LENGTH,
                              &isMatch );

    if( isMatch == true )
    {
        prvMqttJobCallback( pvIncomingPublishCallbackContext, pxPublishInfo );
    }
}

STATIC void prvMqttDataCallback( void * pvIncomingPublishCallbackContext,
                                 MQTTPublishInfo_t * pxPublishInfo )
{
    OtaDataEvent_t * pxData;
    OtaEventMsg_t eventMsg = { 0 };

    configASSERT( pxPublishInfo != NULL );
    ( void ) pvIncomingPublishCallbackContext;

    LogInfo( ( "Received data message callback, size %zu.\n", pxPublishInfo->payloadLength ) );

    pxData = getOtaDataEventBuffer();

    if( pxData != NULL )
    {
        if( ( size_t ) pxData->dataLength >= pxPublishInfo->payloadLength )
        {
            memcpy( pxData->data, pxPublishInfo->pPayload, pxPublishInfo->payloadLength );
            pxData->dataLength = pxPublishInfo->payloadLength;
            eventMsg.eventId = OtaAgentEventReceivedFileBlock;
            eventMsg.dataEvent = pxData;

            /* Send file block received event. */
            OtaSendEvent_FreeRTOS( &eventMsg );
        }
        else
        {
            LogError( ( "Error: OTA data buffers are too small for the data message received.\n" ) );
        }
    }
    else
    {
        LogError( ( "Error: No OTA data buffers available.\n" ) );
    }
}

void prvRegisterOTACallback( const char * pTopicFilter,
                             uint16_t topicFilterLength )
{
    bool isMatch = false;
    MQTTStatus_t mqttStatus = MQTTSuccess;
    uint16_t index = 0U;
    uint16_t numTopicFilters = sizeof( otaTopicFilterCallbacks ) / sizeof( OtaTopicFilterCallback_t );

    bool subscriptionAdded;

    ( void ) mqttStatus;

    /* Match the input topic filter against the wild-card pattern of topics filters
    * relevant for the OTA Update service to determine the type of topic filter. */
    for( ; index < numTopicFilters; index++ )
    {
        mqttStatus = MQTT_MatchTopic( pTopicFilter,
                                      topicFilterLength,
                                      otaTopicFilterCallbacks[ index ].pTopicFilter,
                                      otaTopicFilterCallbacks[ index ].topicFilterLength,
                                      &isMatch );
        assert( mqttStatus == MQTTSuccess );

        if( isMatch )
        {
            /* Add subscription so that incoming publishes are routed to the application callback. */
            subscriptionAdded = addSubscription( pTopicFilter,
                                                 topicFilterLength,
                                                 otaTopicFilterCallbacks[ index ].callback,
                                                 NULL );

            if( subscriptionAdded == false )
            {
                LogError( ( "Failed to register a publish callback for topic %.*s.",
                            pTopicFilter,
                            topicFilterLength ) );
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

/* Functions related to requesting and handling job documents. */

STATIC void requestJobDocumentHandler()
{
    char topicBuffer[ TOPIC_BUFFER_SIZE + 1 ] = { 0 };
    char messageBuffer[ START_JOB_MSG_LENGTH ] = { 0 };
    size_t topicLength = 0U;

    /*
     * AWS IoT Jobs library:
     * Creates the topic string for a StartNextPendingJobExecution request.
     * It used to check if any pending jobs are available.
     */
    Jobs_StartNext( topicBuffer,
                    TOPIC_BUFFER_SIZE,
                    OTA_THING_NAME,
                    ( uint16_t ) strlen( OTA_THING_NAME ),
                    &topicLength );

    /*
     * AWS IoT Jobs library:
     * Creates the message string for a StartNextPendingJobExecution request.
     * It will be sent on the topic created in the previous step.
     */
    size_t messageLength = Jobs_StartNextMsg( OTA_THING_NAME,
                                              strlen( OTA_THING_NAME ),
                                              messageBuffer,
                                              START_JOB_MSG_LENGTH );

    prvMQTTPublish( topicBuffer, topicLength, messageBuffer, messageLength, 0 );
}

STATIC OtaPalJobDocProcessingResult_t receivedJobDocumentHandler( OtaJobEventData_t * jobDoc )
{
    bool parseJobDocument = false;
    char * jobId;
    const char ** jobIdptr = &jobId;
    size_t jobIdLength = 0U;
    OtaPalJobDocProcessingResult_t xResult = OtaPalJobDocFileCreateFailed;

    memset( &jobFields, 0, sizeof( jobFields ) );

    /*
     * AWS IoT Jobs library:
     * Extracting the job ID from the received OTA job document.
     */
    jobIdLength = Jobs_GetJobId( ( char * ) jobDoc->jobData, jobDoc->jobDataLength, jobIdptr );

    if( jobIdLength )
    {
        if( strncmp( globalJobId, jobId, jobIdLength ) )
        {
            parseJobDocument = true;
            memcpy( globalJobId, jobId, jobIdLength );
        }
        else
        {
            xResult = OtaPalJobDocFileCreated;
        }
    }

    if( parseJobDocument )
    {
        bool handled = jobDocumentParser( ( char * ) jobDoc->jobData, jobDoc->jobDataLength, &jobFields );

        populateJobStatusDetailsFields( ( char * ) jobDoc->jobData, jobDoc->jobDataLength, &jobStatusDetails );

        if( handled )
        {
            printJobParams( globalJobId, jobFields );

            /*In pending commit state, the device is in self test mode */
            if( otaPal_GetPlatformImageState( &jobFields ) == OtaPalImageStatePendingCommit )
            {
                /*
                 * Convert the updatedBy string to an integer so the updatedBy
                 * version can be compared to the update firmware version.
                 */
                char updatedByBuffer[ jobStatusDetails.updatedByLen ];
                char * endPtr;

                /*
                 * updatedBy string is not null terminated so copy it to a
                 * temporary string and null terminate.
                 */
                ( void ) memcpy( updatedByBuffer,
                                 jobStatusDetails.updatedBy,
                                 jobStatusDetails.updatedByLen );

                updatedByBuffer[ jobStatusDetails.updatedByLen ] = '\0';

                uint16_t updatedByVer = ( uint16_t ) strtoul( updatedByBuffer,
                                                              &endPtr,
                                                              10 );

                if( updatedByVer < appFirmwareVersion.u.x.build )
                {
                    LogInfo( ( "New image has a higher version number than the current image: "
                               "New image version=%u"
                               ", Previous image version=%u",
                               appFirmwareVersion.u.x.build,
                               updatedByVer ) );

                    otaPal_SetPlatformImageState( &jobFields, OtaImageStateAccepted );
                    ( void ) sendFinalJobStatusMessage( Succeeded );

                    xResult = OtaPalNewImageBooted;
                }
                else
                {
                    LogInfo( ( "Application version of the new image is not higher than the current image: "
                               "New images are expected to have a higher version number." ) );

                    otaPal_SetPlatformImageState( &jobFields, OtaImageStateRejected );

                    /*
                     * Mark the job as FAILED (AWS Job Service will not allow
                     * the job to be set to REJECTED if the job has been
                     * started already).
                     */
                    ( void ) sendFinalJobStatusMessage( Failed );

                    xResult = OtaPalNewImageBootFailed;
                }
            }
            else
            {
                initMqttDownloader( &jobFields );

                /* AWS IoT core returns the signature in a PEM format. We need to
                 * convert it to DER format for image signature verification. */

                handled = convertSignatureToDER( OtaImageSignatureDecoded, sizeof( OtaImageSignatureDecoded ), &jobFields );

                if( handled )
                {
                    xResult = otaPal_CreateFileForRx( &jobFields );
                }
                else
                {
                    LogError( ( "Failed to decode the image signature to DER format." ) );
                }
            }
        }
    }

    return xResult;
}

/* -------------------------------------------------------------------------- */

/* Functions related to handling streamed file blocks. */

STATIC void initMqttDownloader( AfrOtaJobDocumentFields_t * jobFields )
{
    numOfBlocksRemaining = jobFields->fileSize /
                           mqttFileDownloader_CONFIG_BLOCK_SIZE;
    numOfBlocksRemaining += ( jobFields->fileSize %
                              mqttFileDownloader_CONFIG_BLOCK_SIZE > 0 ) ? 1 : 0;
    currentFileId = ( uint8_t ) jobFields->fileId;
    currentBlockOffset = 0;
    totalBytesReceived = 0;

    /*
     * MQTT streams Library:
     * Initializing the MQTT streams downloader. Passing the
     * parameters extracted from the AWS IoT OTA jobs document
     * using OTA jobs parser.
     */
    mqttDownloader_init( &mqttFileDownloaderContext,
                         jobFields->imageRef,
                         jobFields->imageRefLen,
                         OTA_THING_NAME,
                         strlen( OTA_THING_NAME ),
                         DATA_TYPE_JSON );

    prvMQTTSubscribe( mqttFileDownloaderContext.topicStreamData,
                      mqttFileDownloaderContext.topicStreamDataLength,
                      0 );
}

STATIC void requestDataBlock( void )
{
    char getStreamRequest[ GET_STREAM_REQUEST_BUFFER_SIZE ];
    size_t getStreamRequestLength = 0U;

    /*
     * MQTT streams Library:
     * Creating the Get data block request. MQTT streams library only
     * creates the get block request. To publish the request, MQTT libraries
     * like coreMQTT are required.
     */
    getStreamRequestLength = mqttDownloader_createGetDataBlockRequest( mqttFileDownloaderContext.dataType,
                                                                       currentFileId,
                                                                       mqttFileDownloader_CONFIG_BLOCK_SIZE,
                                                                       ( uint16_t ) currentBlockOffset,
                                                                       NUM_OF_BLOCKS_REQUESTED,
                                                                       getStreamRequest,
                                                                       GET_STREAM_REQUEST_BUFFER_SIZE );

    prvMQTTPublish( mqttFileDownloaderContext.topicGetStream,
                    mqttFileDownloaderContext.topicGetStreamLength,
                    ( uint8_t * ) getStreamRequest,
                    getStreamRequestLength,
                    0 );
}

/* Stores the received data blocks in the flash partition reserved for OTA */
STATIC int16_t handleMqttStreamsBlockArrived( uint8_t * data,
                                              size_t dataLength )
{
    int16_t writeblockRes = -1;

    LogInfo( ( "Downloaded block %u of %u. \n", currentBlockOffset, ( currentBlockOffset + numOfBlocksRemaining ) ) );

    writeblockRes = otaPal_WriteBlock( &jobFields,
                                       totalBytesReceived,
                                       data,
                                       dataLength );

    if( writeblockRes > 0 )
    {
        totalBytesReceived += writeblockRes;
    }

    return writeblockRes;
}

/* -------------------------------------------------------------------------- */

/* Functions related to handling data buffers. */

STATIC OtaDataEvent_t * getOtaDataEventBuffer( void )
{
    OtaDataEvent_t * freeBuffer = NULL;

    if( xSemaphoreTake( xBufferSemaphore, portMAX_DELAY ) == pdTRUE )
    {
        for( uint32_t ulIndex = 0; ulIndex < otaconfigMAX_NUM_OTA_DATA_BUFFERS; ulIndex++ )
        {
            if( dataBuffers[ ulIndex ].bufferUsed == false )
            {
                dataBuffers[ ulIndex ].bufferUsed = true;
                freeBuffer = &dataBuffers[ ulIndex ];
                freeBuffer->dataLength = sizeof( freeBuffer->data );
                break;
            }
        }

        ( void ) xSemaphoreGive( xBufferSemaphore );
    }
    else
    {
        LogInfo( ( "Failed to get buffer semaphore. \n" ) );
    }

    return freeBuffer;
}

STATIC void freeOtaDataEventBuffer( OtaDataEvent_t * const pxBuffer )
{
    if( xSemaphoreTake( xBufferSemaphore, portMAX_DELAY ) == pdTRUE )
    {
        pxBuffer->bufferUsed = false;
        ( void ) xSemaphoreGive( xBufferSemaphore );
    }
    else
    {
        LogInfo( ( "Failed to get buffer semaphore.\n" ) );
    }
}

/* -------------------------------------------------------------------------- */

/* Functions related to running the OTA demo. */

STATIC void processOTAEvents()
{
    OtaEventMsg_t recvEvent = { 0 };
    OtaEvent_t recvEventId = 0;
    OtaEventMsg_t nextEvent = { 0 };

    OtaReceiveEvent_FreeRTOS( &recvEvent );
    recvEventId = recvEvent.eventId;

    switch( recvEventId )
    {
        case OtaAgentEventRequestJobDocument:
            LogInfo( ( "Requesting job document.\n" ) );
            requestJobDocumentHandler();
            otaAgentState = OtaAgentStateRequestingJob;
            break;

        case OtaAgentEventReceivedJobDocument:

            if( otaAgentState == OtaAgentStateSuspended )
            {
                LogInfo( ( "OTA agent is in Suspended state. Dropping job document. \n" ) );
                break;
            }
            else if( otaAgentState == OtaAgentStateStopped )
            {
                LogInfo( ( "OTA agent is in Stopped state. Dropping job document. \n" ) );
                break;
            }

            switch( receivedJobDocumentHandler( recvEvent.jobEvent ) )
            {
                case OtaPalJobDocFileCreated:
                    LogInfo( ( "Received OTA job document.\n" ) );
                    nextEvent.eventId = OtaAgentEventRequestFileBlock;
                    OtaSendEvent_FreeRTOS( &nextEvent );
                    otaAgentState = OtaAgentStateCreatingFile;
                    vOtaActiveHook();
                    break;

                case OtaPalNewImageBooted:
                    LogInfo( ( "New firmware image booted.\n" ) );
                    vOtaNotActiveHook();
                    break;

                case OtaPalJobDocFileCreateFailed:
                case OtaPalNewImageBootFailed:
                case OtaPalJobDocProcessingStateInvalid:
                    LogInfo( ( "No OTA job available. \n" ) );
                    break;
            }

            break;

        case OtaAgentEventRequestFileBlock:
            otaAgentState = OtaAgentStateRequestingFileBlock;
            LogInfo( ( "Requesting file block.\n" ) );

            if( currentBlockOffset == 0 )
            {
                LogInfo( ( "Starting the download. \n" ) );
            }

            requestDataBlock();

            break;

        case OtaAgentEventReceivedFileBlock:
            LogInfo( ( "Received file block.\n" ) );

            if( otaAgentState == OtaAgentStateSuspended )
            {
                LogInfo( ( "OTA agent is in Suspended State. Dropping file block. \n" ) );
                freeOtaDataEventBuffer( recvEvent.dataEvent );
                break;
            }
            else if( otaAgentState == OtaAgentStateStopped )
            {
                LogInfo( ( "OTA Agent is in Stopped State. Dropping file block. \n" ) );
                freeOtaDataEventBuffer( recvEvent.dataEvent );
                break;
            }

            int32_t fileId;
            int32_t blockId;
            int32_t blockSize;
            uint8_t decodedData[ mqttFileDownloader_CONFIG_BLOCK_SIZE ];
            size_t decodedDataLength = 0;
            int16_t result;

            /*
             * MQTT streams Library:
             * Extracting and decoding the received data block from the incoming MQTT message.
             */
            mqttDownloader_processReceivedDataBlock(
                &mqttFileDownloaderContext,
                recvEvent.dataEvent->data,
                recvEvent.dataEvent->dataLength,
                &fileId,
                &blockId,
                &blockSize,
                decodedData,
                &decodedDataLength );

            result = handleMqttStreamsBlockArrived( decodedData, decodedDataLength );
            freeOtaDataEventBuffer( recvEvent.dataEvent );

            if( result > 0 )
            {
                numOfBlocksRemaining--;
                currentBlockOffset++;
            }

            if( numOfBlocksRemaining == 0 )
            {
                nextEvent.eventId = OtaAgentEventCloseFile;
                OtaSendEvent_FreeRTOS( &nextEvent );
            }
            else
            {
                nextEvent.eventId = OtaAgentEventRequestFileBlock;
                OtaSendEvent_FreeRTOS( &nextEvent );
            }

            break;

        case OtaAgentEventCloseFile:
            LogInfo( ( "Closing file.\n" ) );

            if( closeFile() == true )
            {
                nextEvent.eventId = OtaAgentEventActivateImage;
                OtaSendEvent_FreeRTOS( &nextEvent );
            }

            break;

        case OtaAgentEventActivateImage:
            LogInfo( ( "Attempting to activate image.\n" ) );

            sendStatusDetailsMessage();

            if( activateImage() == true )
            {
                LogInfo( ( "Activated image.\n" ) );
                nextEvent.eventId = OtaAgentEventActivateImage;
                OtaSendEvent_FreeRTOS( &nextEvent );
            }

            otaAgentState = OtaAgentStateStopped;
            break;


        case OtaAgentEventSuspend:
            LogInfo( ( "Suspending OTA agent.\n" ) );
            otaAgentState = OtaAgentStateSuspended;
            break;

        case OtaAgentEventResume:
            LogInfo( ( "Resuming OTA agent.\n" ) );
            otaAgentState = OtaAgentStateRequestingJob;
            nextEvent.eventId = OtaAgentEventRequestJobDocument;
            OtaSendEvent_FreeRTOS( &nextEvent );
            break;

        case OtaAgentEventShutdown:
            LogInfo( ( "Shutting down OTA agent.\n" ) );
            otaAgentState = OtaAgentStateStopped;
            vOtaNotActiveHook();
            break;

        default:
            break;
    }
}

STATIC void prvOTAAgentTask( void * pParam )
{
    /* Start processing OTA events. */

    otaAgentState = OtaAgentStateReady;

    while( otaAgentState != OtaAgentStateStopped )
    {
        processOTAEvents();
    }

    LogInfo( ( "OTA Agent stopped." ) );

    vTaskDelete( NULL );
}

STATIC BaseType_t prvRunOTADemo( void )
{
    /* Status indicating a successful demo or not. */
    BaseType_t xStatus = pdPASS;

    /* OTA event message used for sending event to OTA Agent.*/
    OtaEventMsg_t eventMsg = { 0 };

    OtaEventMsg_t initEvent = { 0 };

    vWaitUntilMQTTAgentReady();
    vWaitUntilMQTTAgentConnected();

    /****************************** Init OTA Library. ******************************/

    if( xStatus == pdPASS )
    {
        OtaInitEvent_FreeRTOS();
    }

    /****************************** Create OTA Agent Task. ******************************/

    if( xStatus == pdPASS )
    {
        xStatus = xTaskCreate( prvOTAAgentTask,
                               "OTA Agent Task",
                               OTA_AGENT_TASK_STACK_SIZE,
                               NULL,
                               OTA_AGENT_TASK_PRIORITY,
                               NULL );

        if( xStatus != pdPASS )
        {
            LogError( ( "Failed to create OTA agent task:" ) );
        }
    }

    /**
     * Register a callback for receiving messages intended for OTA agent from broker,
     * for which the topic has not been subscribed for.
     */
    prvRegisterOTACallback( OTA_DEFAULT_TOPIC_FILTER, OTA_DEFAULT_TOPIC_FILTER_LENGTH );

    /****************************** Start OTA ******************************/

    if( xStatus == pdPASS )
    {
        initEvent.eventId = OtaAgentEventRequestJobDocument;
        OtaSendEvent_FreeRTOS( &initEvent );

        for( ; ; )
        {
            if( !xIsMqttAgentConnected() )
            {
                eventMsg.eventId = OtaAgentEventSuspend;
                OtaSendEvent_FreeRTOS( &eventMsg );

                LogInfo( ( "Suspended OTA agent." ) );
            }
            else
            {
                if( otaAgentState == OtaAgentStateSuspended )
                {
                    eventMsg.eventId = OtaAgentEventResume;
                    OtaSendEvent_FreeRTOS( &eventMsg );

                    LogInfo( ( "Resumed OTA agent." ) );
                }
            }

            vTaskDelay( pdMS_TO_TICKS( otaexampleTASK_DELAY_MS ) );
        }
    }

    /**
     * Remove callback for receiving messages intended for OTA agent from broker,
     * for which the topic has not been subscribed for.
     */
    removeSubscription( OTA_DEFAULT_TOPIC_FILTER,
                        OTA_DEFAULT_TOPIC_FILTER_LENGTH );

    return xStatus;
}

/**
 * @brief Entry point of Ota demo task.
 *
 * This example initializes the OTA library to enable OTA updates via the
 * MQTT broker. It simply connects to the MQTT broker with the users
 * credentials and spins in an indefinite loop to allow MQTT messages to be
 * forwarded to the OTA agent for possible processing. The OTA agent does all
 * of the real work; checking to see if the message topic is one destined for
 * the OTA agent. If not, it is simply ignored.
 */
STATIC void vOtaDemoTask( void * pvParam )
{
    ( void ) pvParam;

    if( GetImageVersionPSA( FWU_COMPONENT_ID_NONSECURE ) == 0 )
    {
        LogInfo( ( "OTA over MQTT, Application version from appFirmwareVersion %u.%u.%u\n",
                   appFirmwareVersion.u.x.major,
                   appFirmwareVersion.u.x.minor,
                   appFirmwareVersion.u.x.build ) );
    }
    else
    {
        LogError( ( "OTA over MQTT, unable to get application versions" ) );
    }

    /* Initialize semaphore for buffer operations. */
    xBufferSemaphore = xSemaphoreCreateMutex();

    if( xBufferSemaphore == NULL )
    {
        LogError( ( "Failed to initialize buffer semaphore." ) );
    }
    else
    {
        /****************************** Start OTA Demo. ******************************/

        /* Start OTA demo. The function returns only if OTA completes successfully and a
         * shutdown of OTA is triggered for a manual restart of the device. */
        if( prvRunOTADemo() != pdPASS )
        {
            LogError( ( "Failed to complete OTA successfully." ) );
        }

        /* / ****************************** Cleanup ****************************** / */

        /* Cleanup semaphore created for buffer operations. */
        vSemaphoreDelete( xBufferSemaphore );
    }
}

/*
 * @brief Create the task that demonstrates the Ota demo.
 */
void vStartOtaTask( void )
{
    xTaskCreate( vOtaDemoTask,                             /* Function that implements the task. */
                 "OTA Task",                               /* Text name for the task - only used for debugging. */
                 appCONFIG_OTA_MQTT_AGENT_TASK_STACK_SIZE, /* Size of stack (in words, not bytes) to allocate for the task. */
                 NULL,                                     /* Optional - task parameter - not used in this case. */
                 appCONFIG_OTA_MQTT_AGENT_TASK_PRIORITY,   /* Task priority, must be between 0 and configMAX_PRIORITIES - 1. */
                 NULL );                                   /* Optional - used to pass out a handle to the created task. */
}
