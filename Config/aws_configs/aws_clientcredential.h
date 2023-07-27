/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef __AWS_CLIENTCREDENTIAL__H__
#define __AWS_CLIENTCREDENTIAL__H__

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
 */

/* #define clientcredentialMQTT_BROKER_ENDPOINT         "" */

#ifndef clientcredentialMQTT_BROKER_ENDPOINT
    #error "Uncomment the clientcredentialMQTT_BROKER_ENDPOINT macro above and insert AWS IoT Core endpoint"
#endif /* clientcredentialMQTT_BROKER_ENDPOINT */

/**
 * @brief The MQTT client identifier used in this example.  Each client identifier
 * must be unique; so edit as required to ensure that no two clients connecting to
 * the same broker use the same client identifier.
 *
 * Value is defined in "aws_clientcredential.h".
 */

/* #define clientcredentialIOT_THING_NAME               "" */

#ifndef clientcredentialIOT_THING_NAME
    #error "Uncomment the clientcredentialIOT_THING_NAME macro above and insert MQTT client identifier"
#endif /* clientcredentialIOT_THING_NAME */

/**
 * @brief The port to use for the demo.
 *
 * In general, port 8883 is for secured MQTT connections.
 *
 * @note Port 443 requires use of the ALPN TLS extension with the ALPN protocol
 * name. When using port 8883, ALPN is not required.
 *
 */
#define clientcredentialMQTT_BROKER_PORT             8883

#define clientcredentialGREENGRASS_DISCOVERY_PORT    8443
#define clientcredentialWIFI_SSID                    ""
#define clientcredentialWIFI_PASSWORD                ""
#define clientcredentialWIFI_SECURITY                eWiFiSecurityWPA2
#endif /* ifndef __AWS_CLIENTCREDENTIAL__H__ */
