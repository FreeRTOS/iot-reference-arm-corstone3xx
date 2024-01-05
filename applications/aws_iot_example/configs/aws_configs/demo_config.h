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
#include "aws_clientcredential.h"
#include "iot_default_root_certificates.h"

/**
 * To use this demo, please configure the client's certificate and private key
 * in aws_configs/aws_clientcredential_keys.h
 * and credentials in aws_configs/aws_clientcredential.h.
 *
 * For the AWS IoT MQTT broker, refer to the AWS documentation below for details
 * regarding client authentication.
 * https://docs.aws.amazon.com/iot/latest/developerguide/client-authentication.html
 */

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
#define democonfigMQTT_USERNAME                                \
    "?SDK=" democonfigOS_NAME "&Version=" democonfigOS_VERSION \
    "&MQTTLib=" democonfigMQTT_LIB

/**
 * @brief The length of the MQTT metrics string expected by AWS IoT.
 */
#define democonfigMQTT_USERNAME_LENGTH    ( ( uint16_t ) ( sizeof( democonfigMQTT_USERNAME ) - 1 ) )

/* these are set in "aws_clientcredential.h and iot_default_root_certificates.h" */
#define democonfigCLIENT_IDENTIFIER       clientcredentialIOT_THING_NAME
#define democonfigMQTT_BROKER_ENDPOINT    clientcredentialMQTT_BROKER_ENDPOINT
#define democonfigMQTT_BROKER_PORT        clientcredentialMQTT_BROKER_PORT
#define democonfigROOT_CA_PEM             tlsATS1_ROOT_CERTIFICATE_PEM

#endif /* DEMO_CONFIG_H */
