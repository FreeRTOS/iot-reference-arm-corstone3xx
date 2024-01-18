/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */
#ifndef SNTP_CLIENT_TASK_H
#define SNTP_CLIENT_TASK_H

#include "mbedtls/platform_time.h"

/**
 * @brief Utility macro to convert milliseconds to the fractions value of an SNTP timestamp.
 * @note The fractions value MUST be less than 1000 as duration of seconds is not represented
 * as fractions part of SNTP timestamp.
 */
#define MILLISECONDS_TO_SNTP_FRACTIONS( ms )    ( ms * 1000 * SNTP_FRACTION_VALUE_PER_MICROSECOND )

/**
 * @brief Type representing system time in Coordinated Universal Time (UTC)
 * zone as time since 1st January 1970 00h:00m:00s.
 *
 * @note This task uses RAM-based mathematical model to represent UTC time
 * in system.
 */
typedef struct UTCTime
{
    uint32_t secs;
    uint32_t msecs;
} UTCTime_t;

/**
 * @brief Utility function to print the system time as human-readable time (in the
 * YYYY-MM-DD dd:mm:ss format).
 *
 * @param[in] pTime The system time to be printed.
 */
void vPrintTime( const mbedtls_time_t * pTime );

/**
 * @brief Initializes the system clock with the first second of the year (i.e. at midnight
 * of 1st January) that is configured in the democonfigSYSTEM_START_YEAR config of
 * core_sntp_config.h file.
 */
void initializeSystemClock( void );

/**
 * @brief Used by application to query wall-clock
 * time as unsigned integer seconds from the system.
 *
 * @param[out] mbedtls_time_t This will be populated with the current time
 * in the system as total time since 1st January 1900 00h:00m:00s.
 */
mbedtls_time_t systemGetWallClockTime( mbedtls_time_t * pTime );

/**
 * @brief Sntp Client Task that is responsible for synchronizing system time with the time servers
 * periodically.
 */
void vStartSntpClientTask( void );

#endif /* SNTP_CLIENT_TASK_H */
