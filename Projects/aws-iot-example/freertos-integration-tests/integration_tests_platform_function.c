/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "psa/crypto.h"
#include "platform_function.h"
#include "transport_interface.h"
#include "transport_interface_api.h"

#include "transport_interface_test.h"
#include "qualification_test.h"
#include "ota_pal_test.h"
#include "mqtt_test.h"

#include "demo_config.h"
#include "core_pkcs11_config.h"

static NetworkContext_t xNetworkContext = { 0 };
static NetworkContext_t xSecondNetworkContext = { 0 };
static TransportInterface_t xTransport = { 0 };

#define TRANSPORT_SEND_RECV_TIMEOUT_MS    ( 750 )

static NetworkConnectStatus_t prvTransportNetworkConnect( void * pNetworkContext,
                                                          TestHostInfo_t * pHostInfo,
                                                          void * pNetworkCredentials )
{
    ( void ) pNetworkCredentials;
    ServerInfo_t server_info =
    {
        .pHostName = pHostInfo->pHostName, .hostNameLength = strlen( pHostInfo->pHostName ), .port = pHostInfo->port
    };

    TransportStatus_t status = TRANSPORT_STATUS_CONNECT_FAILURE;

    status = Transport_Connect( pNetworkContext,
                                &server_info,
                                NULL,
                                TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                TRANSPORT_SEND_RECV_TIMEOUT_MS );

    return ( status == TRANSPORT_STATUS_SUCCESS ) ? NETWORK_CONNECT_SUCCESS : NETWORK_CONNECT_FAILURE;
}

static NetworkConnectStatus_t prvTransportNetworkConnectTLS( void * pNetworkContext,
                                                             TestHostInfo_t * pHostInfo,
                                                             void * pNetworkCredentials )
{
    ( void ) pNetworkCredentials;
    ServerInfo_t server_info =
    {
        .pHostName = pHostInfo->pHostName, .hostNameLength = strlen( pHostInfo->pHostName ), .port = pHostInfo->port
    };

    /* Set the credentials for establishing a TLS connection. */
    TLSParams_t tls_params = { 0 };

    tls_params.pRootCa = democonfigROOT_CA_PEM;
    tls_params.rootCaSize = sizeof( democonfigROOT_CA_PEM );
    tls_params.pClientCertLabel = pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS;
    tls_params.pPrivateKeyLabel = pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS;
    tls_params.disableSni = false;
    tls_params.pLoginPIN = configPKCS11_DEFAULT_USER_PIN;

    TransportStatus_t status = TRANSPORT_STATUS_CONNECT_FAILURE;

    status = Transport_Connect( pNetworkContext,
                                &server_info,
                                &tls_params,
                                TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                TRANSPORT_SEND_RECV_TIMEOUT_MS );

    return ( status == TRANSPORT_STATUS_SUCCESS ) ? NETWORK_CONNECT_SUCCESS : NETWORK_CONNECT_FAILURE;
}

static void prvTransportNetworkDisconnect( void * pNetworkContext )
{
    Transport_Disconnect( pNetworkContext );
}

void SetupTransportTestParam( TransportTestParam_t * pTestParam )
{
    if( pTestParam != NULL )
    {
        /* Setup the transport interface. */
        xTransport.pNetworkContext = &xNetworkContext;
        xTransport.send = Transport_Send;
        xTransport.recv = Transport_Recv;
        xTransport.writev = NULL;

        pTestParam->pTransport = &xTransport;
        pTestParam->pNetworkContext = &xNetworkContext;
        pTestParam->pSecondNetworkContext = &xSecondNetworkContext;

        pTestParam->pNetworkConnect = prvTransportNetworkConnect;
        pTestParam->pNetworkDisconnect = prvTransportNetworkDisconnect;
        pTestParam->pNetworkCredentials = NULL;
    }
}

void SetupMqttTestParam( MqttTestParam_t * pTestParam )
{
    if( pTestParam != NULL )
    {
        xTransport.pNetworkContext = &xNetworkContext;
        xTransport.recv = Transport_Recv;
        xTransport.send = Transport_Send;
        xTransport.writev = NULL;

        pTestParam->pTransport = &xTransport;
        pTestParam->pNetworkConnect = prvTransportNetworkConnectTLS;
        pTestParam->pNetworkDisconnect = prvTransportNetworkDisconnect;
        pTestParam->pNetworkCredentials = NULL;

        pTestParam->pNetworkContext = &xNetworkContext;
        pTestParam->pSecondNetworkContext = &xSecondNetworkContext;
        pTestParam->pGetTimeMs = FRTest_GetTimeMs;
    }
}

void SetupOtaPalTestParam( OtaPalTestParam_t * pTestParam )
{
    pTestParam->pageSize = 4096;
}

typedef struct Task_t
{
    TaskHandle_t handle;
    FRTestThreadFunction_t func;
    void * args;
    SemaphoreHandle_t mutex;
    StaticSemaphore_t mutex_buf;
} Task_t;

static void TaskCaller( void * args )
{
    Task_t * task = args;

    task->func( task->args );
    xSemaphoreGive( task->mutex );
    vTaskDelete( NULL );
}

FRTestThreadHandle_t FRTest_ThreadCreate( FRTestThreadFunction_t func,
                                          void * pParam )
{
    Task_t * task = NULL;

    task = pvPortMalloc( sizeof( Task_t ) );
    task->mutex = xSemaphoreCreateBinaryStatic( &task->mutex_buf );
    task->func = func;
    task->args = pParam;
    xTaskCreate( TaskCaller, "test-func", 8192, task, tskIDLE_PRIORITY, &task->handle );
    return ( FRTestThreadHandle_t ) task;
}

int FRTest_ThreadTimedJoin( FRTestThreadHandle_t threadHandle,
                            uint32_t timeoutMs )
{
    Task_t * task = threadHandle;
    BaseType_t ret = xSemaphoreTake( task->mutex, pdMS_TO_TICKS( timeoutMs ) );

    configASSERT( ret == pdTRUE );
    free( task );
    return 0;
}

int FRTest_GenerateRandInt()
{
    uint32_t rnd = 0;

    psa_generate_random( ( uint8_t * ) &rnd, sizeof( uint32_t ) );
    return rnd;
}

uint32_t FRTest_GetTimeMs()
{
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;

    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();

    /* Convert the ticks to milliseconds. */
    ulTimeMs = TICKS_TO_pdMS( xTickCount );

    return ulTimeMs;
}

void FRTest_TimeDelay( uint32_t delayMs )
{
    vTaskDelay( pdMS_TO_TICKS( delayMs ) );
}

void * FRTest_MemoryAlloc( size_t size )
{
    return pvPortMalloc( size );
}

void FRTest_MemoryFree( void * ptr )
{
    return vPortFree( ptr );
}
