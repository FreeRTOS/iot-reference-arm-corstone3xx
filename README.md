# IoT Reference Integration for Arm Corstone-3xx

## Introduction

This reference integration demonstrates how to develop cloud connected
applications and update them securely by integrating modular
[FreeRTOS kernel](https://www.freertos.org/RTOS.html) and [libraries](https://www.freertos.org/libraries/categories.html)
and utilizing hardware enforced security based on [Arm TrustZone (Armv8-M)](https://www.arm.com/architecture/learn-the-architecture/m-profile).

To utilize the hardware enforced security, this integration uses PSA Certified
reference implementation [Trusted Firmware-M](https://www.trustedfirmware.org/projects/tf-m/).
Trusted Firmware-M provides various Secure services such as Secure boot, Crypto, Secure Storage,
Attestation and Update services meeting [PSA Certified requirements](https://www.psacertified.org/blog/psa-certified-10-security-goals-explained/).

Developers and partners can use this integration as a starting point to build
FreeRTOS kernel and libraries based software stack on top of Arm Cortex-M based
platforms. All the components are put together in a modular manner to make
porting of this integration across platforms easy.

## Supported Targets

* [Corstone-300](https://developer.arm.com/Processors/Corstone-300)
* [Corstone-310](https://developer.arm.com/Processors/Corstone-310)

## Examples

This reference integration contains following two examples:

* [Blinky example](docs/blinky.md)
    * Demonstrates FreeRTOS kernel and TF-M integration
* [AWS IoT example](docs/aws_iot_example.md)
    * Demonstrates [secure connectivity](#secure-tls-connection) to AWS IoT core using [Mbed TLS](#mbed-tls),
      [PKCS#11 PSA Shim](#pkcs11-psa-shim) and [coreMQTT-agent](https://docs.aws.amazon.com/freertos/latest/userguide/coremqtt-agent.html)
      library. In addition, [secure OTA](#secure-ota-updates) using [OTA agent](https://freertos.org/ota/index.html)
      and [AWS OTA PAL PSA implementation](#aws-ota-pal-psa-implementation).

### Secure TLS Connection

Corstone platform communicates with the AWS IoT Core over a secure TLS
connection. Mbed TLS running on the NSPE is used to establish the TLS
connection. Mbed TLS makes use of the PSA Crypto APIs provided by TF-M for
Crypto operations.

[PKCS#11](https://www.freertos.org/pkcs11/index.html) APIs to perform TLS
client authentication and import TLS client certificate and private key into
the device. PKCS#11 has been integrated with TF-M using a thin shim. In the
integration, the PKCS#11 APIs invoke the appropriate PSA Secure Storage API or
Cryptographic API via the shim. This ensures the keys and certificates are
protected and the cryptographic operations are performed securely within the
SPE of TF-M and is isolated from the kernel, libraries and applications in the
Non-secure Processing Environment. Keys and certificates are securely stored.
This is enabled by TF-M’s Internal Trusted Storage (ITS) and Protected Storage
(PS) services. Signing during TLS client authentication is performed by TF-M’s
Crypto service.

### Secure OTA Updates

FreeRTOS OTA Agent provides an OTA PAL layer for platforms to integrate and
enable OTA updates. The demo integrates and OTA PAL implementation that makes
use of the PSA Certified Firmware Update API implemented in TF-M. This allows
Corstone device to receive new images from AWS IoT Core, authenticate using
TF-M before deploying the image as the active image. The secure (TF-M) and the
non-secure (FreeRTOS kernel and the application) images can be updated
separately.

Every time the device boots, MCUBoot (bootloader) verifies that the image
signature is valid before booting the image. Since the secure (TF-M) and the
non-secure (FreeRTOS kernel and the application) images are singed separately,
MCUBoot verifies that both image signatures are valid before booting. If either
of the verification fails, then MCUBoot stops the booting process.

## Software Components

### Trusted Firmware M

Trusted Firmware-M (TF-M) implements the Secure Processing Environment (SPE)
for Armv8-M, Armv8.1-M architectures (e.g. the Cortex-M33, Cortex-M23,
Cortex-M55, Cortex-M85 processors) and dual-core platforms. It is the platform
security architecture reference implementation aligning with PSA Certified
guidelines, enabling chips, Real Time Operating Systems and devices to become
PSA Certified. Follow the [link](https://tf-m-user-guide.trustedfirmware.org/introduction/readme.html)
for more information on Trusted Firmware M.

### Mbed TLS

Project implements cryptographic primitives, X.509 certificate manipulation and
the SSL/TLS and DTLS protocols. The project provides reference implementation
of [PSA Cryptography API Specification](https://developer.arm.com/documentation/ihi0086/b)
by supporting the cryptographic operations via. PSA Crypto APIs. Follow the
[link](https://www.trustedfirmware.org/projects/mbed-tls/) for more information
on Mbed TLS.

### PKCS11 PSA Shim

[PKCS#11 PSA shim layer](https://github.com/Linaro/freertos-pkcs11-psa.git)
provides a reference implementation of PKCS#11 API based on
[Platform Security Architecture](https://www.arm.com/architecture/psa-certified)
API.

This shim layer maps the PKCS#11 APIs to PSA Cryptography and Storage APIs
V1.0. It follows the same PSA Cryptography API version supported in
[Mbed TLS 3.4.0](https://github.com/Mbed-TLS/mbedtls/tree/mbedtls-3.4.0).
Certificate objects and key objects are protected by PSA secure service.
By default, the device private/public keys are persistent while the code verify
key is volatile.

### AWS OTA PAL PSA implementation

Implementation of [AWS OTA PAL](https://github.com/Linaro/freertos-ota-pal-psa.git)
based on [Platform Security Architecture](https://www.arm.com/architecture/psa-certified)
API.

This implementation maps the AWS OTA PAL APIs to the PSA Firmware Update and
PSA Cryptography APIs. The writing, verification and activation of the update
image are protected by the PSA secure services.

## Contributing

See [CONTRIBUTING](CONTRIBUTING.md) for more information.

## License

Source code located in the *Projects* directory is
available under the terms of the MIT License. See the [LICENSE](./LICENSE) file
for more details.

Other files in the repository are available under the terms specified in each
source file.
