/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/**
 * @file tls_freertos.c
 * @brief TLS transport interface implementations. This implementation uses
 * mbedTLS.
 */

#include "logging_levels.h"

#define LIBRARY_LOG_NAME     "TRANSPORT TLS"
#define LIBRARY_LOG_LEVEL    LOG_INFO

#include "logging_stack.h"

/* Standard includes. */
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* Transport header. */
#include "transport_interface_api.h"

/* TLS helper header. */
#include "iot_tls.h"

static int Recv_Cb( void * pvCallerContext,
                    unsigned char * pucReceiveBuffer,
                    size_t xReceiveLength );
static int Send_Cb( void * pvCallerContext,
                    const unsigned char * pucData,
                    size_t xDataLength );

TransportStatus_t Transport_Connect( NetworkContext_t * pNetworkContext,
                                     const ServerInfo_t * pServerInfo,
                                     const TLSParams_t * pTLSParams,
                                     uint32_t sendTimeoutMs,
                                     uint32_t recvTimeoutMs )
{
    TransportStatus_t status = TRANSPORT_STATUS_SUCCESS;
    int32_t socketStatus = 0;
    struct freertos_sockaddr serverAddress = { 0 };
    TLSHelperParams_t tlsHelperParams = { 0 };
    TickType_t transportTimeout = 0;

    if( ( pNetworkContext == NULL ) || ( pServerInfo == NULL ) )
    {
        status = TRANSPORT_STATUS_INVALID_PARAMETER;
    }
    else if( pServerInfo->pHostName == NULL )
    {
        status = TRANSPORT_STATUS_INVALID_PARAMETER;
    }
    else
    {
        /* Create a TCP socket. */
        pNetworkContext->socket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );

        if( pNetworkContext->socket == FREERTOS_INVALID_SOCKET )
        {
            LogError( ( "Failed to create new socket." ) );
            status = TRANSPORT_STATUS_SOCKET_CREATE_FAILURE;
        }

        /* Resolve the DNS name and get the IP address of the host. */
        if( status == TRANSPORT_STATUS_SUCCESS )
        {
            serverAddress.sin_family = FREERTOS_AF_INET;
            serverAddress.sin_port = FreeRTOS_htons( pServerInfo->port );
            serverAddress.sin_len = ( uint8_t ) sizeof( serverAddress );

            LogInfo( ( "Resolving host name: %s.", pServerInfo->pHostName ) );
            #if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
                serverAddress.sin_address.ulIP_IPv4 = ( uint32_t ) FreeRTOS_gethostbyname( pServerInfo->pHostName );

                /* Check for errors from DNS lookup. */
                if( serverAddress.sin_address.ulIP_IPv4 == 0U )
            #else
                serverAddress.sin_addr = ( uint32_t ) FreeRTOS_gethostbyname( pServerInfo->pHostName );

                /* Check for errors from DNS lookup. */
                if( serverAddress.sin_addr == 0U )
            #endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */
            {
                LogError( ( "Failed to connect to server: DNS resolution failed: Hostname=%s.",
                            pServerInfo->pHostName ) );
                status = TRANSPORT_STATUS_DNS_FAILURE;
            }
        }

        /* Create a TCP connection to the host. */
        if( status == TRANSPORT_STATUS_SUCCESS )
        {
            LogInfo( ( "Initiating TCP connection with host: %s:%d\r\n", pServerInfo->pHostName, pServerInfo->port ) );
            socketStatus = FreeRTOS_connect( pNetworkContext->socket, &serverAddress, sizeof( serverAddress ) );

            if( socketStatus < 0 )
            {
                LogError( ( "Failed to connect to server: FreeRTOS_Connect failed: ReturnCode=%d,"
                            " Hostname=%s, Port=%u.",
                            socketStatus,
                            pServerInfo->pHostName,
                            pServerInfo->port ) );
                status = TRANSPORT_STATUS_CONNECT_FAILURE;
            }
        }

        /* IF this is a secure TLS connection, perform the TLS handshake. */
        if( ( status == TRANSPORT_STATUS_SUCCESS ) && ( pTLSParams != NULL ) )
        {
            /* Initialize TLS */
            tlsHelperParams.pcDestination = pServerInfo->pHostName;
            tlsHelperParams.pcServerCertificate = pTLSParams->pRootCa;
            tlsHelperParams.ulServerCertificateLength = pTLSParams->rootCaSize;
            tlsHelperParams.pvCallerContext = pNetworkContext;
            tlsHelperParams.pxNetworkRecv = &Recv_Cb;
            tlsHelperParams.pxNetworkSend = &Send_Cb;
            tlsHelperParams.pPrivateKeyLabel = pTLSParams->pPrivateKeyLabel;
            tlsHelperParams.pClientCertLabel = pTLSParams->pClientCertLabel;
            tlsHelperParams.pcLoginPIN = pTLSParams->pLoginPIN;


            pNetworkContext->pTLSContext = pvPortMalloc( sizeof( TLSContext_t ) );

            if( pNetworkContext->pTLSContext != NULL )
            {
                /* Set socket receive timeout. */
                transportTimeout = pdMS_TO_TICKS( recvTimeoutMs );
                /* Setting the receive block time cannot fail. */
                ( void ) FreeRTOS_setsockopt( pNetworkContext->socket,
                                              0,
                                              FREERTOS_SO_RCVTIMEO,
                                              &transportTimeout,
                                              sizeof( TickType_t ) );

                /* Set socket send timeout. */
                transportTimeout = pdMS_TO_TICKS( sendTimeoutMs );
                /* Setting the send block time cannot fail. */
                ( void ) FreeRTOS_setsockopt( pNetworkContext->socket,
                                              0,
                                              FREERTOS_SO_SNDTIMEO,
                                              &transportTimeout,
                                              sizeof( TickType_t ) );

                if( TLS_Init( &tlsHelperParams,
                              ( TLSContext_t * ) ( pNetworkContext->pTLSContext ) ) == pdPASS )
                {
                    LogInfo( ( "Initiating TLS handshake with host: %s:%d\r\n", pServerInfo->pHostName, pServerInfo->port ) );

                    /* Initiate TLS handshake */
                    if( TLS_Connect( ( TLSContext_t * ) ( pNetworkContext->pTLSContext ) ) != 0 )
                    {
                        status = TRANSPORT_STATUS_TLS_FAILURE;
                    }
                }
                else
                {
                    status = TRANSPORT_STATUS_TLS_FAILURE;
                }
            }
            else
            {
                status = TRANSPORT_STATUS_INSUFFICIENT_MEMORY;
            }
        }
    }

    return status;
}

TransportStatus_t Transport_Disconnect( NetworkContext_t * pNetworkContext )
{
    TransportStatus_t status = TRANSPORT_STATUS_SUCCESS;
    int32_t socketStatus;
    BaseType_t waitForShutdownLoopCount = 0;
    uint8_t pDummyBuffer[ 2 ];

    if( ( pNetworkContext == NULL ) || ( pNetworkContext->socket == NULL ) )
    {
        status = TRANSPORT_STATUS_INVALID_PARAMETER;
    }
    else
    {
        /* Initiate graceful shutdown. */
        ( void ) FreeRTOS_shutdown( pNetworkContext->socket, FREERTOS_SHUT_RDWR );

        /* Wait for the socket to disconnect gracefully (indicated by FreeRTOS_recv()
         * returning a FREERTOS_EINVAL error) before closing the socket. */
        while( FreeRTOS_recv( pNetworkContext->socket, pDummyBuffer, sizeof( pDummyBuffer ), 0 ) >= 0 )
        {
            /* We don't need to delay since FreeRTOS_recv should already have a timeout. */

            if( ++waitForShutdownLoopCount >= 3 )
            {
                break;
            }
        }

        ( void ) FreeRTOS_closesocket( pNetworkContext->socket );

        if( pNetworkContext->pTLSContext != NULL )
        {
            TLS_Cleanup( ( TLSContext_t * ) ( pNetworkContext->pTLSContext ) );
            vPortFree( pNetworkContext->pTLSContext );
            pNetworkContext->pTLSContext = NULL;
        }
    }

    return status;
}
/*-----------------------------------------------------------*/

int32_t Transport_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv )
{
    int32_t rc = -1;

    if( ( pNetworkContext == NULL ) || ( pBuffer == NULL ) || ( bytesToRecv == 0 ) )
    {
        rc = -1;
    }
    else
    {
        if( pNetworkContext->pTLSContext != NULL )
        {
            rc = TLS_Recv( ( TLSContext_t * ) ( pNetworkContext->pTLSContext ), pBuffer, bytesToRecv );
        }
        else
        {
            LogError( ( "Transport_Recv: TLS context is NULL" ) );
        }
    }

    return rc;
}

int32_t Transport_Send( NetworkContext_t * pNetworkContext,
                        const void * pMessage,
                        size_t bytesToSend )
{
    int32_t rc = -1;

    if( ( pNetworkContext == NULL ) || ( pMessage == NULL ) || ( bytesToSend == 0 ) )
    {
        rc = -1;
    }
    else
    {
        if( pNetworkContext->pTLSContext != NULL )
        {
            rc = TLS_Send( ( TLSContext_t * ) ( pNetworkContext->pTLSContext ), pMessage, bytesToSend );
        }
        else
        {
            LogError( ( "Transport_Send: TLS context is NULL" ) );
        }
    }

    return rc;
}

/**
 * @brief Defines callback type for receiving bytes from the network.
 *
 * @param[in] pvCallerContext Opaque context handle provided by caller.
 * @param[out] pucReceiveBuffer Buffer to fill with received data.
 * @param[in] xReceiveLength Length of previous parameter in bytes.
 *
 * @return The number of bytes actually read or appropriate error code.
 */
static int Recv_Cb( void * pvCallerContext,
                    unsigned char * pucReceiveBuffer,
                    size_t xReceiveLength )
{
    int xReturnStatus = pdFREERTOS_ERRNO_NONE;
    NetworkContext_t * pNetworkContext = ( NetworkContext_t * ) ( pvCallerContext );

    if( ( pNetworkContext == NULL ) || ( pucReceiveBuffer == NULL ) )
    {
        xReturnStatus = pdFREERTOS_ERRNO_EINVAL;
    }
    else
    {
        xReturnStatus = FreeRTOS_recv( pNetworkContext->socket, pucReceiveBuffer, xReceiveLength, 0 );
    }

    return( xReturnStatus );
}

/**
 * @brief Defines callback type for sending bytes to the network.
 *
 * @param[in] pvCallerContext Opaque context handle provided by caller.
 * @param[out] pucReceiveBuffer Buffer of data to send.
 * @param[in] xReceiveLength Length of previous parameter in bytes.
 *
 * @return The number of bytes actually sent.
 */
static int Send_Cb( void * pvCallerContext,
                    const unsigned char * pucData,
                    size_t xDataLength )
{
    int xReturnStatus = pdFREERTOS_ERRNO_NONE;
    NetworkContext_t * pNetworkContext = ( NetworkContext_t * ) ( pvCallerContext );

    if( ( pNetworkContext == NULL ) || ( pucData == NULL ) )
    {
        xReturnStatus = pdFREERTOS_ERRNO_EINVAL;
    }
    else
    {
        xReturnStatus = FreeRTOS_send( pNetworkContext->socket, pucData, xDataLength, 0 );
    }

    return( xReturnStatus );
}
