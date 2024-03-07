/* Copyright 2023-2024, Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

/** @brief Set logging task as high priority task */
#define appCONFIG_LOGGING_TASK_PRIORITY           ( configMAX_PRIORITIES - 1 )
#define appCONFIG_LOGGING_TASK_STACK_SIZE         ( 2048 )
#define appCONFIG_LOGGING_MESSAGE_QUEUE_LENGTH    ( 32 )


/**
 * @brief Stack size and priority for OTA MQTT agent task.
 * Stack size is capped to an adequate value based on requirements from MbedTLS stack
 * for establishing a TLS connection. Task priority of OTA MQTT agent is set to a priority
 * higher than other MQTT application tasks, so that the agent can drain the queue
 * as work is being produced.
 */
#define appCONFIG_OTA_MQTT_AGENT_TASK_STACK_SIZE    ( 4096 )
#define appCONFIG_OTA_MQTT_AGENT_TASK_PRIORITY      ( tskIDLE_PRIORITY + 3 )

/**
 * @brief Stack size and priority for MQTT agent task.
 * Stack size is capped to an adequate value based on requirements from MbedTLS stack
 * for establishing a TLS connection. Task priority of MQTT agent is set to a priority
 * higher than other MQTT application tasks, so that the agent can drain the queue
 * as work is being produced.
 */
#define appCONFIG_MQTT_AGENT_TASK_STACK_SIZE        ( 4096 )
#define appCONFIG_MQTT_AGENT_TASK_PRIORITY          ( tskIDLE_PRIORITY + 2 )


#define appCONFIG_ML_TASK_STACK_SIZE                ( 8192 )
#define appCONFIG_ML_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 1 )

#define appCONFIG_DSP_TASK_STACK_SIZE               ( 8192 )
#define appCONFIG_DSP_TASK_PRIORITY                 ( tskIDLE_PRIORITY + 1 )


#define appCONFIG_ML_MQTT_TASK_STACK_SIZE           ( configMINIMAL_STACK_SIZE )
#define appCONFIG_ML_MQTT_TASK_PRIORITY             ( tskIDLE_PRIORITY + 1 )

/**
 * @brief Stack size and priority for Blink task.
 * Stack size is capped to an minimal value. Task priority of Blink is set to a priority
 * lower than other ML task.
 */
#define appCONFIG_BLINK_TASK_STACK_SIZE             ( configMINIMAL_STACK_SIZE )
#define appCONFIG_BLINK_TASK_PRIORITY               ( tskIDLE_PRIORITY )

#define appCONFIG_VSI_CALLBACK_TASK_STACK_SIZE      ( configMINIMAL_STACK_SIZE )
#define appCONFIG_VSI_CALLBACK_TASK_PRIORITY        ( tskIDLE_PRIORITY + 2 )


/** @brief Increase backoff algorithm timeout by 8 seconds when device advisor
 * test is active.
 */

#define appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE    0
