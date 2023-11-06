/* Copyright 2021-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef ML_INTERFACE_H
    #define ML_INTERFACE_H

    #include <stddef.h>
    #include <stdint.h>

    #ifdef __cplusplus
    extern "C" {
    #endif

/**
 * @brief Start the inference task.
 */
    void vMlTaskInferenceStart( void );

/**
 * @brief Stop the inference task.
 */
    void vMlTaskInferenceStop( void );

/**
 * @brief Task to perform ML processing.
 *        It is gated by the net task which lets it run
 *        if no ota job is present.
 * @param pvParameters Contextual data for the task.
 */
    void vMlTask( void * pvParameters );

/**
 * @brief Task to communicate ML results via MQTT.
 * @param pvParameters Contextual data for the task.
 */
    void vMlMqttTask( void * pvParameters );

/**
 * @brief Create ML MQTT task.
 */
    void vStartMlMqttTask( void );

/**
 * @brief Create ML task.
 * @param pvParameters Contextual data for the task.
 */
    void vStartMlTask( void * pvParameters );

    #ifdef __cplusplus
    }
    #endif

#endif /* ! ML_INTERFACE_H */
