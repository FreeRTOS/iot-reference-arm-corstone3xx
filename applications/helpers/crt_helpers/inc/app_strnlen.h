/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef APP_STRNLEN_H

#include <stddef.h>

/**
 * @brief Determine the length of a fixed-size string, excluding the terminating
 * null byte ('\0'), and at most `maxlen`.
 *
 * @param[in] s The string to determine the length of.
 * @param[in] maxlen The maximum number of characters of the string `s` that
 * should be checked.
 *
 * @return The length of the string, up to `maxlen`.
 */
size_t app_strnlen( const char * s,
                    size_t maxlen );

#endif
