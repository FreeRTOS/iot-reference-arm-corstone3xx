/*
 * Copyright (c) 2023 Arm Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stdint.h"

typedef uint32_t acamera_uart_t;

void acamera_uart_init( acamera_uart_t * uart );

int acamera_uart_read( void * p_ctrl,
                       uint8_t * data,
                       int size );
int acamera_uart_write( void * p_ctrl,
                        const uint8_t * data,
                        int size );
