/* Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

/* Libseepx config */
#ifndef CONFIG_H
#define CONFIG_H


#define USE_KISS_FFT

/* We don't support visibility on Win32 */
#define EXPORT

#define SIZEOF_INT          4

/* The size of `int16_t', as computed by sizeof. */
#define SIZEOF_INT16_T      2

/* The size of `int32_t', as computed by sizeof. */
#define SIZEOF_INT32_T      4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG         4

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT        2

/* The size of `uint16_t', as computed by sizeof. */
#define SIZEOF_UINT16_T     2

/* The size of `uint32_t', as computed by sizeof. */
#define SIZEOF_UINT32_T     4

/* The size of `u_int16_t', as computed by sizeof. */
#define SIZEOF_U_INT16_T    0

/* The size of `u_int32_t', as computed by sizeof. */
#define SIZEOF_U_INT32_T    0


#endif /* ifndef CONFIG_H */
