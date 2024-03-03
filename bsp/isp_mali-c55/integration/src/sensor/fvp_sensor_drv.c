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

#include "acamera_command_api.h"
#include "acamera_firmware_config.h"
#include "acamera_interface_config.h"
#include "acamera_logger.h"
#include "acamera_sensor_api.h"
#include "acamera_types.h"

typedef struct _sensor_context_t
{
    uint8_t address; /* Sensor address for direct write (not used currently) */
    sensor_param_t param;
    sensor_mode_t supported_modes[ ISP_MAX_SENSOR_MODES ];
} sensor_context_t;

static sensor_context_t s_ctx[ FIRMWARE_CONTEXT_NUMBER ];
static int ctx_counter = 0;
volatile uint32_t STREAM_ENABLED = 0;

static int32_t sensor_alloc_analog_gain( void * ctx,
                                         int32_t gain )
{
    /* This is const for the emulated sensor */
    return 0;
}

static int32_t sensor_alloc_digital_gain( void * ctx,
                                          int32_t gain )
{
    /* This is const for the emulated sensor */
    return 0;
}

static void sensor_alloc_integration_time( void * ctx,
                                           uint16_t * int_time,
                                           uint16_t * int_time_M,
                                           uint16_t * int_time_L )
{
    /* This is const for the emulated sensor */
}

static void sensor_update( void * ctx )
{
}

static void sensor_set_mode( void * ctx,
                             uint8_t mode )
{
    sensor_context_t * p_ctx = ctx;
    sensor_param_t * param = &p_ctx->param;

    param->active.width = param->modes_table[ mode ].resolution.width;
    param->active.height = param->modes_table[ mode ].resolution.height;
    param->total.width = param->modes_table[ mode ].resolution.width;
    param->total.height = param->modes_table[ mode ].resolution.height;
    param->pixels_per_line = param->total.width;
    param->integration_time_min = 1;
    param->integration_time_max = 1000;
    param->integration_time_limit = 1000;
    param->mode = mode;
    param->lines_per_second = 0;
    param->sensor_exp_number = param->modes_table[ mode ].exposures;
}

static uint16_t sensor_get_id( void * ctx )
{
    return 0xFFFF;
}

static const sensor_param_t * sensor_get_parameters( void * ctx )
{
    sensor_context_t * p_ctx = ctx;

    return ( const sensor_param_t * ) &p_ctx->param;
}

static void sensor_disable_isp( void * ctx )
{
}

static uint32_t read_register( void * ctx,
                               uint32_t address )
{
    return 0;
}

static void write_register( void * ctx,
                            uint32_t address,
                            uint32_t data )
{
}

static void stop_streaming( void * ctx )
{
    STREAM_ENABLED = 0;
}

static void start_streaming( void * ctx )
{
    STREAM_ENABLED = 1;
}

void fvp_sensor_deinit( void * ctx )
{
}

/*--------------------Initialization------------------------------------------------------------ */
void fvp_sensor_init( void ** ctx,
                      sensor_control_t * ctrl )
{
    if( ctx_counter < FIRMWARE_CONTEXT_NUMBER )
    {
        sensor_context_t * p_ctx = &s_ctx[ ctx_counter ];
        ctx_counter++;

        p_ctx->param.rggb_start = FVP_SENSOR_RGGB_START;
        p_ctx->param.sensor_exp_number = 1;
        p_ctx->param.again_log2_max = 0;
        p_ctx->param.dgain_log2_max = 0;
        p_ctx->param.integration_time_apply_delay = 2;
        p_ctx->param.isp_exposure_channel_delay = 0;
        p_ctx->param.modes_table = p_ctx->supported_modes;
        p_ctx->param.modes_num = 1;

        p_ctx->supported_modes[ 0 ].wdr_mode = WDR_MODE_LINEAR;
        p_ctx->supported_modes[ 0 ].fps = 1 * 256;
        p_ctx->supported_modes[ 0 ].resolution.width = FVP_SENSOR_WIDTH;
        p_ctx->supported_modes[ 0 ].resolution.height = FVP_SENSOR_HEIGHT;
        p_ctx->supported_modes[ 0 ].bits = FVP_SENSOR_DEPTH;
        p_ctx->supported_modes[ 0 ].exposures = 1;

        p_ctx->param.sensor_ctx = &s_ctx;

        *ctx = p_ctx;

        ctrl->alloc_analog_gain = sensor_alloc_analog_gain;
        ctrl->alloc_digital_gain = sensor_alloc_digital_gain;
        ctrl->alloc_integration_time = sensor_alloc_integration_time;
        ctrl->sensor_update = sensor_update;
        ctrl->set_mode = sensor_set_mode;
        ctrl->get_id = sensor_get_id;
        ctrl->get_parameters = sensor_get_parameters;
        ctrl->disable_sensor_isp = sensor_disable_isp;
        ctrl->read_sensor_register = read_register;
        ctrl->write_sensor_register = write_register;
        ctrl->start_streaming = start_streaming;
        ctrl->stop_streaming = stop_streaming;
    }
    else
    {
        LOG( LOG_ERR, "Attempt to initialize more sensor instances than was configured. Sensor initialization failed." );
        *ctx = NULL;
    }
}

/************************************************************************************** */
