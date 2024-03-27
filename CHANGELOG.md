# v202403.00 (2024-03-28)

## Highlights

* Added ML reference applications
    * Keyword Detection
    * Speech Recognition
    * Object Detection
* Build and debug reference applications using VS Code on:
    * Linux
        * Ubuntu 20.04
    * Windows 11
    * Mac OS 14.x
* Device provisioning support
  * Create device credentials (certificate and private key) using python script
    and provision them into the device.
* Validated keyword-detection application using AWS Device Advisor tests
* Removed aws-iot-example reference application
* Updated ml-eval-kit to version 23.11
* Updated Mbed TLS to version v3.5.2
* Updated TF-M to version v2.0.0
* Re-structure the project to ease maintenance

## Changes

* ci: Improve CI to support multiple applications and platforms.
* ci: Fix S3 bucket emptying in cleanup
* Re-structure the project to ease maintenance
* uncrustify: Use replace option for correcting findings.
* Use exported tf-m image signing public keys for OTA signature
* tools: Improve `uncrustify` filtering algorithm
* tf-m: Added TF-M v2.0.0 compatibility
* toolchain: Add Arm GNU Toolchain version 13.2 support
* docs: Add project organisation document
* apps: Add Keyword Detection and Speech Recognition examples
* freertos-libs: Use LTS for `aws_iot` components
* freertos-libs: Use LTS for `connectivity` components
* serial: Fix multithread synchronisation
* provisioning: Share files between apps
* ml-apps: Add support for VSI audio source.
* build: Add custom build directory option
* fri: Add minor fixes.
* gnu-compiler: Generate map file
* build: Do not require certificate and key for Blinky
* ml-eval-kit: Require apps to include build CMake module
  ml-eval-kit: Remove default NPU config list patch
  ml-eval-kit: Build individual libraries
* mbedtls: Update to v3.5.2
* Fix nightly integration tests
* ml-eval-kit: Update to version 23.11
* ci: Add improvements to increase reliability and decrease execution time.
* build: Resolve certificate and key paths.
* docs: Update Arm Compiler for Embedded version to 6.21
* patches: Use robust patches function to fix corstone300's hardfault.
* ml-eval-kit: Add configurable NPU and MAC support
* apps: Extract FreeRTOS IoT Libraries Tests app from aws-iot-example
* freertos-libs-tests: Fix closefile-validsignature OTA Pal test.
* apps: Consolidate common CMake configurations
* vht: Remove vht support
* aws-iot-example: Remove example.
  ci: Build ML apps in Github Actions
  keyword-detection: Add AWS Device Advisor validation.
* integration-app: Reduce binary code size with debug experience
* patches: Unify the way of applying patches.
* bsp: Update BSP to CMSIS6
* bsp: Improve async serial driver
* keyword-detection: Add real clock time synchronisation to fix AWS TLS Expired Server Cert test.
* device-provisioning: Add process documentation.
* heap-management: Use C memory management APIs.
* cs300: Modify non-secure RAM space.
* mlek: Reduce build time with reduced use case resources metadata
* ml-apps: Fix building different ML apps subsequently
* lib-tests-app: Fix build issue
* fri: Build and debug FRI using VS Code
* Fix OOB findings

# v202307.00-preview ( July 2023 )

This is the preview release of the repository. This release includes the
following:

* pub-sub-demo: Add a new subscription topic
* aws-iot-example: Improve MQTT reconnection logic
* aws-iot-example: Split OTA MQTT agent task
* aws-iot-example: Add integration tests to example
* fri: Add unity testing framework
* fri: Update MQTT agent and core to support integration tests
* fri: Use release version of integration tests

# v202306.00-alpha ( June 2023 )

The repository contains IoT Reference integration projects using Corstone
platforms.

This is the first alpha release of the repository. This release includes the
following:

* toolchain: Add GNU toolchain support
* docs: Add documentation for blinky and MQTT examples
* ci: Add CI to run OTA test for every merge request
* fri: MQTT example with OTA capability
* fri: Initial version with blinky example based on Corstone-300
