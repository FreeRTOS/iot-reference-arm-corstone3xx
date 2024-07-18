/*
 * Amazon FreeRTOS V1.1.4
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
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
 * @file aws_pkcs11_config.h
 * @brief PCKS#11 config options.
 */


#ifndef _AWS_PKCS11_CONFIG_H_
#define _AWS_PKCS11_CONFIG_H_

#include <stdlib.h>

/**
 * @brief Malloc API used by core_pkcs11.h
 */
extern void * pvPortMalloc( size_t xWantedSize );
#define PKCS11_MALLOC    pvPortMalloc

/**
 * @brief Free API used by core_pkcs11.h
 */
extern void vPortFree( void * pv );
#define PKCS11_FREE    vPortFree

/* A non-standard version of C_INITIALIZE should be used by this port. */
/* #define pkcs11configC_INITIALIZE_ALT */

/**
 * @brief PKCS #11 default user PIN.
 *
 * The PKCS #11 standard specifies the presence of a user PIN. That feature is
 * sensible for applications that have an interactive user interface and memory
 * protections. However, since typical microcontroller applications lack one or
 * both of those, the user PIN is assumed to be used herein for interoperability
 * purposes only, and not as a security feature.
 */
#define configPKCS11_DEFAULT_USER_PIN                      "0000"

/**
 * @brief Maximum length (in characters) for a PKCS #11 CKA_LABEL
 * attribute.
 */
#define pkcs11configMAX_LABEL_LENGTH                       32

/**
 * @brief Maximum number of token objects that can be stored
 * by the PKCS #11 module.
 */
#define pkcs11configMAX_NUM_OBJECTS                        6

/**
 * @brief Set to 1 if a PAL destroy object is implemented.
 *
 * If set to 0, no PAL destroy object is implemented, and this functionality
 * is implemented in the common PKCS #11 layer.
 */
#define pkcs11configPAL_DESTROY_SUPPORTED                  0

/**
 * @brief Set to 1 if OTA image verification via PKCS #11 module is supported.
 *
 * If set to 0, OTA code signing certificate is built in via
 * aws_ota_codesigner_certificate.h.
 */
#define pkcs11configOTA_SUPPORTED                          0

/**
 * @brief Set to 1 if PAL supports storage for JITP certificate,
 * code verify certificate, and trusted server root certificate.
 *
 * If set to 0, PAL does not support storage mechanism for these, and
 * they are accessed via headers compiled into the code.
 */
#define pkcs11configJITP_CODEVERIFY_ROOT_CERT_SUPPORTED    0

/**
 * @brief The PKCS #11 label for device private key.
 *
 * Private key for connection to AWS IoT endpoint.  The corresponding
 * public key should be registered with the AWS IoT endpoint.
 */
#define pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS       "Device Priv TLS Key"

/**
 * @brief The PKCS #11 label for device public key.
 *
 * The public key corresponding to pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS.
 */
#define pkcs11configLABEL_DEVICE_PUBLIC_KEY_FOR_TLS        "Device Pub TLS Key"

/**
 * @brief The PKCS #11 label for the device certificate.
 *
 * Device certificate corresponding to pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS.
 */
#define pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS       "Device Cert"

/**
 * @brief The PKCS #11 label for the object to be used for code verification.
 *
 * Used by over-the-air update code to verify an incoming signed image.
 */
#define pkcs11configLABEL_CODE_VERIFICATION_KEY            "Code Verify Key"

/**
 * @brief The PKCS #11 label for Just-In-Time-Provisioning.
 *
 * The certificate corresponding to the issuer of the device certificate
 * (pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS) when using the JITR or
 * JITP flow.
 */
#define pkcs11configLABEL_JITP_CERTIFICATE                 "JITP Cert"

/**
 * @brief The PKCS #11 label for the AWS Trusted Root Certificate.
 *
 * @see aws_default_root_certificates.h
 */
#define pkcs11configLABEL_ROOT_CERTIFICATE                 "Root Cert"

/*
 * Define the key ID of the device keys which will be saved as
 * persistent keys in TF-M. The key ID servers as the a name.
 */
#ifdef PSA_CRYPTO_IMPLEMENTATION_TFM
    #define PSA_DEVICE_PRIVATE_KEY_ID    0x01
    #define PSA_DEVICE_PUBLIC_KEY_ID     0x10
#elif defined PSA_CRYPTO_IMPLEMENTATION_MBEDTLS

/* The PSA Crypto specification
 * https://arm-software.github.io/psa-api/crypto/1.1/api/keys/ids.html
 * defines the volatile key range as PSA_KEY_ID_VENDOR_MIN (0x40000000) to
 * PSA_KEY_ID_VENDOR_MAX (0x7fffffff). However, in the default PSA Crypto
 * configuration in Mbed TLS, volatile key range is defined by
 * PSA_KEY_ID_VOLATILE_MIN and PSA_KEY_ID_VOLATILE_MAX.
 */
    #define PSA_DEVICE_PRIVATE_KEY_ID    0x7FFFFFE0
#else
    #error "Missing PSA crypto implementation definition. Define either \
    `PSA_CRYPTO_IMPLEMENTATION_TFM` or `PSA_CRYPTO_IMPLEMENTATION_MBEDTLS`"
#endif

/* */
/* FIXME: are these needed? */
/* extern void *MPU_pvPortMalloc( size_t xSize ); */
/* extern void MPU_vPortFree( void *pv ); */

/* #define pvPortMalloc             MPU_pvPortMalloc */
/* #define vPortFree                MPU_vPortFree */

/**
 * @brief The PKCS #11 label for the object to be used for CMAC operations.
 * It can be used by tasks during setting up the PKCS11 object for AES CMAC
 * operations.
 */
#define pkcs11configLABEL_CMAC_KEY    "CMAC Key"

#endif /* _AWS_PKCS11_CONFIG_H_ include guard. */
