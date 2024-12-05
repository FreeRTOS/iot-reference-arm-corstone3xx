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

#ifndef OTA_TYPES_DEFINITIONS_H
#define OTA_TYPES_DEFINITIONS_H

/* Standard includes. */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "MQTTFileDownloader.h"

#include "demo_config.h"
#include "MQTTFileDownloader_config.h"

/**
 * @brief The maximum time for which OTA demo waits for an MQTT operation to be complete.
 * This involves receiving an acknowledgment for broker for SUBSCRIBE, UNSUBSCRIBE and non
 * QOS0 publishes.
 */
#define otaexampleMQTT_TIMEOUT_MS         ( 5000U )

/**
 * @brief The maximum size of the file paths used in the demo.
 */
#define otaexampleMAX_FILE_PATH_SIZE      ( 260 )

/**
 * @brief The maximum size of the stream name required for downloading update file
 * from streaming service.
 */
#define otaexampleMAX_STREAM_NAME_SIZE    ( 128 )

/**
 * @brief The delay used in the OTA demo task to periodically output the OTA
 * statistics like number of packets received, dropped, processed and queued per connection.
 */
#define otaexampleTASK_DELAY_MS           ( 10000U )

/**
 * @brief Used to clear bits in a task's notification value.
 */
#define otaexampleMAX_UINT32              ( 0xffffffff )

/**
 * @brief Starting index of client identifier within OTA topic.
 */
/* #define OTA_TOPIC_CLIENT_IDENTIFIER_START_IDX    ( 12U ) */

/**
 * @brief Stack size required for OTA agent task.
 */
#define OTA_AGENT_TASK_STACK_SIZE    ( 5000U )

/**
 * @brief Priority required for OTA agent task.
 */
#define OTA_AGENT_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1 )

/* Max bytes supported for a file signature (3072 bit RSA is 384 bytes). */
#define OTA_MAX_SIGNATURE_SIZE       ( 384U )

/**
 * @brief The timeout for waiting for the agent to get suspended after closing the
 * connection.
 *
 * Timeout value should be large enough for OTA agent to finish any pending MQTT operations
 * and suspend itself.
 *
 */
#define OTA_SUSPEND_TIMEOUT_MS       ( 10000U )

#define OTA_DATA_BLOCK_SIZE          mqttFileDownloader_CONFIG_BLOCK_SIZE
#define JOB_DOC_SIZE                 2048U

typedef enum OtaEvent
{
    OtaAgentEventStart = 0,           /*!< @brief Start the OTA state machine */
    OtaAgentEventRequestJobDocument,  /*!< @brief Event for requesting job document. */
    OtaAgentEventReceivedJobDocument, /*!< @brief Event when job document is received. */
    OtaAgentEventCreateFile,          /*!< @brief Event to create a file. */
    OtaAgentEventRequestFileBlock,    /*!< @brief Event to request file blocks. */
    OtaAgentEventReceivedFileBlock,   /*!< @brief Event to trigger when file block is received. */
    OtaAgentEventCloseFile,           /*!< @brief Event to trigger closing file. */
    OtaAgentEventActivateImage,       /*!< @brief Event to trigger activation of the image. */
    OtaAgentEventSuspend,             /*!< @brief Event to suspend ota task */
    OtaAgentEventResume,              /*!< @brief Event to resume suspended task */
    OtaAgentEventUserAbort,           /*!< @brief Event triggered by user to stop agent. */
    OtaAgentEventShutdown,            /*!< @brief Event to trigger ota shutdown */
    OtaAgentEventMax                  /*!< @brief Last event specifier */
} OtaEvent_t;

/**
 * @brief OTA Agent states.
 *
 * The current state of the OTA Task (OTA Agent).
 */
typedef enum OtaState
{
    OtaAgentStateNoTransition = -1,
    OtaAgentStateInit = 0,
    OtaAgentStateReady,
    OtaAgentStateRequestingJob,
    OtaAgentStateWaitingForJob,
    OtaAgentStateCreatingFile,
    OtaAgentStateRequestingFileBlock,
    OtaAgentStateWaitingForFileBlock,
    OtaAgentStateClosingFile,
    OtaAgentStateSuspended,
    OtaAgentStateShuttingDown,
    OtaAgentStateStopped,
    OtaAgentStateAll
} OtaState_t;

/**
 * @brief  The OTA Agent event and data structures.
 */

typedef struct OtaDataEvent
{
    uint8_t data[ OTA_DATA_BLOCK_SIZE * 2 ]; /*!< Buffer for storing event information. */
    size_t dataLength;                       /*!< Total space required for the event. */
    bool bufferUsed;                         /*!< Flag set when buffer is used otherwise cleared. */
} OtaDataEvent_t;

typedef struct OtaJobEventData
{
    uint8_t jobData[ JOB_DOC_SIZE ];
    size_t jobDataLength;
} OtaJobEventData_t;

/**
 * @brief Stores information about the event message.
 *
 */
typedef struct OtaEventMsg
{
    OtaDataEvent_t * dataEvent;   /*!< Data Event message. */
    OtaJobEventData_t * jobEvent; /*!< Job Event message. */
    OtaEvent_t eventId;           /*!< Identifier for the event. */
} OtaEventMsg_t;

#endif /* OTA_TYPES_DEFINITIONS_H */
