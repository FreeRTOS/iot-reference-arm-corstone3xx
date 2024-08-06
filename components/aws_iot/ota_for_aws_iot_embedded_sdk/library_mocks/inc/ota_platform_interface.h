/*
 * AWS IoT Over-the-air Update v3.4.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 *
 * SPDX-License-Identifier: MIT
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
 */

#ifndef OTA_PLATFORM_INTERFACE
#define OTA_PLATFORM_INTERFACE

typedef enum
{
    otaPal_Abort = 1,
} OtaPalAbort_t;
typedef enum
{
    otaPal_CreateFileForRx = 1,
} OtaPalCreateFileForRx_t;
typedef enum
{
    otaPal_ResetDevice = 1,
} OtaPalResetDevice_t;
typedef enum
{
    otaPal_CloseFile = 1,
} OtaPalCloseFile_t;
typedef enum
{
    otaPal_ActivateNewImage = 1,
} OtaPalActivateNewImage_t;
typedef enum
{
    otaPal_WriteBlock = 1,
} OtaPalWriteBlock_t;
typedef enum
{
    otaPal_SetPlatformImageState = 1,
} OtaPalSetPlatformImageState_t;
typedef enum
{
    otaPal_GetPlatformImageState = 1,
} OtaPalGetPlatformImageState_t;

typedef struct OtaPalInterface
{
    OtaPalAbort_t abort;
    OtaPalCreateFileForRx_t createFile;
    OtaPalCloseFile_t closeFile;
    OtaPalWriteBlock_t writeBlock;
    OtaPalActivateNewImage_t activate;
    OtaPalResetDevice_t reset;
    OtaPalSetPlatformImageState_t setPlatformImageState;
    OtaPalGetPlatformImageState_t getPlatformImageState;
} OtaPalInterface_t;

#endif /* OTA_PLATFORM_INTERFACE */
