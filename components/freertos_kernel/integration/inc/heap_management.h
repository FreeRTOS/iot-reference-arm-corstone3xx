/* Copyright 2025 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

/**
 * @file heap_management.h
 * @brief Thin wrappers around the C library used by the integration code.
 */
#ifndef HEAP_MANAGEMENT_H
    #define HEAP_MANAGEMENT_H

    #include <stddef.h>

    #ifdef __cplusplus
    extern "C" {
    #endif

/**
 * @brief Wrapper for the C libraries malloc
 * @param xWantedSize The number of bytes to be allocated
 * @return Either a non-NULL pointer to a block of memory on success, or NULL on failure
 */
    void * pvPortMalloc( size_t xWantedSize );

/**
 * @brief Wrapper for the C libraries free function
 * @param pv Pointer to a memory location to be freed, or NULL
 */
    void vPortFree( void * pv );

/**
 * @brief Wrapper for the C libraries calloc
 * @param xNum The number of elements
 * @param xSize The size of each element in bytes
 * @return Either a non-NULL pointer to a block of memory on success, or NULL on failure
 */
    void * pvPortCalloc( size_t xNum,
                         size_t xSize );

/**
 * @brief A dummy implementation as C standard library does not provide
 * functions to get the statistics of heap memory.
 * @return Always 0 in the integration build
 * @note These dummy implementation are needed
 * as this API is used as part of FreeRTOS Plus TCP code which is unused in the
 * FRI code (removed by the linker) but ARMClang linker requires all the compiled
 * symbols to be defined.
 */
    size_t xPortGetFreeHeapSize( void );

/**
 * @brief A dummy implementation as C standard library does not provide
 * functions to get the statistics of heap memory.
 * @return Always 0 in the integration build
 * @note These dummy implementation are needed
 * as this API is used as part of FreeRTOS Plus TCP code which is unused in the
 * FRI code (removed by the linker) but ARMClang linker requires all the compiled
 * symbols to be defined.
 */
    size_t xPortGetMinimumEverFreeHeapSize( void );

    #ifdef __cplusplus
    }
    #endif

#endif /* HEAP_MANAGEMENT_H */
