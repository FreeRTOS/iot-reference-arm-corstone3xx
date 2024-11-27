/*
 * Copyright Amazon.com, Inc. and its affiliates. All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 *
 * SPDX-License-Identifier: MIT
 * Licensed under the MIT License. See the LICENSE accompanying this file
 * for the specific language governing permissions and limitations under
 * the License.
 */

/**
 * @file MQTTFileDownloader_config.h
 * @brief MQTT File Streams options.
 */

#ifndef MQTT_FILE_DOWNLOADER_CONFIG_H
#define MQTT_FILE_DOWNLOADER_CONFIG_H

/* Standard includes */
#include <stdint.h>

/**
 * @ingroup mqtt_file_downloader_const_types
 * @brief Configure the Maximum size of the data payload. The
 * smallest value is 256 bytes, maximum is 128KB. For more see
 * https://docs.aws.amazon.com/general/latest/gr/iot-core.html
 */
#define mqttFileDownloader_CONFIG_BLOCK_SIZE    4096U

#endif /* MQTT_FILE_DOWNLOADER_CONFIG_H */
