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

#include "logging_levels.h"
/* Logging configuration for the PKCS #11 library. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME    "TRANSPORT-TLS"
#endif

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

/* C runtime includes. */
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "iot_tls.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "mbedtls/debug.h"
#include "mbedtls/base64.h"
#include "iot_default_root_certificates.h"

int8_t PKI_pkcs11SignatureTombedTLSSignature( uint8_t * pucSig,
                                              size_t * pxSigLen );

#ifdef MBEDTLS_DEBUG_C
    #define tlsDEBUG_VERBOSE    4
#endif

#define TLS_ERROR_SIGN          ( -2003 )       /*!< Error in sign operation. */

/**
 * @brief Represents string to be logged when mbedTLS returned error
 * does not contain a high-level code.
 */
static const char * pNoHighLevelMbedTlsCodeStr = "<No-High-Level-Code>";

/**
 * @brief Represents string to be logged when mbedTLS returned error
 * does not contain a low-level code.
 */
static const char * pNoLowLevelMbedTlsCodeStr = "<No-Low-Level-Code>";

/**
 * @brief Utility for converting the high-level code in an mbedTLS error to string,
 * if the code-contains a high-level code; otherwise, using a default string.
 */
#define mbedtlsHighLevelCodeOrDefault( mbedTlsCode )    pNoHighLevelMbedTlsCodeStr


/**
 * @brief Utility for converting the level-level code in an mbedTLS error to string,
 * if the code-contains a level-level code; otherwise, using a default string.
 */
#define mbedtlsLowLevelCodeOrDefault( mbedTlsCode )    pNoLowLevelMbedTlsCodeStr

/*-----------------------------------------------------------*/

/**
 * @brief TLS internal context rundown helper routine.
 *
 * @param[in] pvContext Caller context.
 */

static void prvFreeContext( TLSContext_t * pxContext )
{
    if( NULL != pxContext )
    {
        /* Cleanup mbedTLS. */
        mbedtls_x509_crt_free( &pxContext->xMbedX509CA );
        mbedtls_x509_crt_free( &pxContext->xMbedX509Cli );
        mbedtls_ssl_close_notify( &pxContext->xMbedSslCtx ); /*lint !e534 The error is already taken care of inside mbedtls_ssl_close_notify*/
        mbedtls_ssl_free( &pxContext->xMbedSslCtx );
        mbedtls_ssl_config_free( &pxContext->xMbedSslConfig );
        mbedtls_ctr_drbg_free( &pxContext->xMbedDrbgCtx );

        if( ( CK_INVALID_HANDLE != pxContext->xP11Session ) &&
            ( NULL != pxContext->pxP11FunctionList->C_CloseSession ) )
        {
            ( void ) pxContext->pxP11FunctionList->C_CloseSession( pxContext->xP11Session );
            pxContext->xP11Session = CK_INVALID_HANDLE;
        }
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Callback that wraps PKCS#11 for pseudo-random number generation.
 *
 * @param[in] pvCtx Caller context.
 * @param[in] pucRandom Byte array to fill with random data.
 * @param[in] xRandomLength Length of byte array.
 *
 * @return Zero on success.
 */
static int prvGenerateRandomBytes( void * pvContext,
                                   unsigned char * pucRandom,
                                   size_t xRandomLength )
{
    /* Must cast from void pointer to conform to mbed TLS API. */
    TLSContext_t * pxTLSContext = ( TLSContext_t * ) pvContext;
    CK_RV xResult;

    xResult = pxTLSContext->pxP11FunctionList->C_GenerateRandom( pxTLSContext->xP11Session, pucRandom, xRandomLength );

    if( xResult != CKR_OK )
    {
        LogError( ( "Failed to generate random bytes from the PKCS #11 module, error = %ld", xResult ) );
    }

    return xResult;
}

/*-----------------------------------------------------------*/

/**
 * @brief Callback that enforces a worst-case expiration check on TLS server
 * certificates.
 *
 * @param[in] pvCtx Caller context.
 * @param[in] pxCertificate Certificate to check.
 * @param[in] lPathCount Location of this certificate in the chain.
 * @param[in] pulFlags Verification status flags.
 *
 * @return Zero on success.
 */
static int prvCheckCertificate( void * pvContext,
                                mbedtls_x509_crt * pxCertificate,
                                int lPathCount,
                                uint32_t * pulFlags )
{
    /* Unreferenced parameters. */
    ( void ) ( pvContext );
    ( void ) ( lPathCount );

    /* TODO: Implement this with RTC. */
    return 0;
}

/*-----------------------------------------------------------*/

/**
 * @brief Sign a cryptographic hash with the private key.
 *
 * @param[in] pvContext Crypto context.
 * @param[in] xMdAlg Unused.
 * @param[in] pucHash Length in bytes of hash to be signed.
 * @param[in] uiHashLen Byte array of hash to be signed.
 * @param[out] pucSig RSA signature bytes.
 * @param[in] pxSigLen Length in bytes of signature buffer.
 * @param[in] piRng Unused.
 * @param[in] pvRng Unused.
 *
 * @return Zero on success.
 */
static int prvPrivateKeySigningCallback( void * pvContext,
                                         mbedtls_md_type_t xMdAlg,
                                         const unsigned char * pucHash,
                                         size_t xHashLen,
                                         unsigned char * pucSig,
                                         size_t * pxSigLen,
                                         int ( * piRng )( void *,
                                                          unsigned char *,
                                                          size_t ), /*lint !e955 This parameter is unused. */
                                         void * pvRng )
{
    CK_RV xResult = CKR_OK;
    int lFinalResult = 0;
    TLSContext_t * pxTLSContext = ( TLSContext_t * ) pvContext;
    CK_MECHANISM xMech = { 0 };
    CK_BYTE xToBeSigned[ 256 ];
    CK_ULONG xToBeSignedLen = sizeof( xToBeSigned );

    /* Unreferenced parameters. */
    ( void ) ( piRng );
    ( void ) ( pvRng );
    ( void ) ( xMdAlg );

    /* Sanity check buffer length. */
    if( xHashLen > sizeof( xToBeSigned ) )
    {
        xResult = CKR_ARGUMENTS_BAD;
    }

    /* Format the hash data to be signed. */
    if( CKK_RSA == pxTLSContext->xKeyType )
    {
        xMech.mechanism = CKM_RSA_PKCS;

        /* mbedTLS expects hashed data without padding, but PKCS #11 C_Sign function performs a hash
         * & sign if hash algorithm is specified.  This helper function applies padding
         * indicating data was hashed with SHA-256 while still allowing pre-hashed data to
         * be provided. */
        xResult = vAppendSHA256AlgorithmIdentifierSequence( ( uint8_t * ) pucHash, xToBeSigned );
        xToBeSignedLen = pkcs11RSA_SIGNATURE_INPUT_LENGTH;
    }
    else if( CKK_EC == pxTLSContext->xKeyType )
    {
        xMech.mechanism = CKM_ECDSA;
        memcpy( xToBeSigned, pucHash, xHashLen );
        xToBeSignedLen = xHashLen;
    }
    else
    {
        xResult = CKR_ARGUMENTS_BAD;
    }

    if( CKR_OK == xResult )
    {
        /* Use the PKCS#11 module to sign. */
        xResult = pxTLSContext->pxP11FunctionList->C_SignInit( pxTLSContext->xP11Session,
                                                               &xMech,
                                                               pxTLSContext->xP11PrivateKey );
    }

    if( CKR_OK == xResult )
    {
        *pxSigLen = sizeof( xToBeSigned );
        xResult = pxTLSContext->pxP11FunctionList->C_Sign( ( CK_SESSION_HANDLE ) pxTLSContext->xP11Session,
                                                           xToBeSigned,
                                                           xToBeSignedLen,
                                                           pucSig,
                                                           ( CK_ULONG_PTR ) pxSigLen );
    }

    if( ( xResult == CKR_OK ) && ( CKK_EC == pxTLSContext->xKeyType ) )
    {
        /* PKCS #11 for P256 returns a 64-byte signature with 32 bytes for R and 32 bytes for S.
         * This must be converted to an ASN.1 encoded array. */
        if( *pxSigLen != pkcs11ECDSA_P256_SIGNATURE_LENGTH )
        {
            xResult = CKR_FUNCTION_FAILED;
        }

        if( xResult == CKR_OK )
        {
            PKI_pkcs11SignatureTombedTLSSignature( pucSig, pxSigLen );
        }
    }

    if( xResult != CKR_OK )
    {
        LogError( ( "Failure in signing callback: %ld \r\n", xResult ) );
        lFinalResult = TLS_ERROR_SIGN;
    }

    return lFinalResult;
}

/*-----------------------------------------------------------*/

/**
 * @brief Helper for reading the specified certificate object, if present,
 * out of storage, into RAM, and then into an mbedTLS certificate context
 * object.
 *
 * @param[in] pxTlsContext Caller TLS context.
 * @param[in] pcLabelName PKCS #11 certificate object label.
 * @param[in] xClass PKCS #11 certificate object class.
 * @param[out] pxCertificateContext Certificate context.
 *
 * @return Zero on success.
 */
static CK_RV prvReadCertificateIntoContext( TLSContext_t * pxTlsContext,
                                            char * pcLabelName,
                                            CK_OBJECT_CLASS xClass,
                                            mbedtls_x509_crt * pxCertificateContext )
{
    CK_RV xResult = CKR_OK;
    int mbedTLSResult = 0;
    CK_ATTRIBUTE xTemplate = { 0 };
    CK_OBJECT_HANDLE xCertObj = 0;

    /* Get the handle of the certificate. */
    xResult = xFindObjectWithLabelAndClass( pxTlsContext->xP11Session,
                                            pcLabelName,
                                            strlen( pcLabelName ),
                                            xClass,
                                            &xCertObj );

    if( ( CKR_OK == xResult ) && ( xCertObj == CK_INVALID_HANDLE ) )
    {
        xResult = CKR_OBJECT_HANDLE_INVALID;
    }

    /* Query the certificate size. */
    if( CKR_OK == xResult )
    {
        xTemplate.type = CKA_VALUE;
        xTemplate.ulValueLen = 0;
        xTemplate.pValue = NULL;
        xResult = ( BaseType_t ) pxTlsContext->pxP11FunctionList->C_GetAttributeValue( pxTlsContext->xP11Session,
                                                                                       xCertObj,
                                                                                       &xTemplate,
                                                                                       1 );
    }

    /* Create a buffer for the certificate. */
    if( CKR_OK == xResult )
    {
        xTemplate.pValue = pvPortMalloc( xTemplate.ulValueLen ); /*lint !e9079 Allow casting void* to other types. */

        if( NULL == xTemplate.pValue )
        {
            xResult = ( BaseType_t ) CKR_HOST_MEMORY;
        }
    }

    /* Export the certificate. */
    if( CKR_OK == xResult )
    {
        xResult = ( BaseType_t ) pxTlsContext->pxP11FunctionList->C_GetAttributeValue( pxTlsContext->xP11Session,
                                                                                       xCertObj,
                                                                                       &xTemplate,
                                                                                       1 );
    }

    /* Decode the certificate. */
    if( CKR_OK == xResult )
    {
        mbedTLSResult = mbedtls_x509_crt_parse( pxCertificateContext,
                                                ( const unsigned char * ) xTemplate.pValue,
                                                xTemplate.ulValueLen );

        if( mbedTLSResult != 0 )
        {
            LogError( ( "Failed to parse the certificate with mbedtls error: ",
                        mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                        mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );
            xResult = CKR_FUNCTION_FAILED;
        }
    }

    /* Free memory. */
    if( NULL != xTemplate.pValue )
    {
        vPortFree( xTemplate.pValue );
    }

    return xResult;
}

/*-----------------------------------------------------------*/

/**
 * @brief Helper for setting up potentially hardware-based cryptographic context
 * for the client TLS certificate and private key.
 *
 * @param Caller context.
 *
 * @return Zero on success.
 */
static CK_RV prvInitializeClientCredential( TLSContext_t * pxContext,
                                            TLSHelperParams_t * pxParams )
{
    CK_RV xResult = CKR_OK;
    CK_ATTRIBUTE xTemplate[ 2 ];
    mbedtls_pk_type_t xKeyAlgo = ( mbedtls_pk_type_t ) ~0;

    /* Initialize the mbed contexts. */
    mbedtls_x509_crt_init( &pxContext->xMbedX509Cli );

    if( pxContext->xP11Session == CK_INVALID_HANDLE )
    {
        xResult = CKR_SESSION_HANDLE_INVALID;
        LogError( ( "PKCS #11 session was not initialized.\r\n" ) );
    }

    /* Put the module in authenticated mode. */
    if( CKR_OK == xResult )
    {
        xResult = ( BaseType_t ) pxContext->pxP11FunctionList->C_Login( pxContext->xP11Session,
                                                                        CKU_USER,
                                                                        ( CK_UTF8CHAR_PTR ) pxParams->pcLoginPIN,
                                                                        strlen( pxParams->pcLoginPIN ) );
    }

    if( CKR_OK == xResult )
    {
        /* Get the handle of the device private key. */
        xResult = xFindObjectWithLabelAndClass( pxContext->xP11Session,
                                                ( char * ) pxParams->pPrivateKeyLabel,
                                                strlen( pxParams->pPrivateKeyLabel ),
                                                CKO_PRIVATE_KEY,
                                                &pxContext->xP11PrivateKey );
    }

    if( ( CKR_OK == xResult ) && ( pxContext->xP11PrivateKey == CK_INVALID_HANDLE ) )
    {
        xResult = CKR_FUNCTION_FAILED;
        LogError( ( "Private key not found at label %.*s", strlen( pxParams->pPrivateKeyLabel ), ( char * ) pxParams->pPrivateKeyLabel ) );
    }

    /* Query the device private key type. */
    if( xResult == CKR_OK )
    {
        xTemplate[ 0 ].type = CKA_KEY_TYPE;
        xTemplate[ 0 ].pValue = &pxContext->xKeyType;
        xTemplate[ 0 ].ulValueLen = sizeof( CK_KEY_TYPE );
        xResult = pxContext->pxP11FunctionList->C_GetAttributeValue( pxContext->xP11Session,
                                                                     pxContext->xP11PrivateKey,
                                                                     xTemplate,
                                                                     1 );
    }

    /* Map the PKCS #11 key type to an mbedTLS algorithm. */
    if( xResult == CKR_OK )
    {
        switch( pxContext->xKeyType )
        {
            case CKK_RSA:
                xKeyAlgo = MBEDTLS_PK_RSA;
                break;

            case CKK_EC:
                xKeyAlgo = MBEDTLS_PK_ECKEY;
                break;

            default:
                xResult = CKR_ATTRIBUTE_VALUE_INVALID;
                break;
        }
    }

    /* Map the mbedTLS algorithm to its internal metadata. */
    if( xResult == CKR_OK )
    {
        memcpy( &pxContext->xMbedPkInfo, mbedtls_pk_info_from_type( xKeyAlgo ), sizeof( mbedtls_pk_info_t ) );

        pxContext->xMbedPkInfo.sign_func = prvPrivateKeySigningCallback;
        pxContext->xMbedPkCtx.pk_info = &pxContext->xMbedPkInfo;
        pxContext->xMbedPkCtx.pk_ctx = pxContext;
    }

    /* Get the handle of the device client certificate. */
    if( xResult == CKR_OK )
    {
        xResult = prvReadCertificateIntoContext( pxContext,
                                                 ( char * ) pxParams->pClientCertLabel,
                                                 CKO_CERTIFICATE,
                                                 &pxContext->xMbedX509Cli );
    }

    /* Attach the client certificate(s) and private key to the TLS configuration. */
    if( CKR_OK == xResult )
    {
        xResult = mbedtls_ssl_conf_own_cert( &pxContext->xMbedSslConfig,
                                             &pxContext->xMbedX509Cli,
                                             &pxContext->xMbedPkCtx );
    }

    return xResult;
}

/*-----------------------------------------------------------*/

/**
 * @brief Helper to seed the entropy module used by the DRBG. Periodically this
 * this function will be called to get more random data from the TRNG.
 *
 * @param[in] tlsContext The TLS context.
 * @param[out] outputBuffer The output buffer to return the generated random data.
 * @param[in] outputBufferLength Length of the output buffer.
 *
 * @return Zero on success, otherwise a negative error code telling the cause of the error.
 */
static int prvEntropyCallback( void * pxContext,
                               unsigned char * outputBuffer,
                               size_t outputBufferLength )
{
    int ret = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    CK_RV xResult = CKR_OK;
    TLSContext_t * pxCtx = ( TLSContext_t * ) pxContext; /*lint !e9087 !e9079 Allow casting void* to other types. */

    if( pxCtx->xP11Session != CK_INVALID_HANDLE )
    {
        xResult = pxCtx->pxP11FunctionList->C_GenerateRandom( pxCtx->xP11Session,
                                                              outputBuffer,
                                                              outputBufferLength );
    }
    else
    {
        xResult = CKR_SESSION_HANDLE_INVALID;
        LogError( ( "PKCS #11 session was not initialized.\r\n" ) );
    }

    if( xResult == CKR_OK )
    {
        ret = 0;
    }
    else
    {
        LogError( ( "PKCS #11 C_GenerateRandom failed with error code: %d\r\n", xResult ) );
    }

    return ret;
}


/*-----------------------------------------------------------*/

#ifdef MBEDTLS_DEBUG_C
    static void prvTlsDebugPrint( void * ctx,
                                  int lLevel,
                                  const char * pcFile,
                                  int lLine,
                                  const char * pcStr )
    {
        /* Unused parameters. */
        ( void ) ctx;
        ( void ) pcFile;
        ( void ) lLine;

        /* Send the debug string to the portable logger. */
        vLoggingPrintf( "mbedTLS: |%d| %s", lLevel, pcStr );
    }
#endif /* ifdef MBEDTLS_DEBUG_C */
/*-----------------------------------------------------------*/

static int parseDefaultRootCA( TLSContext_t * pxContext )
{
    int xResult;

    xResult = mbedtls_x509_crt_parse( &pxContext->xMbedX509CA,
                                      ( const unsigned char * ) tlsVERISIGN_ROOT_CERTIFICATE_PEM,
                                      tlsVERISIGN_ROOT_CERTIFICATE_LENGTH );

    if( 0 == xResult )
    {
        xResult = mbedtls_x509_crt_parse( &pxContext->xMbedX509CA,
                                          ( const unsigned char * ) tlsATS1_ROOT_CERTIFICATE_PEM,
                                          tlsATS1_ROOT_CERTIFICATE_LENGTH );

        if( 0 == xResult )
        {
            xResult = mbedtls_x509_crt_parse( &pxContext->xMbedX509CA,
                                              ( const unsigned char * ) tlsATS3_ROOT_CERTIFICATE_PEM,
                                              tlsATS3_ROOT_CERTIFICATE_LENGTH );

            if( 0 == xResult )
            {
                xResult = mbedtls_x509_crt_parse( &pxContext->xMbedX509CA,
                                                  ( const unsigned char * ) tlsSTARFIELD_ROOT_CERTIFICATE_PEM,
                                                  tlsSTARFIELD_ROOT_CERTIFICATE_LENGTH );
            }
        }
    }

    return xResult;
}

BaseType_t TLS_Init( TLSHelperParams_t * pxParams,
                     TLSContext_t * pxContext )
{
    CK_RV xPKCS11Result = CKR_OK;
    int mbedTLSResult = 0;
    BaseType_t xResult = pdTRUE;
    CK_C_GetFunctionList xCkGetFunctionList = NULL;

    if( NULL != pxContext )
    {
        memset( pxContext, 0, sizeof( TLSContext_t ) );

        mbedtls_ssl_init( &pxContext->xMbedSslCtx );
        mbedtls_ctr_drbg_init( &pxContext->xMbedDrbgCtx );


        /* Get the function pointer list for the PKCS#11 module. */
        xCkGetFunctionList = C_GetFunctionList;
        xPKCS11Result = ( BaseType_t ) xCkGetFunctionList( &pxContext->pxP11FunctionList );

        /* Ensure that the PKCS #11 module is initialized and create a session. */
        if( xPKCS11Result == CKR_OK )
        {
            xPKCS11Result = xInitializePkcs11Session( &pxContext->xP11Session );

            /* It is ok if the module was previously initialized. */
            if( xPKCS11Result == CKR_CRYPTOKI_ALREADY_INITIALIZED )
            {
                xPKCS11Result = CKR_OK;
            }

            if( xPKCS11Result != CKR_OK )
            {
                LogError( ( "Failed to initialize PKCS11 session with error: %d", xPKCS11Result ) );
                xResult = pdFALSE;
            }
        }

        if( xResult == pdTRUE )
        {
            mbedTLSResult = mbedtls_ctr_drbg_seed( &pxContext->xMbedDrbgCtx,
                                                   prvEntropyCallback,
                                                   pxContext,
                                                   NULL,
                                                   0 );

            if( 0 != mbedTLSResult )
            {
                LogError( ( "Failed to setup DRBG seed %s : %s \r\n",
                            mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                            mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );
                xResult = pdFALSE;
            }
        }

        if( xResult == pdTRUE )
        {
            mbedtls_x509_crt_init( &pxContext->xMbedX509CA );

            if( pxParams->pcServerCertificate != NULL )
            {
                mbedTLSResult = mbedtls_x509_crt_parse( &pxContext->xMbedX509CA,
                                                        ( const unsigned char * ) pxParams->pcServerCertificate,
                                                        pxParams->ulServerCertificateLength );

                if( 0 != mbedTLSResult )
                {
                    LogError( ( "Failed to parse custom server certificates %s : %s \r\n",
                                mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                                mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );

                    xResult = pdFALSE;
                }
            }
            else
            {
                mbedTLSResult = parseDefaultRootCA( pxContext );

                if( 0 != mbedTLSResult )
                {
                    /* Default root certificates should be in aws_default_root_certificate.h */
                    LogError( ( "Failed to parse default server certificates %s : %s \r\n",
                                mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                                mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );
                    xResult = pdFALSE;
                }
            }
        }

        /* Configure protocol defaults. */
        if( xResult == pdTRUE )
        {
            mbedtls_ssl_config_init( &pxContext->xMbedSslConfig );
            mbedTLSResult = mbedtls_ssl_config_defaults( &pxContext->xMbedSslConfig,
                                                         MBEDTLS_SSL_IS_CLIENT,
                                                         MBEDTLS_SSL_TRANSPORT_STREAM,
                                                         MBEDTLS_SSL_PRESET_DEFAULT );

            if( 0 != mbedTLSResult )
            {
                LogError( ( "Failed to set ssl config defaults %s : %s \r\n",
                            mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                            mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );

                xResult = pdFALSE;
            }
        }

        if( xResult == pdTRUE )
        {
            /* Use a callback for additional server certificate validation. */
            mbedtls_ssl_conf_verify( &pxContext->xMbedSslConfig,
                                     &prvCheckCertificate,
                                     pxContext );
            /* Server certificate validation is mandatory. */
            mbedtls_ssl_conf_authmode( &pxContext->xMbedSslConfig, MBEDTLS_SSL_VERIFY_REQUIRED );

            /* Set the RNG callback. */
            mbedtls_ssl_conf_rng( &pxContext->xMbedSslConfig, &prvGenerateRandomBytes, pxContext ); /*lint !e546 Nothing wrong here. */

            /* Set issuer certificate. */
            mbedtls_ssl_conf_ca_chain( &pxContext->xMbedSslConfig, &pxContext->xMbedX509CA, NULL );

            pxContext->xCertProfile = mbedtls_x509_crt_profile_default;
            mbedtls_ssl_conf_cert_profile( &( pxContext->xMbedSslConfig ),
                                           &( pxContext->xCertProfile ) );


            #ifdef MBEDTLS_DEBUG_C
            {
                /* If mbedTLS is being compiled with debug support, assume that the
                 * runtime configuration should use verbose output. */
                mbedtls_ssl_conf_dbg( &pxContext->xMbedSslConfig, prvTlsDebugPrint, NULL );
                mbedtls_debug_set_threshold( tlsDEBUG_VERBOSE );
            }
            #endif
        }

        if( xResult == pdTRUE )
        {
            xPKCS11Result = prvInitializeClientCredential( pxContext, pxParams );

            if( xPKCS11Result != CKR_OK )
            {
                LogError( ( "Failed to initialize credentials, PKCS11 error: %d", xPKCS11Result ) );
                xResult = pdFALSE;
            }
        }

        if( ( xResult == pdTRUE ) && ( NULL != pxParams->pcAlpnProtocols ) )
        {
            /* Include an application protocol list in the TLS ClientHello
             * message. */
            mbedTLSResult = mbedtls_ssl_conf_alpn_protocols( &pxContext->xMbedSslConfig,
                                                             pxParams->pcAlpnProtocols );

            if( 0 != mbedTLSResult )
            {
                LogError( ( "Failed to set ALPN protocol %s : %s \r\n",
                            mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                            mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );

                xResult = pdFALSE;
            }
        }

        /* Set the hostname, if requested. */
        if( ( xResult == pdTRUE ) && ( NULL != pxParams->pcDestination ) )
        {
            mbedTLSResult = mbedtls_ssl_set_hostname( &pxContext->xMbedSslCtx, pxParams->pcDestination );

            if( 0 != mbedTLSResult )
            {
                LogError( ( "Failed to set hostname %s : %s \r\n",
                            mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                            mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );

                xResult = pdFALSE;
            }
        }

        #ifdef MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
            if( xResult == pdTRUE )
            {
                /* Enable the max fragment extension. 4096 bytes is currently the largest fragment size permitted.
                 * See RFC 6066 https://tools.ietf.org/html/rfc6066#page-8 for more information on the Maximum
                 * Fragment Length extension of TLS.
                 *
                 * Smaller values can be found in "mbedtls/include/ssl.h".
                 */
                mbedTLSResult = mbedtls_ssl_conf_max_frag_len( &pxContext->xMbedSslConfig, MBEDTLS_SSL_MAX_FRAG_LEN_4096 );

                if( 0 != mbedTLSResult )
                {
                    LogError( ( "Failed to set max frag len, %s : %s \r\n",
                                mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                                mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );

                    xResult = pdFALSE;
                }
            }
        #endif /* ifdef MBEDTLS_SSL_MAX_FRAGMENT_LENGTH */

        if( xResult == pdTRUE )
        {
            /* Set the resulting protocol configuration. */
            mbedTLSResult = mbedtls_ssl_setup( &pxContext->xMbedSslCtx, &pxContext->xMbedSslConfig );

            if( 0 != mbedTLSResult )
            {
                LogError( ( "Failed to setup ssl %s : %s \r\n",
                            mbedtlsHighLevelCodeOrDefault( mbedTLSResult ),
                            mbedtlsLowLevelCodeOrDefault( mbedTLSResult ) ) );

                xResult = pdFALSE;
            }
        }

        if( xResult == pdTRUE )
        {
            mbedtls_ssl_set_bio( &pxContext->xMbedSslCtx,
                                 pxParams->pvCallerContext,
                                 pxParams->pxNetworkSend,
                                 pxParams->pxNetworkRecv,
                                 NULL );
        }
    }
    else
    {
        xResult = pdFALSE;
    }

    if( xResult == pdFALSE )
    {
        /* Cleanup context created. */
        if( pxContext != NULL )
        {
            prvFreeContext( pxContext );
            memset( pxContext, 0x00, sizeof( TLSContext_t ) );
        }
    }

    return xResult;
}


/*-----------------------------------------------------------*/

int32_t TLS_Connect( TLSContext_t * pxContext )
{
    int32_t result = 0;

    /* Negotiate. */
    while( 0 != ( result = mbedtls_ssl_handshake( &pxContext->xMbedSslCtx ) ) )
    {
        if( ( MBEDTLS_ERR_SSL_WANT_READ != result ) &&
            ( MBEDTLS_ERR_SSL_WANT_WRITE != result ) )
        {
            /* There was an unexpected error. Per mbedTLS API documentation,
             * ensure that upstream clean-up code doesn't accidentally use
             * a context that failed the handshake. */
            prvFreeContext( pxContext );

            LogError( ( "TLS handshake failed, error code = %d", result ) );

            break;
        }
    }

    return result;
}

/*-----------------------------------------------------------*/

int32_t TLS_Recv( TLSContext_t * pxContext,
                  unsigned char * pucReadBuffer,
                  size_t xReadLength )
{
    int32_t xResult = 0;
    size_t xRead = 0;

    if( NULL != pxContext )
    {
        /* This routine will return however many bytes are returned from from mbedtls_ssl_read
         * immediately unless MBEDTLS_ERR_SSL_WANT_READ is returned, in which case we try again. */
        do
        {
            xResult = mbedtls_ssl_read( &pxContext->xMbedSslCtx,
                                        pucReadBuffer + xRead,
                                        xReadLength - xRead );

            if( xResult > 0 )
            {
                /* Got data, so update the tally and keep looping. */
                xRead += ( size_t ) xResult;
            }

            /* If xResult == 0, then no data was received (and there is no error).
             * The secure sockets API supports non-blocking read, so stop the loop,
             * but don't flag an error. */
        } while( xResult == MBEDTLS_ERR_SSL_WANT_READ );
    }
    else
    {
        xResult = MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }

    if( xResult >= 0 )
    {
        xResult = ( int32_t ) xRead;
    }
    else
    {
        /* xResult < 0 is a hard error, so invalidate the context and stop. */
        prvFreeContext( pxContext );
    }

    return xResult;
}

/*-----------------------------------------------------------*/

int32_t TLS_Send( TLSContext_t * pxContext,
                  const unsigned char * pucMsg,
                  size_t xMsgLength )
{
    int32_t result = 0;
    size_t xWritten = 0;

    if( ( NULL != pxContext ) )
    {
        while( xWritten < xMsgLength )
        {
            result = mbedtls_ssl_write( &pxContext->xMbedSslCtx,
                                        pucMsg + xWritten,
                                        xMsgLength - xWritten );

            if( result > 0 )
            {
                /* Sent data, so update the tally and keep looping. */
                xWritten += ( size_t ) result;
            }
            else if( ( 0 == result ) || ( MBEDTLS_ERR_SSL_WANT_WRITE == result ) )
            {
                result = 0;
                break;
            }
            else
            {
                /* Hard error: invalidate the context and stop. */
                LogError( ( "Failed to send data with mbedtls error %s : %s \r\n",
                            mbedtlsHighLevelCodeOrDefault( result ),
                            mbedtlsLowLevelCodeOrDefault( result ) ) );
                prvFreeContext( pxContext );
                break;
            }
        }
    }
    else
    {
        result = MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }

    if( 0 <= result )
    {
        result = ( int32_t ) xWritten;
    }

    return result;
}

/*-----------------------------------------------------------*/

void TLS_Cleanup( TLSContext_t * pxContext )
{
    prvFreeContext( pxContext );
}
