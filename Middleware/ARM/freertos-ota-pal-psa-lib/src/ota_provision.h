/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OTA_PROVISION_
    #define _OTA_PROVISION_
    #include "psa/crypto.h"
    #ifdef __cplusplus
    extern "C" {
    #endif

    int ota_privision_code_signing_key( psa_key_handle_t * key_handle );

    #ifdef __cplusplus
}
    #endif
#endif /* ifndef _OTA_PROVISION_ */
