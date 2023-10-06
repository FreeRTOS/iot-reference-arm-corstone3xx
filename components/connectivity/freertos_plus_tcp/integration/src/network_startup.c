/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "FreeRTOS_IP.h"

/* System events helper header. */
#include "events.h"

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Configure name and log level. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "NW_STARTUP"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

/* The default IP and MAC address used by the demo.  The address configuration
 * defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
 * 1 but a DHCP server could not be contacted.  See the online documentation for
 * more information. */
static const uint8_t ucIPAddress[ 4 ] =
{
    configIP_ADDR0,
    configIP_ADDR1,
    configIP_ADDR2,
    configIP_ADDR3
};
static const uint8_t ucNetMask[ 4 ] =
{
    configNET_MASK0,
    configNET_MASK1,
    configNET_MASK2,
    configNET_MASK3
};
static const uint8_t ucGatewayAddress[ 4 ] =
{
    configGATEWAY_ADDR0,
    configGATEWAY_ADDR1,
    configGATEWAY_ADDR2,
    configGATEWAY_ADDR3
};
static const uint8_t ucDNSServerAddress[ 4 ] =
{
    configDNS_SERVER_ADDR0,
    configDNS_SERVER_ADDR1,
    configDNS_SERVER_ADDR2,
    configDNS_SERVER_ADDR3
};
const uint8_t ucMACAddress[ 6 ] =
{
    configMAC_ADDR0,
    configMAC_ADDR1,
    configMAC_ADDR2,
    configMAC_ADDR3,
    configMAC_ADDR4,
    configMAC_ADDR5
};

#if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )

/* In case multiple interfaces are used, define them statically. */

/* There is only 1 physical interface. */
    static NetworkInterface_t xInterfaces[ 1 ];

/* It will have several end-points. */
    static NetworkEndPoint_t xEndPoints[ 4 ];

#endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */

int32_t network_startup( void )
{
    #if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
        /* Initialise the interface descriptor for WinPCap. */
        pxLAN91C111_FillInterfaceDescriptor( 0, &( xInterfaces[ 0 ] ) );

        /* === End-point 0 === */
        FreeRTOS_FillEndPoint( &( xInterfaces[ 0 ] ), &( xEndPoints[ 0 ] ), ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
    #if ( ipconfigUSE_DHCP != 0 )
        {
            /* End-point 0 wants to use DHCPv4. */
            xEndPoints[ 0 ].bits.bWantDHCP = pdTRUE;
        }
        #endif /* ( ipconfigUSE_DHCP != 0 ) */

        memcpy( ipLOCAL_MAC_ADDRESS, ucMACAddress, sizeof( ucMACAddress ) );

        FreeRTOS_IPInit_Multi();
    #else /* if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */
        /* Using the old /single /IPv4 library, or using backward compatible mode of the new /multi library. */
        FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
    #endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */

    return 0;
}

/* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect
 * events are only received if implemented in the MAC driver. */
#if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
    void vApplicationIPNetworkEventHook_Multi( eIPCallbackEvent_t eNetworkEvent,
                                               struct xNetworkEndPoint * pxEndPoint )
#else
    void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
#endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */
{
    /* If the network has just come up...*/
    if( eNetworkEvent == eNetworkUp )
    {
        LogInfo( ( "Network is up" ) );
        ( void ) xEventGroupSetBits( xSystemEvents, EVENT_MASK_NETWORK_UP );
    }
    else
    {
        LogInfo( ( "Network is down" ) );
        ( void ) xEventGroupClearBits( xSystemEvents, EVENT_MASK_NETWORK_UP );
    }
}
