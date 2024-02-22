/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */


/**
 * @brief Stack size and priority for MQTT agent task.
 * Stack size is capped to an adequate value based on requirements from MbedTLS stack
 * for establishing a TLS connection. Task priority of MQTT agent is set to a priority
 * higher than other MQTT application tasks, so that the agent can drain the queue
 * as work is being produced.
 */
#define appCONFIG_MQTT_AGENT_TASK_STACK_SIZE        ( 4096 )
#define appCONFIG_MQTT_AGENT_TASK_PRIORITY          ( tskIDLE_PRIORITY + 2 )

/**
 * @brief Stack size and priority for OTA MQTT agent task.
 * Stack size is capped to an adequate value based on requirements from MbedTLS stack
 * for establishing a TLS connection. Task priority of OTA MQTT agent is set to a priority
 * higher than other MQTT application tasks, so that the agent can drain the queue
 * as work is being produced.
 */
#define appCONFIG_OTA_MQTT_AGENT_TASK_STACK_SIZE    ( 4096 )
#define appCONFIG_OTA_MQTT_AGENT_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1 )

/** @brief Set logging task as high priority task */
#define appCONFIG_LOGGING_TASK_PRIORITY             ( configMAX_PRIORITIES - 1 )
#define appCONFIG_LOGGING_TASK_STACK_SIZE           ( 2048 )
#define appCONFIG_LOGGING_MESSAGE_QUEUE_LENGTH      ( 32 )
