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

#ifndef __ACAMERA_INTERFACE_CONFIG_H__
#define __ACAMERA_INTERFACE_CONFIG_H__

extern volatile uint32_t STREAM_ENABLED;

#if 0
    #define FVP_SENSOR_WIDTH     1920
    #define FVP_SENSOR_HEIGHT    1080
#else
    #define FVP_SENSOR_WIDTH     576
    #define FVP_SENSOR_HEIGHT    576
#endif
/* 0:RGGB 2:GBRG */
#define FVP_SENSOR_RGGB_START    2
#define FVP_SENSOR_DEPTH         12
#define FVP_SENSOR_TWIDTH        FVP_SENSOR_WIDTH
#define FVP_SENSOR_THEIGHT       FVP_SENSOR_HEIGHT

#endif /* ifndef __ACAMERA_INTERFACE_CONFIG_H__ */
