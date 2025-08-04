# Blinky example

## Introduction

This blinky example demonstrates the integration of FreeRTOS kernel and
[TrustedFirmware-M](https://www.trustedfirmware.org/projects/tf-m/). The FreeRTOS application running on non-secure side uses
PSA APIs to fetch the firmware framework version of TrustedFirmware-M running
on the secure-side and prints it on the console. In addition, to simulate LED
blinking, `LED On` and `LED off` are printed onto the console at regular
intervals.

## Development environment

The [document](../development_environment/introduction.md)
describes the steps to setup the development environment. Ensure that, it is
setup correctly before proceeding.

## Building the application

To build the blinky example, run the following command:
```bash
./tools/scripts/build.sh blinky --target <corstone300/corstone310/corstone315/corstone320> --toolchain <ARMCLANG/GNU>
```

Run the command below to perform a clean build:
```bash
./tools/scripts/build.sh blinky --target <corstone300/corstone310/corstone315/corstone320> --toolchain <ARMCLANG/GNU> -c
```

## Running the application

To run the blinky example, run the following command:
```bash
./tools/scripts/run.sh blinky --target <corstone300/corstone310/corstone315/corstone320>
```

### Expected output

```bash
[INF] Starting bootloader
[INF] Beginning provisioning
[INF] Waiting for provisioning bundle
[INF] Running provisioning bundle
[INF] PSA Crypto init done, sig_type: EC-P256, using builtin keys
[INF] Image index: 1, Swap type: none
[INF] Image index: 0, Swap type: none
[INF] Bootloader chainload address offset: 0x0
[INF] Image version: v2.2.1
[INF] Jumping to the first image slot
[1;34mBooting TF-M v2.2.1+gdd2b7de[0m
<NUL>[1;34m[Sec Thread] Secure image initializing![0m
<NUL>TF-M Float ABI: Hard
<NUL>Lazy stacking enabled
<NUL>psa_framework_version is: 257
LED on
LED off
LED on
LED off
LED on
LED off
```
