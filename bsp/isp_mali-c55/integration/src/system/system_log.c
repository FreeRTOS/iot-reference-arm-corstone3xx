/*
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 * - Neither the name of ARM nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "system_log.h"
#include "acamera_firmware_config.h"
#include "system_timer.h"

/* debug log names for level */
const char * const log_level_name[ SYSTEM_LOG_LEVEL_MAX ] = { "DEBUG", "INFO", "NOTICE", "WARNING", "ERR", "CRIT" };
/* debug log names for modules */
const char * const log_module_name[ SYSTEM_LOG_MODULE_MAX ] = FSM_NAMES;

const char * sys_time_log_cb( void )
{
    static char buffer[ 18 ];
    uint32_t t_ms, t_sec, t_min, t_hour, t_day;

    t_ms = ( uint32_t ) ( ( ( uint64_t ) system_timer_timestamp() ) * 1000 / system_timer_frequency() );
    t_sec = t_ms / 1000;
    t_ms %= 1000;
    t_min = t_sec / 60;
    t_sec %= 60;
    t_hour = t_min / 60;
    t_min %= 60;
    t_day = t_hour / 24;
    t_hour %= 24;
    /* ddTHH:mm:ss.SSSZ */
    /* With the 32 bit timestamp, time will overflow around 49 days and 17 hours */
    snprintf( buffer, 18, "%02uT%02u:%02u:%02u.%03uZ ", t_day, t_hour, t_min, t_sec, t_ms );
    return buffer;
}
