/* Copyright 2021-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include "FreeRTOS.h"
#include "events.h"

int32_t network_startup( void )
{
    /* The iot-vsocket implementation utilizes the underlying host network
     * stack for network operation. It is assumed that the host network is
     * already operational by the time the application runs. */
    EventBits_t uxBits = xEventGroupSetBits( xSystemEvents, EVENT_MASK_NETWORK_UP );

    if( !( uxBits & EVENT_MASK_NETWORK_UP ) )
    {
        /* During this stage of application intialisation, there are no other
         * tasks active except logging task. Hence the value returned by
         * xEventGroupSetBits must have EVENT_MASK_NETWORK_UP bit set. If not,
         * something went wrong during initialisation. Trigger an assert to
         * catch this. */
        configASSERT( 0 );
    }

    return 0;
}
