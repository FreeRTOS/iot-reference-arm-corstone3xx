# Project Organisation

## Naming conventions
* Files and subdirectories are consistently named using `snake_case` (except for those files expected to have different cases such as `README.md`, `CONTRIBUTING.md`, `CMakeLists.txt`, etc), with underscores replacing spaces.
When names contain underscores, such as 'Trusted Firmware-M', the resultant subdirectory retains them, like `trusted_firmware-m`.
* CMake targets created by this project follow a consistent naming convention using `kebab-case`.
* CMake modules follow the typical naming convention of `VerbWithCamelCase`.

## Root directory and files organisation

```sh
├── applications/
├── bsp/
├── components/
├── docs/
├── release_changes/
├── tools/
├── CHANGELOG.md
├── CMakeLists.txt
├── CODE_OF_CONDUCT.md
├── CONTRIBUTING.md
├── cspell.config.yaml
├── DCO.txt
├── LICENSE
├── manifest.yml
├── NOTICE.txt
├── pyproject.toml
├── README.md
└── setup.cfg
```
*Figure 1: Root directory and files organisation*

* `applications`: Contains all application specific files including helper libraries and components configuration files.
* `bsp`:  Contains the integrations for the component that provide hardware platform supports. It is designed to be interchangeable and easily modifiable by silicon partners if they choose to utilize ARM FRI as a foundation for rapidly creating their own solution.
* `components`: Contains external libraries fetched with Git Submodules along with other libraries that are developed as part of FRI.
* `docs`: Design documents, guides and other documentation specific applications.
* `release_changes`: Automatically generated directory housing changelogs for each merged pull request.
* `tools`: Comprises all the tools developed by this project for accomplishing various tasks.
* `CHANGELOG.md`: An automatically generated changelog created using external tools and scripts within `tools` subdirectory.
* `CMakeLists.txt`: Determines the minimum CMake version, the project name (which matches the repository name), augments the CMake module paths to include CMake modules in the tools  subdirectory, and adds the bsp  and components subdirectories to the iot-reference-arm-corstone3xx CMake project.
* `CODE_OF_CONDUCT.md`: Code of conduct for the project.
* `CONTRIBUTING.md`: Guidelines for contributing.
* `cspell.config.yaml`: Code spell checker configuration file.
* `DCO.txt`: Developer Certificate of Origin
* `LICENSE`: Type of open-source license for the iot-reference-arm-corstone3xx project is under.
* `manifest.yml`: Record of all external project dependencies fetched by this project. 
* `NOTICE.txt`: Project notice.
* `pyproject.toml`: Configuration files for the build requirement of Python projects used with the iot-reference-arm-corstone3xx project.
* `README.md`: Project README including an additional section for subdirectories content.
* `setup.cfg`: Configuration file for packaging Python scripts created by the iot-reference-arm-corstone3xx project.

### Components subdirectory organisation

The components subdirectory contains external libraries along with other libraries that are developed as part of FRI.


```sh
components/
├── aws_iot/
│   ├── corejson/
│   ├── coremqtt/
│   ├── coremqtt_agent/
│   ├── corepkcs11/
│   ├── coresntp/
│   ├── ota_for_aws_iot_embedded_sdk/
│   └── tinycbor/
├── connectivity/
│   ├── backoff_algorithm/
│   ├── freertos_plus_tcp/
│   ├── iot_socket/
│   └── iot_vsocket/
├── freertos_kernel/
├── security/
│   ├── freertos_ota_pal_psa/
│   ├── freertos_pkcs11_psa/
│   ├── mbedtls/
│   └── trusted_firmware-m/
└── tools/
    ├── freertos_libraries_integration_tests/
    ├── open_iot_sdk_toolchain/
    └── unity/
```
*Figure 2: Libraries grouping per category within the components subdirectory.*

* `aws`: Includes the integrations for components used to make a connection to AWS IoT Core.
* `connectivity`: Includes the integrations for components used to establish networking.
* `freertos_kernel`: Includes the FreeRTOS kernel.
* `security`: Includes integrations that enhance security.
* `tools`: Includes integrations for components providing tools used by applications.

Each component comprise two parts:

* `library` - the external library source code.
* `integration` - additional integration code for the library to build applications.

If the component requires additional work before its code can be either consumed using CMake or requires addition source code for integration then an integration subdirectory is added.

The integration subdirectory contains distinct directories:

* `src`  - contains integration source code files
* `inc`  - contains integration header files
* `patches`  - contains patch files to be applied to the library

```sh
components/aws_iot/coremqtt_agent/
├── CMakeLists.txt
├── integration/
│   ├── CMakeLists.txt
│   ├── inc/
│   │   ├── freertos_agent_message.h
│   │   ├── freertos_command_pool.h
│   │   ├── mqtt_agent_task.h
│   │   └── subscription_manager.h
│   ├── patches/
│   │    └── 0001-This-patch-is-to-be-applied-to-the-library.patch
│   └── src/
│       ├── freertos_agent_message.c
│       ├── freertos_command_pool.c
│       ├── mqtt_agent_task.c
│       └── subscription_manager.c
└── library/
```
*Figure 3: Example of component organisation*


## CMake targets creation

A modular approach is used to create the components.

There is a subdirectory for each component and each contains a CMakeLists.txt that creates a CMake CACHE INTERNAL variable pointing to the path to the library code and adds the integration library (if additional integration is needed for the component).

💡 This file is also where patches to the library are to be applied from.

```cmake
set(coremqtt_agent_SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/library
    CACHE INTERNAL
    "Path to coreMQTT-Agent source code"
)

include(ApplyPatches)

set(PATCH_FILES_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/integration/patches")
set(PATCH_FILES
    "${PATCH_FILES_DIRECTORY}/0001-This-patch-is-to-be-applied-to-the-library.patch"
    "${PATCH_FILES_DIRECTORY}/0002-This-patch-is-to-be-applied-to-the-library.patch"
    "${PATCH_FILES_DIRECTORY}/000n-This-patch-is-to-be-applied-to-the-library.patch"
)
iot_reference_arm_corstone3xx_apply_patches("${coremqtt_agent_SOURCE_DIR}" "${PATCH_FILES}")

add_subdirectory(integration)
```
*Figure 4: Component main CMakeLists.txt example*


Two CMake targets are created for each component integration:

* library target (if one is not already provided).
* library configuration target for applications to customize the target for
  their use cases.
```cmake
include(${coremqtt_agent_SOURCE_DIR}/mqttAgentFilePaths.cmake)

add_library(coremqtt-agent
    ${MQTT_AGENT_SOURCES}
    src/mqtt_agent_task.c
    src/subscription_manager.c
    src/freertos_command_pool.c
    src/freertos_agent_message.c
)

target_include_directories(coremqtt-agent
    PUBLIC
        ${MQTT_AGENT_INCLUDE_PUBLIC_DIRS}
        inc
)

add_library(coremqtt-agent-config INTERFACE)

target_link_libraries(coremqtt-agent
    PUBLIC
        coremqtt-agent-config
    PRIVATE
        backoff-algorithm
        connectivity-stack
        coremqtt
        freertos_kernel
        helpers-events
        helpers-logging
)
```
*Figure 5: Component integration CMakeLists.txt example*

## Applications subdirectory organisation

Applications are located within their respective subdirectories within the top level applications subdirectory.

Anything specific to an application should live in its respective subdirectory (component configuration files, cloud credential files, etc).

Each applications has its own configs subdirectory as it is expected that the configurations will differ from an application to another.

```sh
applications/keyword_detection/
├── configs
│   ├── app_config
│   │   ├── app_config.h
│   │   └── CMakeLists.txt
│   ├── aws_configs
│   │   ├── aws_clientcredential.h
│   │   ├── ...
│   │   └── ota_demo_config.h
│   ├── CMakeLists.txt
│   ├── freertos_config
│   │   ├── CMakeLists.txt
│   │   ├── FreeRTOSConfig.h
│   │   └── FreeRTOSIPConfig.h
│   ├── mbedtls_config
│   │   ├── aws_mbedtls_config.h
│   │   ├── CMakeLists.txt
│   │   └── threading_alt.h
│   └── tfm_config
│       └── project_config.h
├── resources
│   ├── test.wav
└── tests
    ├── fail_output.log
    └── pass_output.log
├── blink_task.c
├── blink_task.h
├── CMakeLists.txt
├── main.c
├── ml_interface.cc
├── ml_interface.h
├── model_config.cc
```
*Figure 6: Content of keyword-detection application*

Each application executable link only with the component target library it needs:
```cmake
target_link_libraries(keyword-detection
    PRIVATE
        backoff-algorithm
        connectivity-stack
        coremqtt
        coremqtt-agent
        corepkcs11
        freertos_kernel
        freertos-ota-pal-psa
        fri-bsp
        helpers-device-advisor
        helpers-events
        kws_api
        kws_model
        mbedtls
        mbedtls-threading-freertos
        ota-for-aws-iot-embedded-sdk
        provisioning-lib
        tfm-ns-interface
        toolchain-override
)

# sntp helper library depends on FreeRTOS-Plus-TCP connectivity stack as it
# includes `FreeRTOS_IP.h` header file in one of its source files (sntp_client_task.c),
# thus this library is only added in case of using FREERTOS_PLUS_TCP connectivity stack.
if(CONNECTIVITY_STACK STREQUAL "FREERTOS_PLUS_TCP")
    target_link_libraries(keyword-detection
        PRIVATE
            coresntp
            helpers-sntp
    )
endif()
```
*Figure 7: keyword-detection application linked library alphabetically ordered*


### Application helper libraries

It is expected that applications share some common code, these are organized as libraries and made available in a helpers subdirectory within the root applications subdirectory so they can be used by any application.

```sh
applications/helpers/
├── CMakeLists.txt
├── events/
├── logging/
├── mqtt/
└── provisioning/
```
*Figure 8: Application helper libraries organisation*
