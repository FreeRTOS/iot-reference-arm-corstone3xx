/*
 * FreeRTOS Kernel V10.4.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
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
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Here is a good place to include header files that are required across
 * your application. */
/* #include <stdint.h> */
/* #include "aws_secure_sockets_config.h" */
/* #include "RTOS_config.h" */

#include "app_config.h"

#ifndef   __USED
    #define __USED    __attribute__( ( used ) )
#endif
#ifndef   __WEAK
    #define __WEAK    __attribute__( ( weak ) )
#endif


extern uint32_t SystemCoreClock;

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION     1
#define configSUPPORT_DYNAMIC_ALLOCATION    1
#define configTOTAL_HEAP_SIZE               720896
#define configAPPLICATION_ALLOCATED_HEAP    0

#define configENABLE_MVE                    0
#define configENABLE_FPU                    1
#define configENABLE_MPU                    0
#define configENABLE_TRUSTZONE              0
#define configRUN_FREERTOS_SECURE_ONLY      0

/* The target specific macros `configTICK_RATE_HZ`, `pdMS_TO_TICKS` and
 * `TICKS_TO_pdMS` are defined in `FreeRTOSConfig_target.h`. */
#include "FreeRTOSConfig_target.h"

#define configMINIMAL_STACK_SIZE                   4096
#define configUSE_16_BIT_TICKS                     0
#define portTICK_TYPE_IS_ATOMIC                    1

#define configUSE_PREEMPTION                       1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    0
#define configUSE_TICKLESS_IDLE                    0
#define configCPU_CLOCK_HZ                         ( ( unsigned long ) SystemCoreClock )
#define configMAX_PRIORITIES                       56
#define configMAX_TASK_NAME_LEN                    16
#define configIDLE_SHOULD_YIELD                    1
#define configUSE_TASK_NOTIFICATIONS               1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES      3
#define configUSE_MUTEXES                          1
#define configUSE_RECURSIVE_MUTEXES                1
#define configUSE_COUNTING_SEMAPHORES              1
#define configUSE_ALTERNATIVE_API                  0 /* Deprecated! */
#define configQUEUE_REGISTRY_SIZE                  10
#define configUSE_QUEUE_SETS                       0
#define configUSE_TIME_SLICING                     1
#define configUSE_NEWLIB_REENTRANT                 0
#define configENABLE_BACKWARD_COMPATIBILITY        0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS    5

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                        0
#define configUSE_TICK_HOOK                        0
#define configCHECK_FOR_STACK_OVERFLOW             1 /* This should only be set for development */
#define configUSE_MALLOC_FAILED_HOOK               0
#define configUSE_DAEMON_TASK_STARTUP_HOOK         0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS              0
#define configUSE_TRACE_FACILITY                   1
#define configUSE_STATS_FORMATTING_FUNCTIONS       0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                      0
#define configMAX_CO_ROUTINE_PRIORITIES            1

/* Software timer related definitions. */
#define configUSE_TIMERS                           1
#define configTIMER_TASK_PRIORITY                  3
#define configTIMER_QUEUE_LENGTH                   10
#define configTIMER_TASK_STACK_DEPTH               configMINIMAL_STACK_SIZE

/* Interrupt nesting behaviour configuration. */
/* FIXME: these were taken from a cortex M4 example project */
#define configKERNEL_INTERRUPT_PRIORITY         255               /* Should be set to the lowest interrupt priority */

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 5 << ( 8U - 3U ) ) /* Platform implements 3 priority bits */
#define configMAC_INTERRUPT_PRIORITY            7                    /* Used at ethernet init and shifted by CMSIS call */

/* Define to trap errors during development. */
void vAssertCalled( const char * pcFile,
                    unsigned long ulLine );
#define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ );

/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS    0
#define configTOTAL_MPU_REGIONS                                   8      /* Default value. */
#define configTEX_S_C_B_FLASH                                     0x07UL /* Default value. */
#define configTEX_S_C_B_SRAM                                      0x07UL /* Default value. */
#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY               1

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                                  1
#define INCLUDE_uxTaskPriorityGet                                 1
#define INCLUDE_vTaskDelete                                       1
#define INCLUDE_vTaskSuspend                                      1
#define INCLUDE_xResumeFromISR                                    1
#define INCLUDE_vTaskDelayUntil                                   1
#define INCLUDE_vTaskDelay                                        1
#define INCLUDE_xTaskGetSchedulerState                            1
#define INCLUDE_xTaskGetCurrentTaskHandle                         1
#define INCLUDE_uxTaskGetStackHighWaterMark                       1
#define INCLUDE_xTaskGetIdleTaskHandle                            0
#define INCLUDE_eTaskGetState                                     1
#define INCLUDE_xEventGroupSetBitFromISR                          1
#define INCLUDE_xTimerPendFunctionCall                            1
#define INCLUDE_xTaskAbortDelay                                   1
#define INCLUDE_xTaskGetHandle                                    0
#define INCLUDE_xTaskResumeFromISR                                1
#define INCLUDE_xEventGroupSetBitsFromISR                         1
#define INCLUDE_xSemaphoreGetMutexHolder                          1

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
    /* __NVIC_PRIO_BITS will be specified when CMSIS is being used. */
    #define configPRIO_BITS    __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS    3                                 /* 8 priority levels. */
#endif

/* Logging task definitions. */
void vLoggingPrintf( const char * pcFormat,
                     ... );

/* Map the FreeRTOS printf() to the logging task printf. */
#define configPRINTF( x )          vLoggingPrintf x

/* Map the logging task's printf to the board specific output function. */
#define configPRINT_STRING( x )    printf( "%s", ( x ) )

/* Sets the length of the buffers into which logging messages are written - so
 * also defines the maximum length of each log message. */
#if ( appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE == 1 )
    #define configLOGGING_MAX_MESSAGE_LENGTH        20480
#else
    #define configLOGGING_MAX_MESSAGE_LENGTH        1024
#endif
/* Prepend each log message with a message number, the task name and a time stamp. */
#define configLOGGING_INCLUDE_TIME_AND_TASK_NAME    1

/* A header file that defines trace macro can be included here. */

/* FIXME: this is just to build freeRTOS demo */

/* The address of an echo server that will be used by the two demo echo client
 * tasks:
 * http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_Echo_Clients.html
 * http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/UDP_Echo_Clients.html */
#define configECHO_SERVER_ADDR0       192
#define configECHO_SERVER_ADDR1       168
#define configECHO_SERVER_ADDR2       0
#define configECHO_SERVER_ADDR3       105
#define configTCP_ECHO_CLIENT_PORT    45000

/* Default MAC address configuration.  The demo creates a virtual network
 * connection that uses this MAC address by accessing the raw Ethernet/WiFi data
 * to and from a real network connection on the host PC.  See the
 * configNETWORK_INTERFACE_TO_USE definition above for information on how to
 * configure the real network connection to use. */
#define configMAC_ADDR0               0x00
#define configMAC_ADDR1               0x11
#define configMAC_ADDR2               0x22
#define configMAC_ADDR3               0x33
#define configMAC_ADDR4               0x44
#define configMAC_ADDR5               0x21

/* Default IP address configuration.  Used in ipconfigUSE_DHCP is set to 0, or
 * ipconfigUSE_DHCP is set to 1 but a DNS server cannot be contacted. */
#define configIP_ADDR0                192
#define configIP_ADDR1                168
#define configIP_ADDR2                0
#define configIP_ADDR3                105

/* Default gateway IP address configuration.  Used in ipconfigUSE_DHCP is set to
 * 0, or ipconfigUSE_DHCP is set to 1 but a DNS server cannot be contacted. */
#define configGATEWAY_ADDR0           192
#define configGATEWAY_ADDR1           168
#define configGATEWAY_ADDR2           0
#define configGATEWAY_ADDR3           1

/* Default DNS server configuration.  OpenDNS addresses are 208.67.222.222 and
 * 208.67.220.220.  Used in ipconfigUSE_DHCP is set to 0, or ipconfigUSE_DHCP is
 * set to 1 but a DNS server cannot be contacted.*/
#define configDNS_SERVER_ADDR0        208
#define configDNS_SERVER_ADDR1        67
#define configDNS_SERVER_ADDR2        222
#define configDNS_SERVER_ADDR3        222

/* Default netmask configuration.  Used in ipconfigUSE_DHCP is set to 0, or
 * ipconfigUSE_DHCP is set to 1 but a DNS server cannot be contacted. */
#define configNET_MASK0               255
#define configNET_MASK1               255
#define configNET_MASK2               255
#define configNET_MASK3               0

#define democonfigNETWORK_TYPES       ( AWSIOT_NETWORK_TYPE_ETH )

#endif /* FREERTOS_CONFIG_H */
