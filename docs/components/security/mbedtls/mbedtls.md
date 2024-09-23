# MbedTLS

## Overview

MbedTLS is a lightweight crytographic and SSL/TLS library designed for embedded systems and IoT devices.

It provides a wide range of cryptographic and security features including:
* SSL/TLS support
* Cryptography
* Certificate handling
* Key Management
* Secure communication

In the FRI, MbedTLS is a crucial component for ensuring secure communication between IoT devices and
cloud services. It is primarily used for transport layer security, authentication, encryption and certificate management.

Consult the FRI `manifest.yml` for the currently supported version of MbedTLS.

## Configuration

User must provide an MbedTLS configuration file. It can be an empty file or override MbedTLS default [configurations](https://tls.mbed.org/api/config_8h.html).

The configuration file specified by the application is retrieved by adding the C macro `MBEDTLS_CONFIG_FILE=<filename>` to the `mbedtls-config` target and its include path.

Example:

```cmake
target_include_directories(mbedtls-config
    INTERFACE
        mbedtls-config
)

target_compile_definitions(mbedtls-config
    INTERFACE
        MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h"
)
```

To enable the FreeRTOS threading protection `#define MBEDTLS_THREADING_ALT` should be present in the user provided mbedtls configuration file.

### PSA Crypto APIs Implementation

PSA Crypto is part of Platform Security Architecture initiative that standardized crypto interfaces. The PSA Crypto specification is available [here](https://arm-software.github.io/psa-api/crypto/1.1/). Mbed TLS, and Trusted Firmware-M libraries provide implementation for PSA Crypto APIs.

User can choose which library is to be used for implementing PSA Crypto APIs used by Mbed TLS library running on the Non-Secure side through the build option `--psa-crypto-implementation <TF-M/MBEDTLS>`.

The default PSA Crypto APIs implementation is the one provided by Trusted Firmware-M library, where the `--psa-crypto-implementation` build option default value is `TF-M`.

In case of using Trusted Firmware-M as the PSA Crypto APIs implementation, `tfm-ns-interface` library has to be linked to the `mbedtls` target.

```cmake
target_link_libraries(mbedtls
    PUBLIC
        mbedtls-config
        tfm-ns-interface
)
```

## Integration

### FreeRTOS threading support

The library *`mbedtls-threading-freertos`* is implemented in the FRI to provide a threading implementation for MbedTLS using FreeRTOS threading API.

The application must call `mbedtls_threading_set_alt()` to enable the multi threading protection.

### Linking

In your application's `CMakeLists.txt`, link the application executable against the `mbedtls` library alongside
any other libraries you need:

```cmake
target_link_libraries(my-application
    ...
    mbedtls
)
```

> :bulb: Replace `my-application` with the actual name of your application.

This not only enables the linking of the `mbedtls` static library, but also makes its API headers' include paths
available to your application.

## Documentation

For detailed documentation and API reference of MbedTLS, refer to the official [MbedTLS documentation][mbedtls-doc] or [GitHub repository][mbedtls-doc].

## Support

If you encounter any issues or have questions regarding the integration of MbedTLS into your IoT
project, feel free to reach out to the Arm support community or consult the official documentation for
assistance.

[mbedtls-doc]: https://mbed-tls.readthedocs.io/en/latest/
[mbedtls-repo]: https://github.com/Mbed-TLS/mbedtls
