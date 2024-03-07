/* Copyright 2024, Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "acamera_firmware_api.h"
#include "acamera_sensor_api.h"
#include "acamera_types.h"

#include "platform_base_address.h"

#include "logging_levels.h"

/* Logging configuration for the MQTT library. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME    "ISP"
#endif

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif

#include "logging_stack.h"

#define isp_configCOHERENT_DMA_MEMORY_BASE       DDR4_BLK2_BASE_NS

#define isp_configMAX_FRAME_WIDTH                2048
#define isp_configMAX_FRAME_HEIGHT               1080
#define isp_configMAX_BITS_PER_PIXEL             20
#define isp_configMAX_BYTES_PER_PIXEL            4
#define isp_configMAX_INPUT_FRAME_SIZE           ( isp_configMAX_FRAME_WIDTH * isp_configMAX_FRAME_HEIGHT * isp_configMAX_BITS_PER_PIXEL / 8 )
#define isp_configMAX_OUTPUT_FRAME_SIZE          ( isp_configMAX_FRAME_WIDTH * isp_configMAX_FRAME_HEIGHT * isp_configMAX_BYTES_PER_PIXEL )

/* 5400KB */
#define isp_configCOHERENT_DMA_MEMORY_SIZE       isp_configMAX_INPUT_FRAME_SIZE

#define isp_configMAX_BUFFERED_FRAMES            4
#define isp_configFULL_RESOLUTION_BUFFER_BASE    ( isp_configCOHERENT_DMA_MEMORY_BASE + isp_configCOHERENT_DMA_MEMORY_SIZE )
#define isp_configDOWNSCALED_BUFFER_BASE         ( isp_configFULL_RESOLUTION_BUFFER_BASE + ( isp_configMAX_OUTPUT_FRAME_SIZE * isp_configMAX_BUFFERED_FRAMES ) )

struct FullResolutionImage
{
    uint32_t ulMode;
    uint32_t ulWidth;
    uint32_t ulHeight;
};

struct LastFrame
{
    uint32_t ulDisplayed;
    uint32_t ulInferred;
    struct FullResolutionImage xFullResolution;
};

typedef void (* pvFrameReadyHandler_t)( uint32_t ulAddress,
                                        uint32_t ulWidth,
                                        uint32_t ulHeight,
                                        uint32_t ulMode,
                                        uint32_t ulFrameId );

extern pvFrameReadyHandler_t pvDownscaledFrameReady;
extern pvFrameReadyHandler_t pvFullResolutionFrameReady;

extern void fvp_sensor_init( void ** ppvContext,
                             sensor_control_t * );
extern void fvp_sensor_deinit( void * pvContext );
extern uint32_t get_calibrations_dummy( uint32_t ulContextNumber,
                                        void * pvSensorArg,
                                        ACameraCalibrations * );

extern void * pvCallbackDmaAllocCoherent( uint32_t ulContextId,
                                          uint64_t ullSize,
                                          uint64_t * pullDmaAddress );
extern void vCallbackDmaFreeCoherent( uint32_t ulContextId,
                                      uint64_t ullSize,
                                      void * pvVirtualAddress,
                                      uint64_t ullDmaAddress );

/* The ISP pipeline can have several outputs such as Full Resolution, DownScaler1, DownScaler2 etc */
/* It is possible to set up the firmware to return the metadata for each output frame from */
/* the specific channel. This callbacks must be set in acamera_settings structure and passed to the firmware in */
/* acamera_init api function */
/* The context id can be used to differentiate contexts */
extern int32_t lCallbackStreamGetFrame( uint32_t ulContextId,
                                        acamera_stream_type_t xType,
                                        aframe_t * pxAFrames,
                                        uint64_t ullNumPlanes );
extern int32_t lCallbackStreamPutFrame( uint32_t ulContextId,
                                        acamera_stream_type_t xType,
                                        aframe_t * pxAFrames,
                                        uint64_t ullNumPlanes );

void vStartISPDemo();
