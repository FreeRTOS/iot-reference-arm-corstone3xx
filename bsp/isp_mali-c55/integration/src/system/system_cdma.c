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

#include "system_cdma.h"
#include "acamera_logger.h"
#include "acamera_types.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define ioremap( addr, size )           ( ( void * ) addr )
#define iounmap( addr )                 ( void ) addr

#define atomic_inc_return( num_ptr )    ( ++( *num_ptr ) )
#define atomic_set( var, val ) \
    do {                       \
        *var = val;            \
    } while( 0 )

typedef struct
{
    void * dev_addr;
    void * fw_addr;
    size_t size;
    void * sys_back_ptr;
} mem_addr_pair_t;

struct tasklet_struct
{
    struct tasklet_struct * next;
    void (* func)( unsigned long );
    unsigned long data;
};
typedef struct
{
    struct tasklet_struct m_task;
    mem_addr_pair_t * mem_data;
} mem_tasklet_t;

struct completion
{
    SemaphoreHandle_t xSemaphore;
    StaticSemaphore_t xSemaphoreBuffer;
};
typedef struct
{
    char * name;
    unsigned int sg_device_nents[ FIRMWARE_CONTEXT_NUMBER ][ SYSTEM_DMA_TOGGLE_COUNT ];

    unsigned int sg_fwmem_nents[ FIRMWARE_CONTEXT_NUMBER ][ SYSTEM_DMA_TOGGLE_COUNT ];

    mem_addr_pair_t mem_addrs[ FIRMWARE_CONTEXT_NUMBER ][ SYSTEM_DMA_TOGGLE_COUNT ][ SYSTEM_DMA_MAX_CHANNEL ];

    mem_tasklet_t task_list[ FIRMWARE_CONTEXT_NUMBER ][ SYSTEM_DMA_TOGGLE_COUNT ][ SYSTEM_DMA_MAX_CHANNEL ];

    int32_t buff_loc;
    uint32_t direction;
    uint32_t cur_fw_ctx_id;

    /* conf synchronization and completion */
    dma_completion_callback complete_func;
    uint32_t nents_done;
    struct completion comp;
} system_cdma_device_t;

volatile struct tasklet_struct head_tasklet;
SemaphoreHandle_t xTaskletSemaphore = NULL;
StaticSemaphore_t xTaskletMutexBuffer;

static void tasklet_schedule( struct tasklet_struct * new_tasklet )
{
    struct tasklet_struct * last;

    xSemaphoreTake( xTaskletSemaphore, portMAX_DELAY );
    new_tasklet->next = NULL;

    if( !head_tasklet.next ) /* Needed to avoid dropping volatile qualifier */
    {
        head_tasklet.next = new_tasklet;
    }
    else
    {
        last = head_tasklet.next;

        while( last->next )
        {
            last = last->next;
        }

        last->next = new_tasklet;
    }

    xSemaphoreGive( xTaskletSemaphore );
}

static void cdma_processing_thread( void * pvParameters )
{
    struct tasklet_struct * current_tasklet;

    while( 1 )
    {
        xSemaphoreTake( xTaskletSemaphore, portMAX_DELAY );
        current_tasklet = head_tasklet.next;

        if( current_tasklet )
        {
            head_tasklet.next = current_tasklet->next;
            xSemaphoreGive( xTaskletSemaphore );
            taskYIELD();
            current_tasklet->func( current_tasklet->data );
        }
        else
        {
            xSemaphoreGive( xTaskletSemaphore );
            vTaskDelay( pdMS_TO_TICKS( 10 ) );
            taskYIELD();
        }
    }
}

static void init_completion( struct completion * comp )
{
    comp->xSemaphore = xSemaphoreCreateBinaryStatic( &comp->xSemaphoreBuffer );
}

static void wait_for_completion( struct completion * comp )
{
    if( comp->xSemaphore != NULL )
    {
        xSemaphoreTake( comp->xSemaphore, portMAX_DELAY );
        vSemaphoreDelete( comp->xSemaphore );
        comp->xSemaphore = NULL;
    }
}

static void complete( struct completion * comp )
{
    if( comp->xSemaphore != NULL )
    {
        xSemaphoreGive( comp->xSemaphore );
    }
}

void system_cdma_setup()
{
    xTaskletSemaphore = xSemaphoreCreateMutexStatic( &xTaskletMutexBuffer );
    xTaskCreate( cdma_processing_thread,
                 "cdma",
                 configMINIMAL_STACK_SIZE,
                 NULL,
                 ( configMAX_PRIORITIES - 3 ) | portPRIVILEGE_BIT,
                 NULL );
}

#define MAX_DMA_MEM_ADDR_PAIRS    2

int cdma_mode = CDMA_MODE_MEMCPY;

/* NOTE: expand list if NUM_CDMA_DEVICES change */
#define NUM_CDMA_DEVICES    2
system_cdma_device_t pool_cdma_devices[ NUM_CDMA_DEVICES ];
system_cdma_device_t * avail_cdma_devices[ NUM_CDMA_DEVICES ] = { &pool_cdma_devices[ 0 ], &pool_cdma_devices[ 1 ] };

int32_t system_cdma_init( void ** ctx,
                          uint8_t mode,
                          void ** ping_only_reg_chg_track_mem )
{
    int32_t result = 0;
    system_cdma_device_t * system_cdma_device;

    if( ctx != NULL )
    {
        int32_t i, j, idx;

        for( i = 0; i < NUM_CDMA_DEVICES; i++ )
        {
            if( avail_cdma_devices[ i ] )
            {
                system_cdma_device = avail_cdma_devices[ i ];
                *ctx = system_cdma_device;
                avail_cdma_devices[ i ] = NULL;
                break;
            }
        }

        if( i == NUM_CDMA_DEVICES )
        {
            LOG( LOG_CRIT, "No memory for ctx" );
            return -1;
        }

        system_cdma_device->name = "TSK_DMA";

        for( idx = 0; idx < FIRMWARE_CONTEXT_NUMBER; idx++ )
        {
            for( i = 0; i < SYSTEM_DMA_TOGGLE_COUNT; i++ )
            {
                system_cdma_device->sg_device_nents[ idx ][ i ] = 0;
                system_cdma_device->sg_fwmem_nents[ idx ][ i ] = 0;

                for( j = 0; j < SYSTEM_DMA_MAX_CHANNEL; j++ )
                {
                    system_cdma_device->mem_addrs[ idx ][ i ][ j ] = ( mem_addr_pair_t ) {
                        0, 0, 0, 0
                    };
                }
            }
        }
    }
    else
    {
        result = -1;
        LOG( LOG_ERR, "Input ctx pointer is NULL" );
    }

    return result;
}

int32_t system_cdma_destroy( void * ctx )
{
    int32_t result = -1;
    system_cdma_device_t * system_cdma_device;

    if( ctx != 0 )
    {
        int32_t i;
        system_cdma_device = ( system_cdma_device_t * ) ctx;

        for( i = 0; i < NUM_CDMA_DEVICES; i++ )
        {
            /* Check if already destroyed */
            if( avail_cdma_devices[ i ] == system_cdma_device )
            {
                LOG( LOG_ERR, "Input ctx already destroyed" );
                result = -1;
                break;
            }

            /* Validate CTX */
            if( &pool_cdma_devices[ i ] == system_cdma_device )
            {
                result = 0;
            }
        }

        if( result != 0 )
        {
            LOG( LOG_ERR, "Input ctx pointer is INVALID" );
        }
        else
        {
            for( i = 0; i < NUM_CDMA_DEVICES; i++ )
            {
                if( avail_cdma_devices[ i ] == NULL )
                {
                    avail_cdma_devices[ i ] = system_cdma_device;
                    break;
                }
            }
        }
    }
    else
    {
        LOG( LOG_ERR, "Input ctx pointer is NULL" );
    }

    return result;
}

static void dma_complete_func( void * ctx )
{
    LOG( LOG_DEBUG, "\nIRQ completion called" );
    system_cdma_device_t * system_cdma_device = ( system_cdma_device_t * ) ctx;

    unsigned int nents_done = atomic_inc_return( &system_cdma_device->nents_done );

    if( nents_done
        >= system_cdma_device->sg_device_nents[ system_cdma_device->cur_fw_ctx_id ][ system_cdma_device->buff_loc ] )
    {
        if( system_cdma_device->complete_func )
        {
            system_cdma_device->complete_func( ctx, system_cdma_device->cur_fw_ctx_id );
            LOG( LOG_DEBUG,
                 "async completed on buff:%d dir:%d",
                 system_cdma_device->buff_loc,
                 system_cdma_device->direction );
        }
        else
        {
            complete( &system_cdma_device->comp );
            LOG( LOG_DEBUG,
                 "sync completed on buff:%d dir:%d",
                 system_cdma_device->buff_loc,
                 system_cdma_device->direction );
        }
    }
}

int32_t system_cdma_sg_device_setup( void * ctx,
                                     int32_t buff_loc,
                                     dma_addr_pair_t * device_addr_pair,
                                     int32_t addr_pairs,
                                     uint32_t fw_ctx_id )
{
    system_cdma_device_t * system_cdma_device = ( system_cdma_device_t * ) ctx;
    int i;

    if( !system_cdma_device || !device_addr_pair || !addr_pairs || ( buff_loc >= SYSTEM_DMA_TOGGLE_COUNT ) ||
        ( addr_pairs > SYSTEM_DMA_MAX_CHANNEL ) || ( fw_ctx_id >= FIRMWARE_CONTEXT_NUMBER ) )
    {
        return -1;
    }

    system_cdma_device->sg_device_nents[ fw_ctx_id ][ buff_loc ] = addr_pairs;

    if( !system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ] )
    {
        LOG( LOG_CRIT, "Failed to allocate virtual address pairs for flushing!!" );
        return -1;
    }

    for( i = 0; i < addr_pairs; i++ )
    {
        system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ][ i ].dev_addr =
            ioremap( device_addr_pair[ i ].address, device_addr_pair[ i ].size );
        system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ][ i ].size = device_addr_pair[ i ].size;
        system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ][ i ].sys_back_ptr = ctx;
    }

    LOG( LOG_INFO, "dma device setup success %d", system_cdma_device->sg_device_nents[ fw_ctx_id ][ buff_loc ] );
    return 0;
}

int32_t system_cdma_sg_fwmem_setup( void * ctx,
                                    int32_t buff_loc,
                                    fwmem_addr_pair_t * fwmem_pair,
                                    int32_t addr_pairs,
                                    uint32_t fw_ctx_id )
{
    int i;
    system_cdma_device_t * system_cdma_device = ( system_cdma_device_t * ) ctx;

    if( !system_cdma_device || !fwmem_pair || !addr_pairs || ( buff_loc >= SYSTEM_DMA_TOGGLE_COUNT ) ||
        ( fw_ctx_id >= FIRMWARE_CONTEXT_NUMBER ) )
    {
        LOG( LOG_CRIT, "null param problems" );
        return -1;
    }

    system_cdma_device->sg_fwmem_nents[ fw_ctx_id ][ buff_loc ] = addr_pairs;

    if( !system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ] )
    {
        LOG( LOG_CRIT, "Failed to allocate virtual address pairs for flushing!!" );
        return -1;
    }

    for( i = 0; i < addr_pairs; i++ )
    {
        system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ][ i ].fw_addr = fwmem_pair[ i ].address;
        system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ][ i ].size = fwmem_pair[ i ].size;
        system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ][ i ].sys_back_ptr = ctx;
    }

    LOG( LOG_INFO, "fwmem setup success %d", system_cdma_device->sg_device_nents[ fw_ctx_id ][ buff_loc ] );

    return 0;
}

static void memcopy_func( unsigned long p_task )
{
    mem_tasklet_t * mem_task = ( mem_tasklet_t * ) p_task;
    mem_addr_pair_t * mem_addr = ( mem_addr_pair_t * ) mem_task->mem_data;
    system_cdma_device_t * system_cdma_device = ( system_cdma_device_t * ) mem_addr->sys_back_ptr;
    void * src_mem = 0;
    void * dst_mem = 0;

    int32_t buff_loc = system_cdma_device->buff_loc;
    uint32_t direction = system_cdma_device->direction;

    if( direction == SYS_DMA_TO_DEVICE )
    {
        src_mem = mem_addr->fw_addr;
        dst_mem = mem_addr->dev_addr;
    }
    else
    {
        dst_mem = mem_addr->fw_addr;
        src_mem = mem_addr->dev_addr;
    }

    memcpy( dst_mem, src_mem, mem_addr->size );

    LOG( LOG_DEBUG, "(%d:%d) d:%p s:%p l:%ld", buff_loc, direction, dst_mem, src_mem, mem_addr->size );

    dma_complete_func( mem_addr->sys_back_ptr );
}

void system_cdma_unmap_sg( void * ctx )
{
}

int32_t system_cdma_copy_sg( void * ctx,
                             int32_t buff_loc,
                             uint32_t direction,
                             dma_completion_callback complete_func,
                             uint32_t fw_ctx_id )
{
    int32_t i, result = 0;

    if( !ctx )
    {
        LOG( LOG_ERR, "Input ctx pointer is NULL" );
        return -1;
    }

    int32_t async_dma = 0;

    if( complete_func != NULL )
    {
        async_dma = 1;
    }

    system_cdma_device_t * system_cdma_device = ( system_cdma_device_t * ) ctx;

    unsigned int src_nents = system_cdma_device->sg_device_nents[ fw_ctx_id ][ buff_loc ];
    unsigned int dst_nents = system_cdma_device->sg_fwmem_nents[ fw_ctx_id ][ buff_loc ];

    if( ( src_nents != dst_nents ) || !src_nents )
    {
        LOG( LOG_CRIT, "Unbalance src_nents:%d dst_nents:%d", src_nents, dst_nents );
        return -1;
    }

    system_cdma_device->cur_fw_ctx_id = fw_ctx_id;

    atomic_set( &system_cdma_device->nents_done, 0 ); /* set the number of nents done */

    if( async_dma == 0 )
    {
        system_cdma_device->complete_func = NULL; /* async mode is not allowed to have callback */
        init_completion( &system_cdma_device->comp );
    }
    else
    {
        system_cdma_device->complete_func = complete_func; /* call this function if all nents are done; */
    }

    system_cdma_device->direction = direction;
    system_cdma_device->buff_loc = buff_loc;

    for( i = 0; i < SYSTEM_DMA_MAX_CHANNEL; i++ )
    {
        system_cdma_device->task_list[ fw_ctx_id ][ buff_loc ][ i ].mem_data =
            &( system_cdma_device->mem_addrs[ fw_ctx_id ][ buff_loc ][ i ] );
        system_cdma_device->task_list[ fw_ctx_id ][ buff_loc ][ i ].m_task.data =
            ( unsigned long ) &system_cdma_device->task_list[ fw_ctx_id ][ buff_loc ][ i ];
        system_cdma_device->task_list[ fw_ctx_id ][ buff_loc ][ i ].m_task.func = memcopy_func;
        tasklet_schedule( &system_cdma_device->task_list[ fw_ctx_id ][ buff_loc ][ i ].m_task );
    }

    if( async_dma == 0 )
    {
        LOG( LOG_DEBUG, "scatterlist DMA waiting completion\n" );
        wait_for_completion( &system_cdma_device->comp );
    }

    LOG( LOG_DEBUG, "scatterlist DMA success\n" );
    return result;
}
