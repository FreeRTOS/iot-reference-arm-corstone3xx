/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#include "psa/crypto.h"
#include "psa/crypto_types.h"

DEFINE_FAKE_VALUE_FUNC( psa_status_t,
                        psa_generate_random,
                        uint8_t *,
                        size_t );
