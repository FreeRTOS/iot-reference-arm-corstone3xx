/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "provisioning_config_target.h"

/* *INDENT-OFF* */

#ifndef _PROVISIONING_CONFIG_H_
#define _PROVISIONING_CONFIG_H_

#define PROVISIONING_MAGIC               0xC0DEFEED
#define PROVISIONING_DATA_LEN            ( 0x1000 )
#define PROVISIONING_DATA_HEADER_SIZE    ( 0x4 )
#define PROVISIONING_PARAM_START         ( PROVISIONING_DATA_START + PROVISIONING_DATA_HEADER_SIZE )

#endif /* _PROVISIONING_CONFIG_H_ */

/* *INDENT-ON* */
