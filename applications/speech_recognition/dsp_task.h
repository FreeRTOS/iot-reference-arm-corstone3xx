/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DSP_TASK_H
    #define DSP_TASK_H

    #include <stdint.h>

    #ifdef __cplusplus
    extern "C" {
    #endif

    typedef enum
    {
        DSP_EVENT_START, DSP_EVENT_STOP
    } dsp_event_t;
    typedef struct
    {
        dsp_event_t event;
    } dsp_msg_t;

/**
 * @brief Create DSP task.
 * @param pvParameters Contextual data for the task.
 */
    void vStartDSPTask( void * pvParameters );

/**
 * @brief Start the DSP task.
 */
    void vDspStart( void );

/**
 * @brief Stop the DSP task.
 */
    void vDspStop( void );

/**
 * @brief Task to digital signal processing
 * @param pvParameters Contextual data for the task.
 */
    void vDspTask( void * pvParameters );

/**
 * @brief Get a new DSP ML Connection object
 */
    void * pvDspGetMlConnection( void );

    #ifdef __cplusplus
    }
    #endif

#endif /* ! DSP_TASK_H*/
