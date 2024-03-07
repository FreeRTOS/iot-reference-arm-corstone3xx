/* Copyright 2024, Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "acamera_logger.h"
#include "acamera_types.h"
#include "application_command_api.h"

#include "isp_config.h"

pvFrameReadyHandler_t pvDownscaledFrameReady = NULL;
pvFrameReadyHandler_t pvFullResolutionFrameReady = NULL;

static uint32_t ulCoherentDmaAllocatedSize = 0;
typedef uint8_t ucFrameBuffer_t[ isp_configMAX_OUTPUT_FRAME_SIZE ];
static ucFrameBuffer_t * xFullResolutionBuffer = ( ucFrameBuffer_t * ) isp_configFULL_RESOLUTION_BUFFER_BASE;
static ucFrameBuffer_t * xDownscaledBuffer = ( ucFrameBuffer_t * ) isp_configDOWNSCALED_BUFFER_BASE;

/* This is for FVP stream optimisation */
#include "FreeRTOS.h"
#include "semphr.h"
extern SemaphoreHandle_t xStreamSemaphore;

/* Only 1 allocation is supported, it is for the Temper Frame */
void * pvCallbackDmaAllocCoherent( uint32_t ulContextId,
                                   uint64_t ullSize,
                                   uint64_t * ullDmaAddress )
{
    uint32_t ulAddress;

    if( ulCoherentDmaAllocatedSize + ullSize > isp_configCOHERENT_DMA_MEMORY_SIZE )
    {
        LOG( LOG_ERR, "Not enough memory" );
        *ullDmaAddress = 0;
        return NULL;
    }

    ulAddress = isp_configCOHERENT_DMA_MEMORY_BASE + ulCoherentDmaAllocatedSize;
    ulCoherentDmaAllocatedSize += ullSize;

    *ullDmaAddress = ulAddress;

    /* compute bus address */
    *ullDmaAddress -= ISP_SOC_DMA_BUS_OFFSET;

    LOG( LOG_DEBUG,
         "DMA alloc: 0x%x, Memory left: 0x%x",
         ( uint32_t ) ullSize,
         isp_configCOHERENT_DMA_MEMORY_SIZE - ulCoherentDmaAllocatedSize );
    return ( void * ) ulAddress;
}

/* Only 1 allocation is supported, it is for the Temper Frame */
void vCallbackDmaFreeCoherent( uint32_t ulContextId,
                               uint64_t ullSize,
                               void * pvVirtualAddress,
                               uint64_t ullDmaAddress )
{
    if( ( ( uint32_t ) pvVirtualAddress ) != isp_configCOHERENT_DMA_MEMORY_SIZE )
    {
        LOG( LOG_ERR, "DMA free: Trying to fr  ee unknown memory: 0x%x", ( uint32_t ) pvVirtualAddress );
    }

    ulCoherentDmaAllocatedSize = 0;
}

int32_t lCallbackStreamGetFrame( uint32_t ulContextId,
                                 acamera_stream_type_t xType,
                                 aframe_t * pxAFrames,
                                 uint64_t ullNumPlanes )
{
    static uint32_t ulDownscaledFrameCounter = 0;
    uint32_t ulAddress = 0;

    while( ullNumPlanes > 1 )
    {
        ullNumPlanes--;
        pxAFrames[ ullNumPlanes ].address = 0;
        pxAFrames[ ullNumPlanes ].status = dma_buf_purge;
    }

    if( xType == ACAMERA_STREAM_FR )
    {
        static uint32_t ulFullResolutionFrameCounter = 0;
        ulAddress = ( uint32_t ) xFullResolutionBuffer[ ulFullResolutionFrameCounter % isp_configMAX_BUFFERED_FRAMES ];
        ulFullResolutionFrameCounter++;
    }
    else if( xType == ACAMERA_STREAM_DS1 )
    {
        ulAddress = ( uint32_t ) xDownscaledBuffer[ ulDownscaledFrameCounter % isp_configMAX_BUFFERED_FRAMES ];
        ulDownscaledFrameCounter++;
    }

    pxAFrames[ 0 ].address = ulAddress;

    if( ulAddress == 0 )
    {
        LOG( LOG_ERR, "No buffer available!" );
        pxAFrames[ 0 ].status = dma_buf_purge;
        return -1;
    }

    pxAFrames[ 0 ].status = dma_buf_empty;
    return 0;
}

/* Return pixel width in bits for format. */
static inline uint32_t prvGetPixelWidth( uint32_t ulFormat )
{
    uint32_t ulResult = 32;

    switch( ulFormat )
    {
        case DMA_FORMAT_RGB24:
            ulResult = 24;
            break;

        case DMA_FORMAT_RGB565:
        case DMA_FORMAT_RAW16:
        case DMA_FORMAT_YUY2:
        case DMA_FORMAT_UYVY:
            ulResult = 16;
            break;

        case DMA_FORMAT_RAW12:
            ulResult = 12;
            break;

        case DMA_FORMAT_NV12_Y:
        case DMA_FORMAT_NV12_UV:
        case DMA_FORMAT_NV12_VU:
        case DMA_FORMAT_YV12_Y:
        case DMA_FORMAT_YV12_U:
        case DMA_FORMAT_YV12_V:
            ulResult = 8;
            break;
    }

    return ulResult;
}

int32_t lCallbackStreamPutFrame( uint32_t ulContextId,
                                 acamera_stream_type_t xType,
                                 aframe_t * pxAFrames,
                                 uint64_t ullNumPlanes )
{
    uint32_t ulAddress;
    uint32_t ulWidth;
    uint32_t ulHeight;
    uint32_t ulBitsPerPixel;
    uint32_t ulFormat;
    uint32_t ulBitsPerPixelFormat;

    /* Note: We only care about the first plane */
    LOG( LOG_CRIT, "\033[1;33m-- aframes->frame_id = %d --\033[1;0m", pxAFrames->frame_id );

    while( ullNumPlanes > 0 )
    {
        ullNumPlanes--;
        pxAFrames[ ullNumPlanes ].status = dma_buf_purge;
    }

    if( pxAFrames->address )
    {
        ulAddress = pxAFrames->address;
        ulWidth = pxAFrames->width;
        ulHeight = pxAFrames->height;
        ulBitsPerPixel = pxAFrames->size / ulWidth / ulHeight;

        LOG( LOG_CRIT, "\033[1;33m-- %u X %u @ %u bytes per pixel --\033[1;0m", ulWidth, ulHeight, ulBitsPerPixel );
    }

    if( xType == ACAMERA_STREAM_DS1 )
    {
        application_command( TIMAGE, DS1_FORMAT_BASE_PLANE_ID, 0, COMMAND_GET, &ulFormat );
        ulBitsPerPixelFormat = _get_pixel_width( ulFormat ) / 8;

        if( ulBitsPerPixel != ulBitsPerPixelFormat )
        {
            /* Current frame and current format mismatch */
            /* Default to 2 bytes = RGB565, otherwise RGB32 */
            ulFormat = ( ulBitsPerPixel == 2 ) ? DMA_FORMAT_RGB565 : DMA_FORMAT_RGB32;
            LOG( LOG_NOTICE, "Bits per pixel mismatch! Expected: %u. Using format: 0x%x", ulBitsPerPixelFormat * 8, ulFormat );
        }

        if( pvDownscaledFrameReady != NULL )
        {
            pvDownscaledFrameReady( ulAddress, ulWidth, ulHeight, ulFormat, pxAFrames->frame_id );
        }

        /* Let stream thread start the next frame */
        xSemaphoreGive( xStreamSemaphore );
    }
    else if( xType == ACAMERA_STREAM_FR )
    {
        application_command( TIMAGE, FR_FORMAT_BASE_PLANE_ID, 0, COMMAND_GET, &ulFormat );
        ulBitsPerPixelFormat = _get_pixel_width( ulFormat ) / 8;

        if( ulBitsPerPixel != ulBitsPerPixelFormat )
        {
            ulFormat = ( ulBitsPerPixel == 2 ) ? DMA_FORMAT_RGB565 : DMA_FORMAT_RGB32;
            LOG( LOG_NOTICE, "Bits per pixel mismatch! Expected: %u. Using format: 0x%x", ulBitsPerPixelFormat * 8, ulFormat );
        }

        if( pvFullResolutionFrameReady != NULL )
        {
            pvFullResolutionFrameReady( ulAddress, ulWidth, ulHeight, ulFormat, pxAFrames->frame_id );
        }
    }

    return 0;
}
