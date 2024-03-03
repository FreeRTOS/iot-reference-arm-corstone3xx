/*
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2016-2024, Arm Limited. All rights reserved.
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

#include "acamera_command_api.h"
#include "acamera_firmware_settings.h"
#include "acamera_logger.h"
#include "acamera_sensor_api.h"

extern uint32_t get_calibrations_static_linear_dummy( ACameraCalibrations * c );
extern uint32_t get_calibrations_static_fs_lin_dummy( ACameraCalibrations * c );
extern uint32_t get_calibrations_static_native_dummy( ACameraCalibrations * c );
extern uint32_t get_calibrations_dynamic_linear_dummy( ACameraCalibrations * c );
extern uint32_t get_calibrations_dynamic_fs_lin_dummy( ACameraCalibrations * c );
extern uint32_t get_calibrations_dynamic_native_dummy( ACameraCalibrations * c );

uint32_t get_calibrations_dummy( uint32_t ctx_id,
                                 void * sensor_arg,
                                 ACameraCalibrations * c )
{
    uint8_t ret = 0;

    if( !sensor_arg )
    {
        LOG( LOG_ERR, "calibration sensor_arg is NULL" );
        return ret;
    }

    int32_t preset = ( ( sensor_mode_t * ) sensor_arg )->wdr_mode;

    /* logic which calibration to apply */
    switch( preset )
    {
        case WDR_MODE_LINEAR:
            LOG( LOG_DEBUG, "calibration switching to WDR_MODE_LINEAR %d ", ( int ) preset );
            ret += ( get_calibrations_dynamic_linear_dummy( c ) + get_calibrations_static_linear_dummy( c ) );
            break;

        case WDR_MODE_NATIVE:
            LOG( LOG_DEBUG, "calibration switching to WDR_MODE_NATIVE %d ", ( int ) preset );
            ret += ( get_calibrations_dynamic_native_dummy( c ) + get_calibrations_static_native_dummy( c ) );
            break;

        case WDR_MODE_FS_LIN:
            LOG( LOG_DEBUG, "calibration switching to WDR mode on mode %d ", ( int ) preset );
            ret += ( get_calibrations_dynamic_fs_lin_dummy( c ) + get_calibrations_static_fs_lin_dummy( c ) );
            break;

        default:
            LOG( LOG_DEBUG, "calibration defaults to WDR_MODE_LINEAR %d ", ( int ) preset );
            ret += ( get_calibrations_dynamic_linear_dummy( c ) + get_calibrations_static_linear_dummy( c ) );
            break;
    }

    return ret;
}
