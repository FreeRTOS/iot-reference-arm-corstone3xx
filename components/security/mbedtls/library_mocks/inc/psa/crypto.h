/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#ifndef PSA_CRYPTO_H
#define PSA_CRYPTO_H

#include "fff.h"

#include "crypto_types.h"
#include <stddef.h>
#include <stdint.h>

DECLARE_FAKE_VALUE_FUNC( psa_status_t, psa_generate_random, uint8_t *, size_t );

#endif /* PSA_CRYPTO_H */
