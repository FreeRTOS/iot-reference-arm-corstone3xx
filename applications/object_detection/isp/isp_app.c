/* Copyright 2024, Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "acamera_command_api.h"
#include "acamera_control_config.h"
#include "acamera_firmware_api.h"
#include "acamera_firmware_config.h"
#include "acamera_interface_config.h"
#include "acamera_logger.h"

#include "application_command_api.h"

#include "system_cdma_platform.h"
#include "system_interrupts.h"

#if ISP_HAS_STREAM_CONNECTION
    #include "acamera_connection.h"
#endif

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "events.h"

#include "isp_config.h"

#define ENABLE_TPG_AT_START    0

static acamera_settings xACameraSettings[ FIRMWARE_CONTEXT_NUMBER ] =
{
    {
        .sensor_init = fvp_sensor_init,
        .sensor_deinit = fvp_sensor_deinit,
        .get_calibrations = get_calibrations_dummy,
        .lens_init = NULL,
        .lens_deinit = NULL,
        .isp_base = 0x0,
        .hw_isp_addr = ISP_SOC_START_ADDR,
        .callback_dma_alloc_coherent = pvCallbackDmaAllocCoherent,
        .callback_dma_free_coherent = vCallbackDmaFreeCoherent,
        .callback_stream_get_frame = lCallbackStreamGetFrame,
        .callback_stream_put_frame = lCallbackStreamPutFrame,
    },
};

/* This example will run the infinite loop to process the firmware events. */
/* This variable also can be changed outside to stop the processing. */
volatile int32_t lACameraMainLoopActive = 1;

/* Stream semaphore is used to block stream thread until previous frame is displayed */
SemaphoreHandle_t xStreamSemaphore;
StaticSemaphore_t xStreamSemaphoreBuffer;

#if ISP_HAS_STREAM_CONNECTION && CONNECTION_IN_THREAD
    static void connection_thread( void * pvParameters );
#endif
static void prvStreamControlThread( void * pvParameters );
static void prvACameraThread( void * pvParameters );

/* this is a main application IRQ handler to drive the firmware */
/* The main purpose is to redirect ISP irq event to the firmware core */
/* Please see the ACamera Porting Guide for details. */
static void prvInterruptHandler( void * pvPtr,
                                 uint32_t ulMask )
{
    /* the lower bits are for ISP interrupts on ACamera FPGA reference platform */
    uint32_t ulIspMask = ulMask & 0x0000FFFF;

    /* tell the firmware that isp interrupts happened */
    if( ulIspMask )
    {
        /* the first irq pins are connected to ISP */
        acamera_interrupt_handler();
    }
}

static void prvCreateTasks()
{
    /* ACamera provides a simple protocol to communicate with firmware */
    /* outside application. Several channels are supported depend on the */
    /* system requirements. */
    /* To start using ACamera Control Tool the connection must be initialised */
    /* before by calling acamera_connection_init */
    /* The connection module parses input commands from ACT and call the required */
    /* api command like acamera_command or acamera_calibrations. */
    /* Please see acamera_command_api.h for details. */
    #if ISP_HAS_STREAM_CONNECTION
        #if !CONNECTION_IN_THREAD
            acamera_connection_init();
        #else

            /*xTaskCreate(connection_thread,
             *          "connection",
             *          configMINIMAL_STACK_SIZE * 2,
             *          NULL,
             *          (configMAX_PRIORITIES - 1) | portPRIVILEGE_BIT,
             *          NULL);*/
        #endif
    #endif

    xTaskCreate( prvACameraThread,
                 "acamera",
                 configMINIMAL_STACK_SIZE * 4,
                 NULL,
                 ( configMAX_PRIORITIES - 2 ) | portPRIVILEGE_BIT,
                 NULL );

    xStreamSemaphore = xSemaphoreCreateBinaryStatic( &xStreamSemaphoreBuffer );
    xTaskCreate( prvStreamControlThread,
                 "stream",
                 configMINIMAL_STACK_SIZE,
                 NULL,
                 ( configMAX_PRIORITIES - 3 ) | portPRIVILEGE_BIT,
                 NULL );
}

static int prvIspInitialConfig()
{
    int32_t lResult;
    uint32_t ulReturnCode;
    uint32_t ulContextNumber;
    uint32_t ulPreviousContextNumber = 0;

    /* The firmware supports multicontext. */
    /* It means that the customer can use the same firmware for controlling */
    /* several instances of different sensors/isp. To initialise a context */
    /* the structure acamera_settings must be filled properly. */
    /* the total number of initialized context must not exceed FIRMWARE_CONTEXT_NUMBER */
    /* all contexts are numerated from 0 till ctx_number - 1 */
    lResult = acamera_init( xACameraSettings, FIRMWARE_CONTEXT_NUMBER );

    if( lResult != 0 )
    {
        LOG( LOG_ERR, "Failed to start firmware processing thread. (0x%x)", lResult );
        return lResult;
    }

    application_command( TGENERAL, ACTIVE_CONTEXT, 0, COMMAND_GET, &ulPreviousContextNumber );

    system_interrupt_set_handler( prvInterruptHandler, NULL );

    for( ulContextNumber = 0; ulContextNumber < FIRMWARE_CONTEXT_NUMBER; ulContextNumber++ )
    {
        application_command( TGENERAL, ACTIVE_CONTEXT, ulContextNumber, COMMAND_SET, &ulReturnCode );

        /* Disable most ISP calibrations. FVP sensor streams processed RGB images which does not need calibration  */
        application_command( TALGORITHMS, AE_MODE_ID, AE_FULL_MANUAL, COMMAND_SET, &ulReturnCode );
        application_command( TALGORITHMS, AWB_MODE_ID, AWB_MANUAL, COMMAND_SET, &ulReturnCode );

        application_command( TISP_MODULES, ISP_MODULES_MANUAL_TEMPER, ON, COMMAND_SET, &ulReturnCode );
        application_command( TISP_MODULES, ISP_MODULES_MANUAL_IRIDIX, ON, COMMAND_SET, &ulReturnCode );

        application_command( TSYSTEM, SYSTEM_MANUAL_EXPOSURE, ON, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_MANUAL_EXPOSURE_RATIO, ON, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_MANUAL_ISP_DIGITAL_GAIN, ON, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_MANUAL_AWB, ON, COMMAND_SET, &ulReturnCode );

#define AWB_GAIN    9
        application_command( TSYSTEM, SYSTEM_AWB_BLUE_GAIN, AWB_GAIN, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_AWB_GREEN_EVEN_GAIN, AWB_GAIN, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_AWB_GREEN_ODD_GAIN, AWB_GAIN, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_AWB_RED_GAIN, AWB_GAIN, COMMAND_SET, &ulReturnCode );
#define CCM_GAIN    4095
        application_command( TSYSTEM, SYSTEM_MANUAL_CCM, ON, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_RR, CCM_GAIN, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_GG, CCM_GAIN, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_BB, CCM_GAIN, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_RG, 0, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_RB, 0, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_GR, 0, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_GB, 0, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_BR, 0, COMMAND_SET, &ulReturnCode );
        application_command( TSYSTEM, SYSTEM_CCM_MATRIX_BG, 0, COMMAND_SET, &ulReturnCode );

        application_command( TSYSTEM, TEST_PATTERN_MODE_ID, 5, COMMAND_SET, &ulReturnCode );

        #if ENABLE_TPG_AT_START
            application_command( TSYSTEM, TEST_PATTERN_ENABLE_ID, ON, COMMAND_SET, &ulReturnCode );
        #endif

        application_command( TSCENE_MODES, SHARPENING_STRENGTH_ID, 0, COMMAND_SET, &ulReturnCode );

        application_command( TIMAGE, FR_FORMAT_BASE_PLANE_ID, DMA_FORMAT_RGB565, COMMAND_SET, &ulReturnCode );
        application_command( TIMAGE, DS1_FORMAT_BASE_PLANE_ID, DMA_FORMAT_RGB565, COMMAND_SET, &ulReturnCode );
    }

    application_command( TGENERAL, ACTIVE_CONTEXT, ulPreviousContextNumber, COMMAND_SET, &ulReturnCode );

    return 0;
}

int32_t lIspInit()
{
    system_cdma_setup();
    prvCreateTasks();
    return 0;
}

/* On systems which support pthreads it is more efficient to run */
/* control channel in a separate thread to let the firmware to communicate */
/* with ACamera Control Tool (ACT) */
/* ACT allows to change the firmware behaviour by calling API functions, change ISP registers and */
/* update calibration LUTs. */
/* Please read the ACamera Control Tool User Guide for details */
#if ISP_HAS_STREAM_CONNECTION && CONNECTION_IN_THREAD
    static void connection_thread( void * pvParameters )
    {
        /* acamera_connection_init is used to initialize the */
        /* communication channel between the firmware application */
        /* and the firmware. It is used only together with ACT tool */
        /* and may be omitted on the customer discretion */
        /* if ACT is not required */
        acamera_connection_init();

        for( ; ; )
        {
            /* the function checks the incoming requests from */
            /* ACT tool and call the corresponding API command. */
            /* Please note that acamera_connection_process may be omitted */
            /* if ACT tool is not used for the firmware API. */
            acamera_connection_process();
        }
    }
#endif /* if ISP_HAS_STREAM_CONNECTION && CONNECTION_IN_THREAD */

extern volatile uint32_t UART_PLEASE_HOLD_STREAM;
static void prvStreamControlThread( void * pvParameters )
{
    uint32_t ulFrameCount = 0;

    while( 1 )
    {
        EventBits_t xFlags = xEventGroupWaitBits( xSystemEvents, ( EventBits_t ) EVENT_MASK_ML_STOP, pdTRUE, pdFAIL, 10 );

        if( xFlags & EVENT_MASK_ML_STOP )
        {
            /* jump out to outer loop */
            LogInfo( ( "Stopping processing\r\n" ) );
            break;
        }

        if( STREAM_ENABLED )
        {
            if( !ulFrameCount || !UART_PLEASE_HOLD_STREAM )
            {
                LOG( LOG_CRIT, "\033[1;34m-- CAMERA STREAM TRIGGER #%d --\033[1;0m", ulFrameCount );
                ulFrameCount++;
                *( ( uint8_t * ) ( ISP_VIRTUAL_CAMERA_BASE_NS ) ) = 0x1; /* camera enable */
            }

            /* Displaying a frame gives the semaphore. If previous frame is not yet
            * displayed, or no frames have been displayed, task waits 100 ticks. */
            xSemaphoreTake( xStreamSemaphore, 100 );
        }

        vTaskDelay( 5 );
    }
}

static void prvACameraThread( void * pvParameters )
{
    static uint32_t ulDoInitialSetup = 1;
    uint32_t ulReturnCode = 0;

    if( prvIspInitialConfig() )
    {
        lACameraMainLoopActive = 0;
    }

    /* acamera_process function must be called on every incoming interrupt */
    /* to give the firmware the possibility to apply */
    /* all internal algorithms and change the ISP state. */
    /* The external application can be run in the same loop on bare metal systems. */
    while( lACameraMainLoopActive )
    {
        /* acamera_process must be called for each initialised context */
        acamera_process();
        #if ISP_HAS_STREAM_CONNECTION && !CONNECTION_IN_THREAD
            /* acamera_connection_process is used for communication between */
            /* firmware and ACT through different possible channels like */
            /* cmd_queue memory in ISP, socket, UART, chardev etc. */
            /* Different channels can be supported depending on the target */
            /* platform. The common case when cmd_queue buffer is used */
            /* ( see acamera_isp_config.h ) */
            /* The channels supported by this routine can be used not only on */
            /* NIOS2 platform but on the customer system as well. */
            acamera_connection_process();
        #endif

        /* This needs to be after first acamera_process(); because for the first time, it resets the FSMs and */
        /* Crop FSM resets the scaler width/height values */
        if( ulDoInitialSetup )
        {
            ulDoInitialSetup = 0;
            application_command( TIMAGE, IMAGE_RESIZE_TYPE_ID, SCALER_DS, COMMAND_SET, &ulReturnCode );
            application_command( TIMAGE, IMAGE_RESIZE_WIDTH_ID, 192, COMMAND_SET, &ulReturnCode );
            application_command( TIMAGE, IMAGE_RESIZE_HEIGHT_ID, 192, COMMAND_SET, &ulReturnCode );
            application_command( TIMAGE, IMAGE_RESIZE_ENABLE_ID, RUN, COMMAND_SET, &ulReturnCode );

            application_command( TSENSOR, SENSOR_STREAMING, ON, COMMAND_SET, &ulReturnCode );
        }

        taskYIELD();
    }

    /* this api function will free */
    /* all resources allocated by the firmware */
    acamera_terminate();

    LOG( LOG_CRIT, "Acamera terminated" );

    for( ; ; )
    {
        /* Task shouldn't exit */
    }
}
