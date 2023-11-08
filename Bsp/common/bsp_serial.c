/* Copyright 2017-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include "device_cfg.h"
#include "Driver_USART.h"
#include "bsp_serial.h"

extern ARM_DRIVER_USART Driver_USART0;

void bsp_serial_init( void )
{
    Driver_USART0.Initialize( NULL );
    Driver_USART0.Control( ARM_USART_MODE_ASYNCHRONOUS, DEFAULT_UART_BAUDRATE );
}

void bsp_serial_print( char * str )
{
    ( void ) Driver_USART0.Send( str, strlen( str ) );
}

#if defined( __ARMCOMPILER_VERSION )

/* Retarget armclang, which requires all IO system calls to be overridden together. */

    #include <rt_sys.h>

    #define STDIN_FILENO     0
    #define STDOUT_FILENO    1
    #define STDERR_FILENO    2

    FILEHANDLE _sys_open( const char * name,
                          int openmode )
    {
        if( name == NULL )
        {
            return -1;
        }

        /* By default, the Arm Compiler uses the special file path ":tt" for stdin, */
        /* stdout and stderr and distinguishes between them using openmode. For details, */
        /* see https://github.com/ARM-software/abi-aa/blob/2022Q1/semihosting/semihosting.rst#sys-open-0x01 */
        if( strcmp( name, ":tt" ) == 0 )
        {
            if( openmode & OPEN_W )
            {
                return STDOUT_FILENO;
            }

            if( openmode & OPEN_A )
            {
                return STDERR_FILENO;
            }

            return STDIN_FILENO;
        }

        return -1;
    }

    int _sys_close( FILEHANDLE fh )
    {
        /* Not implemented */
        ( void ) fh;
        return -1;
    }

    int _sys_write( FILEHANDLE fd,
                    const unsigned char * str,
                    unsigned int len,
                    int mode )
    {
        /* From <rt_sys.h>: `mode' exists for historical reasons and must be ignored. */
        ( void ) mode;

        if( ( fd != STDOUT_FILENO ) && ( fd != STDERR_FILENO ) )
        {
            return -1;
        }

        if( Driver_USART0.Send( str, len ) != ARM_DRIVER_OK )
        {
            return -1;
        }

        return 0;
    }

    int _sys_read( FILEHANDLE fd,
                   unsigned char * str,
                   unsigned int len,
                   int mode )
    {
        /* From <rt_sys.h>: `mode' exists for historical reasons and must be ignored. */
        ( void ) mode;

        /* Not implemented */
        ( void ) str;
        ( void ) len;
        return -1;
    }

    int _sys_istty( FILEHANDLE fh )
    {
        /* Not implemented */
        ( void ) fh;
        return 0;
    }

    long _sys_flen( FILEHANDLE fh )
    {
        /* Not implemented */
        ( void ) fh;
        return -1;
    }

    int _sys_seek( FILEHANDLE fh,
                   long offset )
    {
        /* Not implemented */
        ( void ) fh;
        ( void ) offset;
        return -1;
    }

#else /* !defined(__ARMCOMPILER_VERSION) */

/* Redirects gcc printf to UART0 */
    int _write( int fd,
                char * str,
                int len )
    {
        if( Driver_USART0.Send( str, len ) == ARM_DRIVER_OK )
        {
            return len;
        }

        return 0;
    }

#endif /* if defined( __ARMCOMPILER_VERSION ) */
