/* Copyright 2021-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */


#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"

/* Include header that defines log levels. */
#include "logging_levels.h"
/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "EthosU_Platform_Adaptation"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

/****************************************************************************
* Mutex & Semaphore
* Overrides weak-linked symbols in ethosu_driver.c to implement thread handling
****************************************************************************/

void * ethosu_mutex_create( void )
{
    SemaphoreHandle_t xEthosMutex = xSemaphoreCreateMutex();

    if( xEthosMutex == NULL )
    {
        LogError( ( "Failed to create xEthosMutex\r\n" ) );
    }

    return xEthosMutex;
}

void ethosu_mutex_lock( void * mutex )
{
    if( mutex != NULL )
    {
        /* SemaphoreHandle_t mutex_id = (SemaphoreHandle_t)mutex; */
        BaseType_t ret = xSemaphoreTake( ( SemaphoreHandle_t ) mutex, portMAX_DELAY );

        if( ret != pdTRUE )
        {
            LogError( ( "xSemaphoreTake Ethos Mutex failed %ld\r\n", ret ) );
            return;
        }
    }
}

void ethosu_mutex_unlock( void * mutex )
{
    if( mutex != NULL )
    {
        BaseType_t status = xSemaphoreGive( ( SemaphoreHandle_t ) mutex );

        if( status != pdPASS )
        {
            LogError( ( "xSemaphoreGive Ethos Mutex failed %ld\r\n", status ) );
            return;
        }
    }
}

void * ethosu_semaphore_create( void )
{
    SemaphoreHandle_t xEthosSemaphore = xSemaphoreCreateBinary();

    if( xEthosSemaphore == NULL )
    {
        LogError( ( "Failed to create xEthosSemaphore\r\n" ) );
    }

    return xEthosSemaphore;
}

int ethosu_semaphore_take( void * sem )
{
    if( sem != NULL )
    {
        BaseType_t ret = xSemaphoreTake( ( SemaphoreHandle_t ) sem, portMAX_DELAY );

        if( ret != pdTRUE )
        {
            LogError( ( "xSemaphoreTake Ethos Semaphore failed %ld\r\n", ret ) );
            return -1;
        }
    }

    return 0;
}

int ethosu_semaphore_give( void * sem )
{
    if( sem != NULL )
    {
        BaseType_t ret = xSemaphoreGive( ( SemaphoreHandle_t ) sem );

        if( ret != pdPASS )
        {
            LogError( ( "xSemaphoreGive Ethos Semaphore failed %ld\r\n", ret ) );
            return -1;
        }
    }

    return 0;
}
