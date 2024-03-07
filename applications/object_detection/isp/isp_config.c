/* Copyright 2024, Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include CMSIS_device_header

#include "device_cfg.h"
#include "device_definition.h"
#include "hdlcd_drv.h"
#include "hdlcd_helper.h"
#include "stdint.h"
#include <stddef.h>
#include <stdio.h>

#include "arm_2d.h"

#include "isp_config.h"

#include "ml_interface.h"

/*
 * Semihosting is a mechanism that enables code running on an ARM target
 * to communicate and use the Input/Output facilities of a host computer
 * that is running a debugger.
 * There is an issue where if you use armclang at -O0 optimisation with
 * no parameters specified in the main function, the initialisation code
 * contains a breakpoint for semihosting by default. This will stop the
 * code from running before main is reached.
 * Semihosting can be disabled by defining __ARM_use_no_argv symbol
 * (or using higher optimization level).
 */
#if defined( __ARMCC_VERSION ) && ( __ARMCC_VERSION >= 6010050 )
    __asm( "  .global __ARM_use_no_argv\n" );
#endif

#define isp_configHIGHLIGHTED_FRAME_WIDTH       2

#define isp_configMAX_BORDER_WIDTH              320
#define isp_configMAX_BORDER_HEIGHT             240
#define isp_configMAX_BORDER_BYTES_PER_PIXEL    4
#define isp_configBORDER_BUFFER_SIZE            ( isp_configMAX_BORDER_WIDTH * isp_configMAX_BORDER_HEIGHT * isp_configMAX_BORDER_BYTES_PER_PIXEL )

#define isp_configMAX_DETECT_RESULTS            10

/* This buffer is used by FP unit, align it to prevent unaligned FP faults */
static uint8_t ucBorderTileBuffer[ isp_configBORDER_BUFFER_SIZE ] __ALIGNED( 4 );
static volatile uint8_t ucFlag = 0;

static struct LastFrame xLastFrame =
{
    .ulDisplayed     = 0,
    .ulInferred      = 0,
    .xFullResolution =
    {
        .ulMode      = HDLCD_PIXEL_FORMAT_NOT_SUPPORTED,
        .ulWidth     = 0,
        .ulHeight    = 0
    }
};

struct arm_2d_tile_t xRootTile =
{
    .tInfo                  =
    {
        .bIsRoot            = true,
        .bHasEnforcedColour = true,
    },
};

#define isp_configMAX_INFER_FRAME_WIDTH     192
#define isp_configMAX_INFER_FRAME_HEIGHT    192
#define isp_configMAX_INFER_FRAME_SIZE      ( isp_configMAX_INFER_FRAME_WIDTH * isp_configMAX_INFER_FRAME_HEIGHT )
uint8_t ucGrayBuffer[ isp_configMAX_INFER_FRAME_SIZE ] __attribute__( ( aligned( 32 ) ) );
static void prvHdlcdShow( uint32_t address,
                          uint32_t width,
                          uint32_t height,
                          uint32_t mode );
static void prvHandleFullResolutionFrame( uint32_t ulAddress,
                                          uint32_t ulWidth,
                                          uint32_t ulHeight,
                                          uint32_t ulMode,
                                          uint32_t ulFrameId );
static void prvHandleDownscaledFrame( uint32_t ulAddress,
                                      uint32_t ulWidth,
                                      uint32_t ulHeight,
                                      uint32_t ulMode,
                                      uint32_t ulFrameId );
static void prvDrawFrameOnTile( struct arm_2d_tile_t * pxDest,
                                struct arm_2d_region_t * pxFrame,
                                enum hdlcd_pixel_format eFormat );
void vHDLCDHandler( void );
void vEnableHdlcdIrq( void );
void vDisableHdlcdIrq( void );

extern int32_t lIspInit();
extern void arm_2d_init( void );

void vStartISPDemo()
{
    enum hdlcd_error_t eHdlcdErr;

    NVIC_SetPriority( ISP_IRQn, 7 );
    NVIC_SetPriority( HDLCD_IRQn, 7 );
    NVIC_SetPriority( NPU0_IRQn, 5 );
    NVIC_SetPriority( UARTTX0_IRQn, 4 );

    LogInfo( ( "ISP init\r\n" ) );

    arm_2d_init();

    LogInfo( ( "Starting HDLCD config!\r\n" ) );

    eHdlcdErr = hdlcd_init( &HDLCD_DEV );

    if( eHdlcdErr != HDLCD_ERR_NONE )
    {
        LogInfo( ( "HDLCD init error! \r\n" ) );

        while( 1 )
        {
        }
    }

    eHdlcdErr = hdlcd_static_config( &HDLCD_DEV );

    if( eHdlcdErr != HDLCD_ERR_NONE )
    {
        LogInfo( ( "HDLCD static config error! \r\n" ) );

        while( 1 )
        {
        }
    }

    LogInfo( ( "Starting ISP init!\r\n" ) );

    lIspInit();

    pvDownscaledFrameReady = prvHandleDownscaledFrame;
    pvFullResolutionFrameReady = prvHandleFullResolutionFrame;
}

static uint32_t ulHdlcdToArm2dColorScheme( enum hdlcd_pixel_format eFormat )
{
    switch( eFormat )
    {
        case HDLCD_PIXEL_FORMAT_RGB565:
            return ARM_2D_COLOUR_RGB565;

        case HDLCD_PIXEL_FORMAT_RGB32:
            return ARM_2D_COLOUR_RGB32;

        case HDLCD_PIXEL_FORMAT_A2R10G10B10:
            return ARM_2D_COLOUR_32BIT;

        default:
            return 0;
    }
}

void vRgbToGrayscale( const uint8_t * pucSrcImage,
                      uint8_t * pucDstImage,
                      const size_t xDstImageSize,
                      enum hdlcd_pixel_format eFormat )
{
    const struct hdlcd_pixel_cfg_t * const pxPixelConfig = HDLCD_MODES[ eFormat ].pixel_cfg;
    const uint32_t ulRedOffset = pxPixelConfig->red.offset;
    const uint32_t ulGreenOffset = pxPixelConfig->green.offset;
    const uint32_t ulBlueOffset = pxPixelConfig->blue.offset;
    const uint32_t ulRedMask = ( 0x1UL << pxPixelConfig->red.bit_size ) - 1;
    const uint32_t ulGreenMask = ( 0x1UL << pxPixelConfig->green.bit_size ) - 1;
    const uint32_t ulBlueMask = ( 0x1UL << pxPixelConfig->blue.bit_size ) - 1;
    const float xRed = 0.299 * ( 0x1UL << ( 8 - pxPixelConfig->red.bit_size ) );
    const float xGreen = 0.587 * ( 0x1UL << ( 8 - pxPixelConfig->green.bit_size ) );
    const float xBlue = 0.114 * ( 0x1UL << ( 8 - pxPixelConfig->blue.bit_size ) );

    for( size_t i = 0; i < xDstImageSize; ++i, pucSrcImage += HDLCD_MODES[ eFormat ].bytes_per_pixel )
    {
        uint32_t ulGrayIntensity = xRed * ( ( ( *( uint32_t * ) pucSrcImage ) >> ulRedOffset ) & ulRedMask ) + xGreen * ( ( ( *( uint32_t * ) pucSrcImage ) >> ulGreenOffset ) & ulGreenMask )
                                   + xBlue * ( ( ( *( uint32_t * ) pucSrcImage ) >> ulBlueOffset ) & ulBlueMask );
        *pucDstImage++ = ulGrayIntensity <= 0xff ? ulGrayIntensity : 0xff;
    }
}

void vGrayscaleToRgb( const uint8_t * pucSrcImage,
                      uint8_t * pucDstImage,
                      const size_t xSrcImageSize,
                      enum hdlcd_pixel_format eFormat )
{
    const struct hdlcd_pixel_cfg_t * const pxPixelConfig = HDLCD_MODES[ eFormat ].pixel_cfg;
    const uint32_t ulRedOffset = pxPixelConfig->red.offset;
    const uint32_t ulGreenOffset = pxPixelConfig->green.offset;
    const uint32_t ulBlueOffset = pxPixelConfig->blue.offset;
    const uint32_t ulRedMask = ( 0x1UL << pxPixelConfig->red.bit_size ) - 1;
    const uint32_t ulGreenMask = ( 0x1UL << pxPixelConfig->green.bit_size ) - 1;
    const uint32_t ulBlueMask = ( 0x1UL << pxPixelConfig->blue.bit_size ) - 1;
    uint32_t ulColor;
    uint32_t ulGrayMinimum = 0xff;
    uint32_t ulGrayMaximum = 0x00;

    for( size_t i = 0; i < xSrcImageSize; ++i, pucDstImage += HDLCD_MODES[ eFormat ].bytes_per_pixel )
    {
        ulColor = ( ( ( ( ( uint32_t ) *pucSrcImage ) >> ( 8 - pxPixelConfig->red.bit_size ) ) & ulRedMask ) << ulRedOffset )
                  | ( ( ( ( ( uint32_t ) *pucSrcImage ) >> ( 8 - pxPixelConfig->green.bit_size ) ) & ulGreenMask ) << ulGreenOffset )
                  | ( ( ( ( ( uint32_t ) *pucSrcImage ) >> ( 8 - pxPixelConfig->blue.bit_size ) ) & ulBlueMask ) << ulBlueOffset );

        if( ( uint32_t ) *pucSrcImage > ulGrayMaximum )
        {
            ulGrayMaximum = ( uint32_t ) *pucSrcImage;
        }

        if( ( uint32_t ) *pucSrcImage < ulGrayMinimum )
        {
            ulGrayMinimum = ( uint32_t ) *pucSrcImage;
        }

        for( size_t j = 0; j < HDLCD_MODES[ eFormat ].bytes_per_pixel; j++ )
        {
            pucDstImage[ j ] = ( ( uint8_t * ) &ulColor )[ j ];
        }

        pucSrcImage++;
    }

    LogInfo( ( "srcGray: %02u-%02u\r\n", ulGrayMinimum, ulGrayMaximum ) );
}

static void prvHandleFullResolutionFrame( uint32_t ulAddress,
                                          uint32_t ulWidth,
                                          uint32_t ulHeight,
                                          uint32_t ulMode,
                                          uint32_t ulFrameId )
{
    /* If the previous full frame wasn't inferred yet, skip this frame */
    if( xLastFrame.ulDisplayed == xLastFrame.ulInferred )
    {
        xLastFrame.ulDisplayed = ulFrameId;
        xLastFrame.xFullResolution.ulMode = ulMode;
        xLastFrame.xFullResolution.ulWidth = ulWidth;
        xLastFrame.xFullResolution.ulHeight = ulHeight;
        prvHdlcdShow( ulAddress, ulWidth, ulHeight, ulMode );
    }
}

static void prvHandleDownscaledFrame( uint32_t ulAddress,
                                      uint32_t ulWidth,
                                      uint32_t ulHeight,
                                      uint32_t ulMode,
                                      uint32_t ulFrameId )
{
    xLastFrame.ulInferred = ulFrameId;
    uint32_t i = 0UL;
    uint32_t ulResultsCount = 0UL;
    struct DetectRegion_t xResults[ isp_configMAX_DETECT_RESULTS ];
    float xUpscaleWidth = xLastFrame.xFullResolution.ulWidth / ulWidth;
    float xUpscaleHeight = xLastFrame.xFullResolution.ulHeight / ulHeight;

    static struct arm_2d_region_t xFrameRegion = { 0 };
    enum hdlcd_pixel_format ePixelFormat = HDLCD_PIXEL_FORMAT_RGB565;

    if( ePixelFormat == HDLCD_PIXEL_FORMAT_NOT_SUPPORTED )
    {
        LogError( ( "Unsupported pixel format: 0x%x\r\n", xLastFrame.xFullResolution.ulMode ) );
        return;
    }

    if( ulWidth * ulHeight > isp_configMAX_INFER_FRAME_SIZE )
    {
        LogError( ( "Input frame too big for inference!\r\n" ) );
        return;
    }

    /* Note: mode Vs xLastFrame.xFullResolution.ulMode */
    LogInfo( ( "Converting to Gray: 0x%x -> 0x%x\r\n", ulAddress, ( uint32_t ) ucGrayBuffer ) );
    vRgbToGrayscale( ( uint8_t * ) ulAddress, ucGrayBuffer, ulWidth * ulHeight, HDLCD_PIXEL_FORMAT_RGB565 );

    /*taskENTER_CRITICAL(); */
    ulResultsCount = isp_configMAX_DETECT_RESULTS;
    lMLRunInference( ucGrayBuffer, xResults, &ulResultsCount );
    /*taskEXIT_CRITICAL(); */

    hdlcd_disable( &HDLCD_DEV );

    /* Draw frame according to selected region in the secondary buffer */
    for( i = 0; i < ulResultsCount; i++ )
    {
        xFrameRegion.tSize.iWidth = ( int ) xResults[ i ].ulW * xUpscaleWidth;
        xFrameRegion.tSize.iHeight = ( int ) xResults[ i ].ulH * xUpscaleHeight;
        xFrameRegion.tLocation.iX = ( int ) xResults[ i ].ulX * xUpscaleWidth;
        xFrameRegion.tLocation.iY = ( int ) xResults[ i ].ulY * xUpscaleHeight;

        if( xFrameRegion.tSize.iWidth < isp_configHIGHLIGHTED_FRAME_WIDTH * 3 )
        {
            xFrameRegion.tSize.iWidth = isp_configHIGHLIGHTED_FRAME_WIDTH * 3;
        }

        if( xFrameRegion.tSize.iHeight < isp_configHIGHLIGHTED_FRAME_WIDTH * 3 )
        {
            xFrameRegion.tSize.iHeight = isp_configHIGHLIGHTED_FRAME_WIDTH * 3;
        }

        prvDrawFrameOnTile( &xRootTile, &xFrameRegion, ePixelFormat );
    }

    hdlcd_enable( &HDLCD_DEV );
    /* Wait until HDLCD is displayed at least once */
    ucFlag = 0;
    vEnableHdlcdIrq();

    while( ucFlag < 5 )
    {
    }

    vDisableHdlcdIrq();
}

static void prvHdlcdShow( uint32_t ulAddress,
                          uint32_t ulWidth,
                          uint32_t ulHeight,
                          uint32_t ulMode )
{
    struct hdlcd_resolution_cfg_t xCustomResolutionConfig = { 0 };

    enum hdlcd_error_t eHdlcdErr = 0;
    enum hdlcd_pixel_format ePixelFormat = HDLCD_PIXEL_FORMAT_RGB565;

    LogInfo( ( "Displaying image...\r\n" ) );

    if( ePixelFormat == HDLCD_PIXEL_FORMAT_NOT_SUPPORTED )
    {
        LogError( ( "Unsupported pixel format: 0x%x\r\n", ulMode ) );
        return;
    }

    hdlcd_disable( &HDLCD_DEV );

    /* Note: This only works, because FVP ignores all timing values */
    xCustomResolutionConfig.v_data = ulHeight;
    xCustomResolutionConfig.h_data = ulWidth;
    hdlcd_set_custom_resolution( &HDLCD_DEV, &xCustomResolutionConfig );

    struct hdlcd_buffer_cfg_t xHdlcdBuffer =
    {
        .base_address = ulAddress,
        .line_length  = ulWidth * HDLCD_MODES[ ePixelFormat ].bytes_per_pixel,
        .line_count   = ulHeight - 1,
        .line_pitch   = ulWidth * HDLCD_MODES[ ePixelFormat ].bytes_per_pixel,
        .pixel_format = HDLCD_MODES[ ePixelFormat ].pixel_format
    };

    eHdlcdErr = hdlcd_buffer_config( &HDLCD_DEV, &xHdlcdBuffer );

    if( eHdlcdErr != HDLCD_ERR_NONE )
    {
        LogError( ( "HDLCD buffer config error! \r\n" ) );

        while( 1 )
        {
        }
    }

    eHdlcdErr = hdlcd_pixel_config( &HDLCD_DEV, HDLCD_MODES[ ePixelFormat ].pixel_cfg );

    if( eHdlcdErr != HDLCD_ERR_NONE )
    {
        LogError( ( "HDLCD pixel config error! \r\n" ) );

        while( 1 )
        {
        }
    }

    xRootTile.pwBuffer = ( uint32_t * ) ulAddress;
    xRootTile.tRegion.tSize.iWidth = ulWidth;
    xRootTile.tRegion.tSize.iHeight = ulHeight;
    xRootTile.tInfo.tColourInfo.chScheme = ulHdlcdToArm2dColorScheme( ePixelFormat );

    hdlcd_enable( &HDLCD_DEV );

    LogInfo( ( "Image displayed\r\n" ) );
}

static void prvDrawFrameOnTile( struct arm_2d_tile_t * pxDestination,
                                struct arm_2d_region_t * pxFrame,
                                enum hdlcd_pixel_format eFormat )
{
    static struct arm_2d_region_t xBlendRegion = { 0 };
    static struct arm_2d_tile_t xBorderTile = { 0 };
    uint_fast8_t xOpacity = 0xFF;

    xBlendRegion.tSize.iWidth = ( pxFrame->tSize.iWidth - 2 * isp_configHIGHLIGHTED_FRAME_WIDTH );
    xBlendRegion.tSize.iHeight = ( pxFrame->tSize.iHeight - 2 * isp_configHIGHLIGHTED_FRAME_WIDTH );
    xBlendRegion.tLocation.iX = isp_configHIGHLIGHTED_FRAME_WIDTH;
    xBlendRegion.tLocation.iY = isp_configHIGHLIGHTED_FRAME_WIDTH;

    xBorderTile.tRegion.tSize.iWidth = pxFrame->tSize.iWidth;
    xBorderTile.tRegion.tSize.iHeight = pxFrame->tSize.iHeight;
    xBorderTile.tInfo.bIsRoot = true;
    xBorderTile.tInfo.bHasEnforcedColour = true;
    xBorderTile.tInfo.tColourInfo.chScheme = ulHdlcdToArm2dColorScheme( eFormat );
    xBorderTile.pwBuffer = ( uint32_t * ) ucBorderTileBuffer;

    if( eFormat == HDLCD_PIXEL_FORMAT_RGB565 )
    {
        arm_2dp_rgb16_fill_colour(
            NULL, &xBorderTile, &xBorderTile.tRegion, ( uint16_t ) HDLCD_MODES[ eFormat ].default_highlight_color );
        arm_2dp_rgb16_fill_colour( NULL, &xBorderTile, &xBlendRegion, ( uint16_t ) HDLCD_MODES[ eFormat ].default_mask_color );
        arm_2dp_rgb565_tile_copy_with_colour_keying_and_opacity(
            NULL,
            &xBorderTile,
            pxDestination,
            pxFrame,
            xOpacity,
            ( arm_2d_color_rgb565_t ) { .tValue = ( uint16_t ) HDLCD_MODES[ eFormat ].default_mask_color } );
    }
    else
    {
        arm_2dp_rgb32_fill_colour(
            NULL, &xBorderTile, &xBorderTile.tRegion, HDLCD_MODES[ eFormat ].default_highlight_color );
        arm_2dp_rgb32_fill_colour( NULL, &xBorderTile, &xBlendRegion, HDLCD_MODES[ eFormat ].default_mask_color );
        arm_2dp_cccn888_tile_copy_with_colour_keying_and_opacity(
            NULL,
            &xBorderTile,
            pxDestination,
            pxFrame,
            xOpacity,
            ( arm_2d_color_cccn888_t ) { .tValue = HDLCD_MODES[ eFormat ].default_mask_color } );
    }
}

void vHDLCDHandler( void )
{
    /* Clear IRQ */
    uint32_t ulIrqState = hdlcd_get_irq_state( &HDLCD_DEV );

    hdlcd_clear_irq( &HDLCD_DEV, ulIrqState );

    if( ulIrqState & INT_DMA_END_Msk )
    {
        ucFlag++;
    }

    __NVIC_ClearPendingIRQ( HDLCD_IRQn );
}

void vEnableHdlcdIrq( void )
{
    NVIC_ClearPendingIRQ( HDLCD_IRQn );
    NVIC_SetVector( HDLCD_IRQn, ( uint32_t ) vHDLCDHandler );
    NVIC_EnableIRQ( HDLCD_IRQn );
    hdlcd_enable_irq( &HDLCD_DEV, INT_DMA_END_Msk );
}

void vDisableHdlcdIrq( void )
{
    NVIC_DisableIRQ( HDLCD_IRQn );
    hdlcd_disable_irq( &HDLCD_DEV, INT_DMA_END_Msk );
}
