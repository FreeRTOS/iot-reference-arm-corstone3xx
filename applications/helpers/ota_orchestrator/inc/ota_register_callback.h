/* Copyright 2023-2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef OTA_REGISTER_CALLBACK_H
#define OTA_REGISTER_CALLBACK_H

/* Standard includes. */
#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Register OTA callbacks with the subscription manager.
 *
 * @param[in] pTopicFilter The topic filter for which a callback needs to be registered for.
 * @param[in] topicFilterLength Length of the topic filter.
 */
void prvRegisterOTACallback( const char * pTopicFilter,
                             uint16_t topicFilterLength );

#endif /* OTA_REGISTER_CALLBACK_H */
