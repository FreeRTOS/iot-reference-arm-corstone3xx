# IoT Reference Integration for Corstone-3xx

## Introduction

This reference integration demonstrates how to develop cloud connected
applications and update them securely by integrating modular
[FreeRTOS kernel](https://www.freertos.org/RTOS.html) and [libraries](https://www.freertos.org/libraries/categories.html)
and utilizing hardware enforced security based on [Arm TrustZone (Armv8-M)](https://www.arm.com/architecture/learn-the-architecture/m-profile).
This project is based on the [Corstone-300](https://developer.arm.com/Processors/Corstone-300)
platform.

## Arm TrustZone (Armv8-M)

TrustZone technology for Armv8-M is an optional Security Extension that is
designed to provide a foundation for improved system security in a wide range
of embedded applications. Follow the [link](https://developer.arm.com/documentation/100690/0201/Arm-TrustZone-technology)
for more information on TrustZone technology for Armv8-M.

## Trusted Firmware M

Trusted Firmware-M (TF-M) implements the Secure Processing Environment (SPE)
for Armv8-M, Armv8.1-M architectures (e.g. the Cortex-M33, Cortex-M23,
Cortex-M55, Cortex-M85 processors) and dual-core platforms. It is the platform
security architecture reference implementation aligning with PSA Certified
guidelines, enabling chips, Real Time Operating Systems and devices to become
PSA Certified. Follow the [link](https://tf-m-user-guide.trustedfirmware.org/introduction/readme.html)
for more information on Trusted Firmware M.

## PKCS#11 PSA Shim

PKCS#11 PSA shim layer provides a reference implementation of PKCS#11 API based
on [Platform Security Architecture](https://www.arm.com/architecture/psa-certified)
API.

This shim layer maps the PKCS#11 APIs to PSA Cryptography and Storage APIs
V1.0. It follows the same PSA Cryptography API version supported in
[Mbed TLS 3.4.0](https://github.com/Mbed-TLS/mbedtls/tree/mbedtls-3.4.0).
Certificate objects and key objects are protected by PSA secure service.
By default, the device private/public keys are persistent while the code verify
key is volatile.

## AWS OTA PAL PSA implementation

Implementation of AWS OTA PAL based on [Platform Security Architecture](https://www.arm.com/architecture/psa-certified)
API.

This implementation maps the AWS OTA PAL APIs to the PSA Firmware Update and
PSA Cryptography APIs. The writing, verification and activation of the update
image are protected by the PSA secure services.

## Examples

This reference integration contains following two examples:

* [Blinky example](Docs/blinky.md)
* [AWS IoT example](Docs/aws-iot-example.md)

## Contributing

See [CONTRIBUTING](CONTRIBUTING.md) for more information.

## License

Source code located in the *Projects*, *Middleware/FreeRTOS* directory is
available under the terms of the MIT License. See the [LICENSE](./LICENSE) file
for more details.

Other files in the repository are available under the terms specified in each
source file.
