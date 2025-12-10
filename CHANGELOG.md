# v202511.01 (2025-12-10)

## Highlights

* Updated TrustedFirmware-M to version `v2.2.2`

## Changes

* tf-m: Bump to v2.2.2 minor release.
* mbedtls-docs: Replace tls links with mbedtls alternatives.

# v202511.00 (2025-11-19)

## Highlights

* Added ML-Model component update feature to `Keyword-Detection` application
* Updated the FRI to use new modular OTA structure
* Added unit tests for heap management module
* Updated AWS libraries to `202406.04-LTS`
* Updated FreeRTOS-Kernel to version `v11.2.0`
* Updated MLEK to version `v25.05`
* Updated Mbed TLS to version `v3.6.4`
* Updated TrustedFirmware-M to version `v2.2.1`

## Changes

* ml-model: Add ML-Model component update feature to keyword-detection application.
* ci: Enable scheduled blackduck scanning
* components: Remove out-of-tree patches
* ci: Enable gitlab-ci cache
* ota: Update FRI to use new modular OTA structure
* build: Integrate TF-M split-build
* tests: Add FVP version logging
* Update AWS libraries to `202406.01-LTS`
* Modular OTA improvements: Print job metadata and check if new firmware version
  higher than previous version
* ubuntu: Adapt to ubuntu v22.04 docker image changes.
* ota-orchestrator: Add multiple enhancements and fixes.
  mqtt-agent: Increase MQTT Agent keep alive interval.
* freertos-kernel: Bump to v11.2.0 release.
  mbedtls: Bump to v3.6.3 release.
  tf-m: Bump to v2.2.0 release.
* run-script: Fix conflicts with DISPLAY system wide variable.
  mlek: Switch to public GitLab repo
  setuptools: Update to latest version
* toolchains: Add recommended toolchain flags.
* armclang: Fix map file generation.
* armclang-map-file: Only add list option if CMake version greater than 4
  license: Exclude new gitlab tmp folder from license check
* github-ci: Add DCO sign-off enforcement workflow
  docs: Redirect TF-M links to Read the Docs URLs
  docs: Add the missing development environment requirements
* ci: Remove redundant default-jdk installation
  syscalls-stub: Fix TF-M link
* components: Bump MLEK version to 25.05
* components: Bump Mbed TLS version to v3.6.4
* ci-license: Check for current year in the license job
* compiler: Update ARMClang to 6.24.18
* components: Bump TF-M to v2.2.1 release.
* unit-test: Add unit tests for heap management module
* dev-container: Fix incorrect pyproject toml file reference location
* npm: Use internal npm mirror
  docs: Update endpoint instructions to new AWS console UI
  docs: Add the SSL certification misconfiguration symptom
  docs: Document outdated submodules symptom
  docs: Update MQTT connectivity and OTA update instructions
  fvp: Update to latest Corstone platforms FVP release
* components: Update AWS libraries to `202406.04-LTS`

# v202410.00 (2024-10-22)

## Highlights

* Added Corstone-320 platform support
* Enabled PSA crypto APIs to be used in Mbed TLS running on Non-Secure side
* Added createIoTThings.py script for automating the process of setting up OTA updates
* Added unit test framework for testing integration code
* Added unit tests for some of the components
* Updated FreeRTOS-Kernel to version v11.1.0
* Updated Mbed TLS to version v3.6.1
* Updated TrustedFirmware-M to version v2.1.0

## Changes

* unit-test: Add framework to add unit test for integration code
  ci: Add unit tests to quality checks
* mbedtls: Improve integration and simply usage of component within apps
  mbedtls: Add documentation
  tf-m: Update the document by adding missing sections
* ml-model: Separate model image from NS image.
* ci: Split build jobs based on target platform.
* build: Bump FreeRTOS-Plus-TCP commit hash
* build: Bump TF-M version to v2.1.0
* psa-crypto: Enable PSA crypto APIs in Mbed TLS on NS side.
* ci: Add spell checker quality check job.
* integration-tests: Fix nightly job failures
* freertos-kernel: Update to `V11.1.0` release
* speech-recognition: Increase the block time for DSP task.
* integration-tests: Increase keep-alive interval
* release: Add release checklist
* freertos-plus-tcp: Update to latest commit on FreeRTOS-Plus-TCP main branch.
* ci: Add IoT-vSocket stack nightly tests.
* patches: Remove out of tree patches.
* fri: Update manifest to include tpip category
* docs: document createIoTThings.py usage in aws_tools.md.
  tools: Add createIoTThings.py script for setting up OTA updates.
* freerots-pkcs11-psa: Update to latest version
* coremqtt-agent-unit-test: Add tests for mqtt_agent_task.c.
  coremqtt-agent-task: Debug mqtt_agent_task.c
* unit-test: add unit tests for subscription_manager.c.
  coremqtt-agent: fix bug in subscription_manager.c.
* bsp: Add Corstone-320
* build: Change default build toolchain to GNU
* tools: Add command to createIoTThings.py for creating OTA updates.
* devcontainer: Use aws ecr registry instead of docker registry
  devcontainer: Add ccache support
* provisioning: Prevent re-provisioning
* coremqtt-agent-unit-test: Add tests for freertos_command_pool.c.
* aws-iot-unit-test: Add tests for ota_agent_task.c.
  ota-agent-task: Debug unsafe memcpy usages in ota_agent_task.c.
* tools: Add command for automatically cleaning up all AWS entities created by 'createIoTThings.py create-update-simplified'
* tools: All commands in createIoTThings.py now accept JSON config inputs.
* spell-checker: Add missing excluded words.
  formatting: Add CPP files to uncrustify check.
* manifest: Add the `security-risk` attribute
* components: Update Mbed TLS to v3.6.1 release

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
