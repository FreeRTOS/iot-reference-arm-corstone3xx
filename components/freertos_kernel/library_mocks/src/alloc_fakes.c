/* Copyright 2025 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "alloc_fakes.h"

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC( void *, test_malloc, size_t );
DEFINE_FAKE_VALUE_FUNC( void *, test_calloc, size_t, size_t );
DEFINE_FAKE_VOID_FUNC( test_free, void * );

void * __wrap_malloc( size_t size )
{
    return test_malloc( size );
}
void * __wrap_calloc( size_t num,
                      size_t size )
{
    return test_calloc( num, size );
}
void __wrap_free( void * ptr )
{
    test_free( ptr );
}
