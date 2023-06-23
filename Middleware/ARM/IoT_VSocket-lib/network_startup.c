/* Copyright 2021-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

extern int32_t socket_startup (void);

int32_t network_startup (void) {
  return socket_startup();
}
