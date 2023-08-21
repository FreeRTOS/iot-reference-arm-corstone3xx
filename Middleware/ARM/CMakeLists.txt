# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_subdirectory(TF-M)
add_subdirectory(mbedtls-lib)
if (DEFINED CONNECTIVITY_STACK)
    if (${CONNECTIVITY_STACK} STREQUAL "IOT_VSOCKET")
        add_subdirectory(IoT_Socket-lib)
        add_subdirectory(IoT_VSocket-lib)
    endif()
endif()
add_subdirectory(freertos-pkcs11-psa-lib)
add_subdirectory(freertos-ota-pal-psa-lib)
