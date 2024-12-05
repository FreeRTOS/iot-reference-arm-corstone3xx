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

#ifndef OTA_ORCHESTRATOR_HELPERS_H
#define OTA_ORCHESTRATOR_HELPERS_H

/* Standard includes. */
#include <stdbool.h>
#include <stdint.h>

#include "job_parser.h"

/**
 * @brief Convert a job document signature into DER format.
 *
 * @param[in] dest The destination buffer to be populated with the decoded
 * signature
 * @param[in] destLength Length of the dest buffer.
 * @param[in] jobFields Pointer to the structure holding the job document
 * parameters, which will be populated with the decoded signature.
 *
 * @return true if the signature was successfully decoded, otherwise false.
 */
bool convertSignatureToDER( uint8_t * dest,
                            size_t destLength,
                            AfrOtaJobDocumentFields_t * jobFields );

/**
 * @brief Parse the job document to extract the parameters needed to download
 * the new firmware.
 *
 * @param[in] message The jobs message received from AWS IoT core.
 * @param[in] messageLength Length of the message.
 * @param[in] jobFields Pointer to the structure to be populated with the job
 * document fields.
 *
 * @return true if all files processed, otherwise false.
 */
bool jobDocumentParser( char * message,
                        size_t messageLength,
                        AfrOtaJobDocumentFields_t * jobFields );

#endif /* OTA_ORCHESTRATOR_HELPERS_H */
