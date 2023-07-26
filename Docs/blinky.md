# Blinky example

## Introduction

This blinky example demonstrates the integration of FreeRTOS kernel and
[TrustedFirmware-M](https://www.trustedfirmware.org/projects/tf-m/). The FreeRTOS application running on non-secure side uses
PSA APIs to fetch the firmware framework version of TrustedFirmware-M running
on the secure-side and prints it on the console. In addition, to simulate LED
blinking, `LED On` and `LED off` are printed onto the console at regular
intervals.

## Setting up Arm Virtual Hardware using Amazon Machine Images

Follow the instructions described in [Launch Arm Virtual Hardware Instance](setting-up-arm-virtual-hardware.md)
to setup your development environment.

If you have successfully followed the instructions, then you should have a
console (either AWS-Web-Console or Local-Console) to Arm Virtual Hardware
Instance. From now on, any command-line commands described in this document
must be run on the console connected to Arm Virtual Hardware Instance.

## Setting up development environment

Follow the instructions described in [Setting Up your Development Environment](development-environment.md)
to setup your development environment.

## Building the application

To build the blinky example, run the following command:
```bash
./Tools/scripts/build.sh blinky
```

Run the command below to perform a clean build:
```bash
./Tools/scripts/build.sh blinky -c
```

This will build the example with the Arm Compiler (armclang) by default, which is
included in the [Arm Virtual Hardware instance](./setting-up-arm-virtual-hardware.md)
on AWS. If you would like to build it with the Arm GNU Toolchain (arm-none-eabi-gcc)
[installed by yourself](./development-environment.md), append the extra option
`--toolchain GNU` to the build command above.

## Running the application

To run the blinky example, run the following command:
```bash
./Tools/scripts/run.sh blinky
```

### Expected output

```bash
[INF] Starting bootloader
[INF] Beginning BL2 provisioning
[WRN] TFM_DUMMY_PROVISIONING is not suitable for production! This device is NOT SECURE
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x3
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Swap type: none
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Swap type: none
[INF] Bootloader chainload address offset: 0x0
[INF] Jumping to the first image slot
[INF] Beginning TF-M provisioning
<NUL>[WRN] <NUL>TFM_DUMMY_PROVISIONING is not suitable for production! <NUL>This device is NOT SECURE<NUL>
<NUL>[Sec Thread] Secure image initializing!
<NUL>Booting TF-M v1.8.0
<NUL>Creating an empty ITS flash layout.
Creating an empty PS flash layout.
[INF][Crypto] Provisioning entropy seed... complete.
[DBG][Crypto] Initialising mbed TLS 3.4.0 as PSA Crypto backend library... complete.
psa_framework_version is: 257
LED on
LED off
LED on
LED off
LED on
LED off
```
