# Trusted Firmware-M

## Overview

[Trusted Firmware-M] (TF-M) implements the Secure Processing Environment (SPE) for Armv8-M, Armv8.1-M architectures (e.g.
the Cortex-M33, Cortex-M23, Cortex-M55, Cortex-M85 processors) or dual-core platforms. It is the platform security
architecture reference implementation aligning with PSA Certified guidelines, enabling chips, Real Time Operating
Systems and devices to become PSA Certified.

A software stack running Trusted Firmware-M consists of three executable binaries:

* the bootloader, known as BL2
* the secure firmware, which is the Trusted Firmware
* the non-secure firmware, which is the user application

The non-secure firmware can make API calls (such as cryptography, protected storage, etc.) to the secure firmware via a
library known as the NS (non-secure) interface.
