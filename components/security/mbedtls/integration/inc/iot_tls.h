/*
 * FreeRTOS TLS V1.3.1
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
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

#ifndef IOT_TLS_H
#define IOT_TLS_H

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha256.h"
#include "mbedtls/pk.h"
#include "mbedtls/pk_internal.h"
#include "mbedtls/debug.h"
#include "core_pkcs11.h"

typedef struct TLSContext
{
    /* mbedTLS. */
    mbedtls_ssl_context xMbedSslCtx;
    mbedtls_ssl_config xMbedSslConfig;
    mbedtls_x509_crt xMbedX509CA;
    mbedtls_x509_crt xMbedX509Cli;
    mbedtls_pk_context xMbedPkCtx;
    mbedtls_pk_info_t xMbedPkInfo;
    mbedtls_ctr_drbg_context xMbedDrbgCtx;
    mbedtls_x509_crt_profile xCertProfile;

    /* PKCS#11. */
    CK_FUNCTION_LIST_PTR pxP11FunctionList;
    CK_SESSION_HANDLE xP11Session;
    CK_OBJECT_HANDLE xP11PrivateKey;
    CK_KEY_TYPE xKeyType;
} TLSContext_t;

/**
 * @brief Defines callback type for receiving bytes from the network.
 *
 * @param[in] pvCallerContext Opaque context handle provided by caller.
 * @param[out] pucReceiveBuffer Buffer to fill with received data.
 * @param[in] xReceiveLength Length of previous parameter in bytes.
 *
 * @return The number of bytes actually read.
 */
typedef int ( * NetworkRecv_t )( void * pvCallerContext,
                                 unsigned char * pucReceiveBuffer,
                                 size_t xReceiveLength );

/**
 * @brief Defines callback type for sending bytes to the network.
 *
 * @param[in] pvCallerContext Opaque context handle provided by caller.
 * @param[out] pucReceiveBuffer Buffer of data to send.
 * @param[in] xReceiveLength Length of previous parameter in bytes.
 *
 * @return The number of bytes actually sent.
 */
typedef int ( * NetworkSend_t )( void * pvCallerContext,
                                 const unsigned char * pucData,
                                 size_t xDataLength );

/**
 * @brief Defines parameter structure for initializing the TLS interface.
 *
 * @param[in] ulSize Size of the structure in bytes.
 * @param[in] pcDestination Network name of the TLS server.
 * @param[in] pcServerCertificate PEM encoded server certificate to trust.
 * @param[in] ulServerCertificateLength Length in bytes of the encoded server
 * certificate. The length must include the null terminator.
 * @param[in] pxNetworkRecv Caller-defined network receive function pointer.
 * @param[in] pxNetworkSend Caller-defined network send function pointer.
 * @param[in] pvCallerContext Caller-defined context handle to be used with callback
 * functions.
 */
typedef struct TLSHelperParams
{
    const char * pcDestination;
    const char ** pcAlpnProtocols;

    NetworkRecv_t pxNetworkRecv;
    NetworkSend_t pxNetworkSend;
    void * pvCallerContext;

    const char * pcServerCertificate;
    uint32_t ulServerCertificateLength;
    const char * pClientCertLabel; /**< @brief String representing the PKCS #11 label for the client certificate. */
    const char * pPrivateKeyLabel; /**< @brief String representing the PKCS #11 label for the private key. */
    const char * pcLoginPIN;       /**< @brief A login Password used to retrive the credentials */
} TLSHelperParams_t;

/**
 * @brief Initializes the Mbedtls Context.
 *
 * @param[in] pxParams TLS parameters specified by caller.
 * @param[in,out] pxContext Context that needs to be initialized.
 *
 * @return Zero on success. Error return codes have the high bit set.
 */
BaseType_t TLS_Init( TLSHelperParams_t * pxParams,
                     TLSContext_t * pxContext );

/**
 * @brief Perform TLS handshake with the given TLS context.
 *
 * @param pxContext Opaque context handle for TLS library.
 *
 * @return Zero on success. Error return codes have the high bit set.
 */
int32_t TLS_Connect( TLSContext_t * pxContext );

/**
 * @brief Frees resources consumed by the TLS context.
 *
 * @param pvContext Opaque context handle for TLS library.
 */
void TLS_Cleanup( TLSContext_t * pxContext );


int32_t TLS_Recv( TLSContext_t * pxContext,
                  unsigned char * pucReadBuffer,
                  size_t xReadLength );

int32_t TLS_Send( TLSContext_t * pxContext,
                  const unsigned char * pucMsg,
                  size_t xMsgLength );


#endif /* ifndef IOT_TLS_H */
