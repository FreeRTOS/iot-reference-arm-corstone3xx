/* Copyright 2021-2024, Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef BLINK_TASK_H
    #define BLINK_TASK_H

    #ifdef __cplusplus
    extern "C" {
    #endif

/**
 * @brief Create blink task.
 */
    void vStartBlinkTask( void );

/**
 * @brief Blinks LEDs according to ML processing.
 *
 *        LED1 ON and LED2 OFF       => heard YES
 *        LED1 OFF and LED2 OFF      => heard NO
 *        LED1 OFF and LED2 blinking => no/unknown input
 * @param pvParameters Contextual data for the task.
 */
    void vBlinkTask( void * pvParameters );

    #ifdef __cplusplus
    }
    #endif

#endif /* ! BLINK_TASK_H*/
