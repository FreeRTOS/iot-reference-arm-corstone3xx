/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

/** @brief Set logging task as high priority task */
#define appCONFIG_LOGGING_TASK_PRIORITY           ( configMAX_PRIORITIES - 1 )
#define appCONFIG_LOGGING_TASK_STACK_SIZE         ( 2048 )
#define appCONFIG_LOGGING_MESSAGE_QUEUE_LENGTH    ( 32 )
