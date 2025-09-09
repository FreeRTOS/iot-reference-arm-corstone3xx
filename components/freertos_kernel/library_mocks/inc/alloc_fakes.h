/* Copyright 2025 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef ALLOC_FAKES_H
#define ALLOC_FAKES_H

#include <stddef.h>
#include "fff.h"

DECLARE_FAKE_VALUE_FUNC( void *, test_malloc, size_t );
DECLARE_FAKE_VALUE_FUNC( void *, test_calloc, size_t, size_t );
DECLARE_FAKE_VOID_FUNC( test_free, void * );

#endif /* ALLOC_FAKES_H*/
