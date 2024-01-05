# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Trusted Firmware-M needs to be added before
# the other security components as its build
# byproducts are listed in the others.
add_subdirectory(trusted_firmware-m)

add_subdirectory(freertos_ota_pal_psa)
add_subdirectory(freertos_pkcs11_psa)
add_subdirectory(mbedtls)
