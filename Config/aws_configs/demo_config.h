/*
 * FreeRTOS V202010.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 */

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

#include "core_mqtt.h"

/**
 * @brief The name of the operating system that the application is running on.
 * The current value is given as an example. Please update for your specific
 * operating system.
 */
#define democonfigOS_NAME       "FreeRTOS"

/**
 * @brief The version of the operating system that the application is running
 * on. The current value is given as an example. Please update for your specific
 * operating system version.
 */
#define democonfigOS_VERSION    "V10.4.3"

/**
 * @brief The name of the MQTT library used and its version, following an "@"
 * symbol.
 */
#define democonfigMQTT_LIB      "core-mqtt@" MQTT_LIBRARY_VERSION

/**
 * @brief The MQTT metrics string expected by AWS IoT.
 */
#define democonfigMQTT_USERNAME                               \
    "?SDK=" democonfigOS_NAME "&Version=" democonfigOS_VERSION \
    "&MQTTLib=" democonfigMQTT_LIB

/**
 * @brief The length of the MQTT metrics string expected by AWS IoT.
 */
#define democonfigMQTT_USERNAME_LENGTH    ( ( uint16_t ) ( sizeof( democonfigMQTT_USERNAME ) - 1 ) )

/**
 * To use this demo, please configure the client's certificate and private key
 * in demos/include/aws_clientcredential_keys.h.
 * 
 * For the AWS IoT MQTT broker, refer to the AWS documentation below for details
 * regarding client authentication.
 * https://docs.aws.amazon.com/iot/latest/developerguide/client-authentication.html
 */

/**
 * @brief The MQTT client identifier used in this example.  Each client identifier
 * must be unique; so edit as required to ensure that no two clients connecting to
 * the same broker use the same client identifier.
 *
 * #define democonfigCLIENT_IDENTIFIER    "insert here."
 */

/*
#define democonfigCLIENT_IDENTIFIER    ""
*/

#ifndef democonfigCLIENT_IDENTIFIER
#error "Uncomment the democonfigCLIENT_IDENTIFIER macro above and insert MQTT client identifier"
#endif /* democonfigCLIENT_IDENTIFIER */

/**
 * @brief Endpoint of the MQTT broker to connect to.
 *
 * This demo application can be run with any MQTT broker, that supports mutual
 * authentication.
 *
 * For AWS IoT MQTT broker, this is the Thing's REST API Endpoint.
 *
 * @note Your AWS IoT Core endpoint can be found in the AWS IoT console under
 * Settings/Custom Endpoint, or using the describe-endpoint REST API (with
 * AWS CLI command line tool).
 *
 * #define democonfigMQTT_BROKER_ENDPOINT    ""
 */

/*
 #define democonfigMQTT_BROKER_ENDPOINT    ""
*/

#ifndef democonfigMQTT_BROKER_ENDPOINT
#error "Uncomment the democonfigMQTT_BROKER_ENDPOINT macro above and insert AWS IoT Core endpoint"
#endif /* democonfigMQTT_BROKER_ENDPOINT */

/**
 * @brief The port to use for the demo.
 *
 * In general, port 8883 is for secured MQTT connections.
 *
 * @note Port 443 requires use of the ALPN TLS extension with the ALPN protocol
 * name. When using port 8883, ALPN is not required.
 *
 * #define democonfigMQTT_BROKER_PORT    "...insert here..."
 */
#define democonfigMQTT_BROKER_PORT 8883

#endif /* DEMO_CONFIG_H */
