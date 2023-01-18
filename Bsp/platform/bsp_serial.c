/* Copyright 2017-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include "device_cfg.h"
#include "Driver_USART.h"
#include "bsp_serial.h"

extern ARM_DRIVER_USART Driver_USART0;

void bsp_serial_init(void)
{
    Driver_USART0.Initialize(NULL);
    Driver_USART0.Control(ARM_USART_MODE_ASYNCHRONOUS, DEFAULT_UART_BAUDRATE);
}

void bsp_serial_print(char *str)
{
    (void)Driver_USART0.Send(str, strlen(str));
}

/* Redirects gcc printf to UART0 */
int _write(int fd, char *str, int len)
{
    if (Driver_USART0.Send(str, len) == ARM_DRIVER_OK) {
        return len;
    }
    return 0;
}
