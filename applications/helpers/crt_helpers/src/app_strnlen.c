/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * This file is based on
 * https://git.trustedfirmware.org/plugins/gitiles/TF-M/trusted-firmware-m.git/+/c9352b59f2a501b5af3f648b3fc91065993c002f/secure_fw/partitions/lib/runtime/crt_strnlen.c
 */

#include <stddef.h>

size_t app_strnlen( const char * s,
                    size_t maxlen )
{
    size_t idx;

    if( s == NULL )
    {
        return 0;
    }

    for( idx = 0; idx < maxlen; idx++ )
    {
        if( s[ idx ] == '\0' )
        {
            return idx;
        }
    }

    return idx;
}
