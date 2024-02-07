/* Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef __AWS_DEVICE_ADVISOR_TASK__H__
#define __AWS_DEVICE_ADVISOR_TASK__H__

/* app_config.h is included before checking the value of appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE
* because appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE is defined with 0 or 1 in that header file. */
#include "app_config.h"

#if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 )

    #include <string.h>

/**
 * @brief Create the device advisor validation task.
 */
    void vStartDeviceAdvisorTask( void );

/**
 * @brief The maximum amount of time in milliseconds to wait for the commands
 * to be posted to the MQTT agent should the MQTT agent's command queue be full.
 * Tasks wait in the Blocked state, so don't use any CPU time.
 */
    #define deviceAdvisorMAX_COMMAND_SEND_BLOCK_TIME_MS    ( 5000 )

    #define deviceAdvisorTOPIC_FORMAT                      "device_advisor_test"
    #define deviceAdvisorTOPIC_BUFFER_LENGTH               ( strlen( deviceAdvisorTOPIC_FORMAT ) )
    #define deviceAdvisorTASK_STACK_SIZE                   ( 2048 )
    #define deviceAdvisorTASK_PRIORITY                     ( tskIDLE_PRIORITY + 1 )

/**
 * @brief Size of statically allocated buffers for holding payloads.
 */
    #define deviceAdvisorSTRING_BUFFER_LENGTH              ( 20480 )

#endif /* if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 ) */

#endif /* __AWS_DEVICE_ADVISOR_TASK__H__ */
