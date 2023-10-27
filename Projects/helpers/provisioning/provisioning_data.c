/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "provisioning_data.h"
#include "aws_clientcredential_keys.h"

const ProvisioningParamsBundle_t provisioningBundle =
{
    .provisioningMagic1       = PROVISIONING_MAGIC,
    .provisioningParams       =
    {
        .pucJITPCertificate   = keyJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM,
        .pucClientCertificate = keyCLIENT_CERTIFICATE_PEM,
        .pucClientPrivateKey  = keyCLIENT_PRIVATE_KEY_PEM
    },
    .codeSigningPublicKey     = keyCODE_SIGNING_PUBLIC_KEY_PEM,
    .provisioningMagic2       = PROVISIONING_MAGIC
};
