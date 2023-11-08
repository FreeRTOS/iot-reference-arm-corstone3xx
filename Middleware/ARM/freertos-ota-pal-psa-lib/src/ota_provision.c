/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

/* Key provisioning include. */
#include "ota_provision.h"

/* This is the public key which is derivated from ./bl2/ext/mcuboot/root-RSA-3072_1.pem.
 * If you used a different key to sign the image, then please replace the values here
 * with your public key.*/

static const char cOTARSAPublicKey[] =
    "-----BEGIN PUBLIC KEY-----\n"
    "MIIBojANBgkqhkiG9w0BAQEFAAOCAY8AMIIBigKCAYEAv7ewn+jI0f4WHVOHl3kc\n"
    "FceZFmzKuC3Kwg1i+euP6ToYQ0fXu9VivOMzY6ejqFzzI3j9LQchH7lUcCipCNpQ\n"
    "fp6OzGhOf0gN6ifoxu+tX51GSrxpmjBfO8FSkvi8ddQ8J3BAAKYuKH9Z5WBDEdwx\n"
    "CX3PL0E/tlIao0kW8rWznDz7XiwfIoa9rr42Ur3E8FhpNqeAPoGzVJjkXZXtIfC6\n"
    "riH7xBmHVdErTwDYQVjL26maU+lsZ8t8XfaRBnVS8sB+sWtdMEBAL9gelTwFl3/w\n"
    "BPBOLNU5DpQ9fAMIHQkI8o1EDc+zlj1aduj27pNk6FfR4vULGGlv6eE9+IlJKOav\n"
    "uKjGQlUtwduMXbJtf/4m6nXZ/R/cIjukG6et63HfvbQ30eu+CBAceIQcmnXEreXv\n"
    "cxesaXi81jeMDBQhBke9+AqsGQmdDR1y4T4adOqG2VxKzczGlKf+2guHEbtr8Drj\n"
    "T4JPseSkzbxwPJ2cSfnPKG242m99OFdVQypzjbYY/XCnAgMBAAE=\n"
    "-----END PUBLIC KEY-----";

/* This function can be found in libraries/3rdparty/mbedtls_utils/mbedtls_utils.c. */
extern int convert_pem_to_der( const unsigned char * pucInput,
                               size_t xLen,
                               unsigned char * pucOutput,
                               size_t * pxOlen );

int ota_privision_code_signing_key( psa_key_handle_t * key_handle )
{
    uint8_t public_key_der[ 512 ];
    size_t xLength = 512;
    int result;
    psa_key_handle_t key_handle_tmp = 0;
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    result = convert_pem_to_der( ( const unsigned char * ) cOTARSAPublicKey,
                                 sizeof( cOTARSAPublicKey ),
                                 public_key_der,
                                 &xLength );

    if( result != 0 )
    {
        return result;
    }

    psa_set_key_usage_flags( &attributes, PSA_KEY_USAGE_VERIFY_HASH );
    psa_set_key_algorithm( &attributes, PSA_ALG_RSA_PSS_ANY_SALT( PSA_ALG_SHA_256 ) );
    psa_set_key_type( &attributes, PSA_KEY_TYPE_RSA_PUBLIC_KEY );
    psa_set_key_bits( &attributes, 3072 );
    status = psa_import_key( &attributes, ( const uint8_t * ) public_key_der, xLength, &key_handle_tmp );

    if( status == PSA_SUCCESS )
    {
        *key_handle = key_handle_tmp;
    }

    return status;
}
