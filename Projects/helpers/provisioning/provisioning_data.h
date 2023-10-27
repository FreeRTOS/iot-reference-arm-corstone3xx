/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef _PROVISIONING_DATA_H_
#define _PROVISIONING_DATA_H_

#include <stdint.h>
#include "provisioning_config.h"

typedef struct ProvisioningParams_t
{
    uint8_t * pucClientPrivateKey;      /**< Pointer to the device private key in PEM format.
                                         *   See tools/certificate_configuration/PEMfileToCString.html
                                         *   for help with formatting.*/
    uint32_t ulClientPrivateKeyLength;  /**< Length of the private key data, in bytes. */
    uint8_t * pucClientCertificate;     /**< Pointer to the device certificate in PEM format.
                                         *   See tools/certificate_configuration/PEMfileToCString.html
                                         *   for help with formatting.*/
    uint32_t ulClientCertificateLength; /**< Length of the device certificate in bytes. */
    uint8_t * pucJITPCertificate;       /**< Pointer to the Just-In-Time Provisioning (JITP) certificate in
                                         *   PEM format.
                                         *   - This is REQUIRED if JITP is being used.
                                         *   - If you are not using JITP, this certificate
                                         *   is not needed and should be set to NULL.
                                         *   - See tools/certificate_configuration/PEMfileToCString.html
                                         *   for help with formatting.
                                         *   - See https://aws.amazon.com/blogs/iot/setting-up-just-in-time-provisioning-with-aws-iot-core/
                                         *   for more information about getting started with JITP */
    uint32_t ulJITPCertificateLength;   /**< Length of the Just-In-Time Provisioning (JITP) certificate in bytes.
                                         *   If JITP is not being used, this value should be set to 0. */
} ProvisioningParams_t;

typedef struct ProvisioningParamsBundle_t
{
    uint32_t provisioningMagic1;
    ProvisioningParams_t provisioningParams;
    uint8_t * codeSigningPublicKey;
    uint32_t provisioningMagic2;
} ProvisioningParamsBundle_t;


#endif /* ifndef _PROVISIONING_DATA_H_ */
