/*
 * FreeRTOS Transport Secure Sockets V1.0.1
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file transport_sockets.h
 * @brief Socket based API for transport interface implementation.
 */

#ifndef TRANSPORT_INTERFACE_API_H
#define TRANSPORT_INTERFACE_API_H

#include <stdint.h>
#include <stdbool.h>
#include "transport_interface.h"


struct NetworkContext
{
    int32_t socket;
    void * pTLSContext;
    bool useTLS;
};

/**
 * @brief TCP, TLS Connect / Disconnect return status.
 */
typedef enum TransportStatus
{
    TRANSPORT_STATUS_SUCCESS = 0,           /**< Function successfully completed. */
    TRANSPORT_STATUS_INVALID_PARAMETER,     /**< At least one parameter was invalid. */
    TRANSPORT_STATUS_INSUFFICIENT_MEMORY,   /**< Insufficient memory required to establish connection. */
    TRANSPORT_STATUS_CREDENTIALS_INVALID,   /**< Provided credentials were invalid. */
    TRANSPORT_STATUS_INTERNAL_ERROR,        /**< A call to a system API resulted in an internal error. */
    TRANSPORT_STATUS_DNS_FAILURE,           /**< Resolving hostname of the server failed. */
    TRANSPORT_STATUS_SOCKET_CREATE_FAILURE, /**< Underlying socket creation failed. */
    TRANSPORT_STATUS_CONNECT_FAILURE,       /**< Initial connection to the server failed. */
    TRANSPORT_STATUS_TLS_FAILURE,           /**< TLS Handshake for the secure connection failed. */
    TRANSPORT_STATUS_SOCKET_CLOSE_FAILURE   /**< Failed to close the underlying socket. */
} TransportStatus_t;


/**
 * @brief Information on the remote server for connection setup.
 */
typedef struct ServerInfo
{
    const char * pHostName; /**< @brief Server host name. Must be NULL-terminated. */
    size_t hostNameLength;  /**< @brief Length of the server host name. */
    uint16_t port;          /**< @brief Server port in host-order. */
} ServerInfo_t;


/**
 * @brief Contains the credentials necessary for connection setup.
 *
 */
typedef struct TLSParams
{
    /**
     * @brief Set this to a non-NULL value to use ALPN.
     *
     * This string must be NULL-terminated.
     *
     * See [this link]
     * (https://aws.amazon.com/blogs/iot/mqtt-with-tls-client-authentication-on-port-443-why-it-is-useful-and-how-it-works/)
     * for more information.
     */
    const char * pAlpnProtos;

    /**
     * @brief Disable server name indication (SNI) for a TLS session.
     */
    bool disableSni;

    /**
     * @brief Set this to a non-zero value to use TLS max fragment length
     * negotiation (TLS MFLN).
     *
     * @note The network stack may have a minimum value for this parameter and
     * may return an error if this parameter is too small.
     */
    size_t maxFragmentLength;

    const char * pRootCa; /**< @brief String representing a trusted server Root CA certificate. */
    size_t rootCaSize;    /**< @brief SizeSecureSocketsTransport_Connect associated with #SocketsConfig_t.pRootCa. */

    char * pPrivateKeyLabel;
    char * pClientCertLabel;
    char * pLoginPIN;
} TLSParams_t;


/**
 * @brief Sets up a TCP only connection or a TLS session on top of a TCP connection with Secure Sockets API.
 *
 * @param[out] pNetworkContext The output parameter to return the created network context.
 * @param[in] pServerInfo Server connection info.
 * @param[in] pTLSParams socket configs for the connection.
 * @param[in] sendTimeoutMs socket send timeout.
 * @param[in] recvTimeoutMs socket receive timeout.
 *
 * @return #TRANSPORT_STATUS_SUCCESS on success;
 *         #TRANSPORT_STATUS_INVALID_PARAMETER, #TRANSPORT_STATUS_INSUFFICIENT_MEMORY,
 *         #TRANSPORT_STATUS_CREDENTIALS_INVALID, #TRANSPORT_STATUS_INTERNAL_ERROR,
 *         #TRANSPORT_STATUS_DNS_FAILURE, #TRANSPORT_STATUS_CONNECT_FAILURE on failure.
 */
TransportStatus_t Transport_Connect( NetworkContext_t * pNetworkContext,
                                     const ServerInfo_t * pServerInfo,
                                     const TLSParams_t * pTLSParams,
                                     uint32_t sendTimeoutMs,
                                     uint32_t recvTimeoutMs );

/**
 * @brief Closes a TLS session on top of a TCP connection using the Secure Sockets API.
 *
 * @param[out] pNetworkContext The output parameter to end the TLS session and
 *             clean the created network context.
 *
 * @return #TRANSPORT_STATUS_SUCCESS on success;
 *         #TRANSPORT_STATUS_INVALID_PARAMETER, #TRANSPORT_STATUS_INTERNAL_ERROR on failure.
 */
TransportStatus_t Transport_Disconnect( NetworkContext_t * pNetworkContext );


/**
 * @brief Receives data over an established TLS session using the Secure Sockets API.
 *
 * This can be used as #TransportInterface.recv function for receiving data
 * from the network.
 *
 * @param[in] pNetworkContext The network context created using Socket_Connect API.
 * @param[out] pBuffer Buffer to receive network data into.
 * @param[in] bytesToRecv Number of bytes requested from the network.
 *
 * @return Number of bytes (> 0) received if successful;
 *         0 if the socket times out without reading any bytes;
 *         negative value on error.
 */
int32_t Transport_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv );

/**
 * @brief Sends data over an established TLS session using the Secure Sockets API.
 *
 * This can be used as the #TransportInterface.send function to send data
 * over the network.
 *
 * @param[in] pNetworkContext The network context created using Secure Sockets API.
 * @param[in] pMessage A message to be sent over the network stack.
 * @param[in] bytesToSend Number of bytes to send over the network.
 *
 * @return Number of bytes sent if successful; negative value on error.
 */
int32_t Transport_Send( NetworkContext_t * pNetworkContext,
                        const void * pMessage,
                        size_t bytesToSend );

#endif /* TRANSPORT_INTERFACE_API_H */
