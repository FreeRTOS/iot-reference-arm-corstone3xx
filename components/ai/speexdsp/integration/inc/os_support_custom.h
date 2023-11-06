/* Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#define OVERRIDE_SPEEX_FATAL
#define OVERRIDE_SPEEX_WARNING
#define OVERRIDE_SPEEX_WARNING_INT
#define OVERRIDE_SPEEX_NOTIFY

static inline void _speex_fatal( const char * str,
                                 const char * file,
                                 int line )
{
    printf( "Fatal (internal) error in %s, line %d: %s\r\n", file, line, str );
}

static inline void speex_warning( const char * str )
{
    printf( "warning: %s\r\n", str );
}

static inline void speex_warning_int( const char * str,
                                      int val )
{
    printf( "warning: %s %d\r\n", str, val );
}

static inline void speex_notify( const char * str )
{
    printf( "notification: %s\r\n", str );
}
