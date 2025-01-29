## Introduction

This document shows how to configure the project to extend
the Over the Air (OTA) firmware update feature to allow an ML-model-only
update.

## Supported toolchain and example

1. Toolchain support is limited to `GNU`.
1. Example application support is limited to `Keyword-Detection`.

## Details

The PSA Firmware Update API already uses the concept of a firmware component,
and the reference implementation integrated into the FRI, the Trusted
Firmware-M, operates on either two-component configuration (Secure and
Non-Secure images), or a single-component config (where the Secure and
Non-Secure images are merged). To enable the ML model OTA update, it was
necessary to update the default configuration to support a 3-component setup,
and to lift any limitations specific to a 2-component implementation.

### Summary of changes

#### ML Model extraction

The ML model was moved from the DDR memory into a separate binary loaded into
flash. This enabled the MCUBoot bootloader to handle the model in the same way
as the other firmware components that are stored in their dedicated flash
partitions. Now, MCUBoot can successfully validate or update the ML model image.

#### Partition resizing

To keep the changes minimal, the ML model partitions were created at the cost of
the Non-Secure partitions. These were re-sized from `0x340000 B` to
`0x240000 B`, and the remaining `0x100000 B` were used for the ML model. As a
result, addresses of the Secure, Non-Secure and Scratch partitions are
unchanged. Also the sizes of the Secure and Scratch partitions remain unchanged.

#### Runtime copy

Since the Ethos NPU doesn't have access flash, the model is copied to DDR
at runtime (during the ML task init). This is why the model is still kept in the
DDR memory region in the linker script.

#### OTA PAL version handling & file path

The OTA PAL (precisely, the `OtaPalInterface_t` interface) had to be extended
with `getPlatformImageVersion` to enable independent version handling for each
component in a multi-component setup.

#### Image processing

Also the build environment required a number of updates to correctly process the
additional ML model image. Provisioning data and signing layout were configured
for the ML model image based on the Non-Secure image config. Currently, the ML
model image is signed with the same key as the Non-Secure image (i.e. `image_ns_signing_private_key.pem`),
but this can be easily changed by adding a new input parameter to the
[SignTfmImage CMake module](../../../components/security/trusted_firmware-m/integration/cmake/SignTfmImage.cmake)
to accept the designated signing key for each image.

## Build instructions

Currently, the ML model update is supported for the Keyword-Detection example
application built with the `GNU` toolchain for all FRI's supported platforms.
Please follow [the application-specific build instructions](../keyword_detection.md).

## Run instructions

Please follow [the application-specific run instructions](../keyword_detection.md).

If you prefer to run the FVP manually, and explicitly set all the arguments
(e.g. when debugging), run the following command:

```bash
FVP_Corstone_SSE-300_Ethos-U55 \
-C mps3_board.visualisation.disable-visualisation=1 \
-C core_clk.mul=200000000 \
-C mps3_board.smsc_91c111.enabled=1 \
-C mps3_board.hostbridge.userNetworking=1 \
-C mps3_board.telnetterminal0.start_telnet=0 \
-C mps3_board.uart0.out_file=- \
-C mps3_board.uart0.unbuffered_output=1 \
-C mps3_board.DISABLE_GATING=1 \
-a /workspaces/iot-reference-arm-corstone3xx/build/iot_reference_arm_corstone3xx/components/security/trusted_firmware-m/integration/trusted_firmware-m-build-prefix/src/trusted_firmware-m-build-build/bin/bl2.axf \
--data build/iot_reference_arm_corstone3xx/components/security/trusted_firmware-m/integration/trusted_firmware-m-build-prefix/src/trusted_firmware-m-build-build/api_ns/bin/encrypted_provisioning_bundle.bin@0x10022000 \
--data build/keyword-detection_signed.bin@0x28040000 \
--data build/iot_reference_arm_corstone3xx/components/security/trusted_firmware-m/integration/trusted_firmware-m-build-prefix/src/trusted_firmware-m-build-build/api_ns/bin/tfm_s_signed.bin@0x38000000 \
--data build/helpers/provisioning/provisioning_data.bin@0x211ff000 \
--data build/application_sectors/ddr.bin@0x60100000 \
--data build/keyword-detection-model_signed.bin@0x28280000
```

### ML Model update with AWS

As for the usual Non-Secure OTA update demo, the updated ML model firmware image
is created during the application build process. The updated image will only
differ in version number. That is enough to demonstrate the OTA process using a
newly created image.

Deploy an AWS update job as described in the [Firmware update
with AWS](../aws_iot/aws_iot_cloud_connection.md) section. The
flow for the ML model is very similar to the one for the Non-Secure image; the
only differences are:

- use `ml_model image` for **Path name of file on device**,
- upload the signed update binary, `build/keyword-detection-model-update_signed.bin`
- use the signature string from `build/model-update-signature.txt`.

Now, start the Keyword-Detection example, and observe the ML model update.

#### Making the ML model update demo more appealing

Although it is enough to observe the ML model component version bump, a more
demonstrative output can be obtained with minimal effort. Follow the (optional)
steps below to run the Keyword-Detection example with a modified faulty model, unable
to detect any keyword, deploy the OTA update of the ML model, after which correct
ML inference results can be observed after the update is complete.

1. Save the update image with a correctly working ML model.

    By default, the keyword-detection example is built with a fully-functional
    ML model, fetched from the [ML-zoo][kws-model]. If you have already built
    the application, the signed model update is available in the build
    directory. Back it up together with its signature string.

    ```bash
    cp build/keyword-detection-model-update_signed.bin build/model-update-signature.txt applications/keyword_detection/ml-model-update-demo
    ```

1. Alter the ML model artifacts in the build dir.

    A modified model is available in
    `../../../applications/keyword_detection/ml-model-update-demo/faulty_kws_micronet_m.tflite`. Compile it with
    Vela and replace the original tflite file.

    ```bash
    source build/mlek_resources_downloaded/env/bin/activate && vela applications/keyword_detection/ml-model-update-demo/faulty_kws_micronet_m.tflite --accelerator-config=ethos-u55-128 --optimise Performance --config components/ai/ml_embedded_evaluation_kit/library/scripts/vela/default_vela.ini --memory-mode=Shared_Sram --system-config=Ethos_U55_High_End_Embedded --output-dir=applications/keyword_detection/ml-model-update-demo --arena-cache-size=2097152
    cp applications/keyword_detection/ml-model-update-demo/faulty_kws_micronet_m_vela.tflite build/mlek_resources_downloaded/kws/kws_micronet_m_vela_H128.tflite
    ```

1. Build the application with the modified model.

    Simply run the build command mentioned in the [Build
    instructions](#build-instructions) section.

1. Run the Keyword-Detection application and confirm that no keywords are
detected. Then stop the application.

1. Deploy an AWS OTA job with the functional ML model from the
`../../../applications/keyword_detection/ml-model-update-demo` dir.

1. Start the Keyword-Detection example again, let it update the ML model, and
detect keywords correctly again.


Before the ML Model update:

```
(...)
58 10031 [ML_TASK] [INFO] Running inference on an audio clip in local memory
59 10058 [OTA Agent Task] [INFO] Current State=[WaitingForJob], Event=[ReceivedJobDocument], New state=[CreatingFile]
60 10103 [ML_TASK] [INFO] ML UNKNOWN
61 10109 [ML_MQTT] [INFO] Attempting to publish (_unknown_) to the MQTT topic MyThing_eu_central_1/ml/inference.
62 10128 [ML_TASK] [INFO] For timestamp: 0.000000 (inference #: 0); label: <none>; threshold: 0.000000
63 10168 [ML_TASK] [INFO] For timestamp: 0.500000 (inference #: 1); label: <none>; threshold: 0.000000
64 10208 [ML_TASK] [INFO] For timestamp: 1.000000 (inference #: 2); label: <none>; threshold: 0.000000
65 10248 [ML_TASK] [INFO] For timestamp: 1.500000 (inference #: 3); label: <none>; threshold: 0.000000
66 10288 [ML_TASK] [INFO] For timestamp: 2.000000 (inference #: 4); label: <none>; threshold: 0.000000
67 10328 [ML_TASK] [INFO] For timestamp: 2.500000 (inference #: 5); label: <none>; threshold: 0.000000
68 10368 [ML_TASK] [INFO] For timestamp: 3.000000 (inference #: 6); label: <none>; threshold: 0.000000
69 10408 [ML_TASK] [INFO] For timestamp: 3.500000 (inference #: 7); label: <none>; threshold: 0.000000
```

After the ML Model update:

```
(...)
57 10000 [OTA Agent Task] [INFO] In self test mode.
58 10009 [OTA Agent Task] [INFO] New image has a higher version number than the current image: New image version=0.0.42, Previous image version=0.0.11
59 10034 [OTA Agent Task] [INFO] Image version is valid: Begin testing file: File ID=0
(...)
75 12259 [OTA Agent Task] [INFO] New image validation succeeded in self test mode.
(...)
95 14005 [ML_TASK] [INFO] Running inference on an audio clip in local memory
96 14032 [OTA Agent Task] [INFO] Current State=[WaitingForJob], Event=[ReceivedJobDocument], New state=[CreatingFile]
97 14078 [ML_TASK] [INFO] ML_HEARD_ON
98 14084 [ML_MQTT] [INFO] Attempting to publish (on) to the MQTT topic MyThing_eu_central_1/ml/inference.
99 14102 [ML_TASK] [INFO] For timestamp: 0.000000 (inference #: 0); label: on, score: 0.996127; threshold: 0.700000
100 14144 [ML_TASK] [INFO] For timestamp: 0.500000 (inference #: 1); label: on, score: 0.962542; threshold: 0.700000
101 14186 [ML_TASK] [INFO] ML UNKNOWN
102 14192 [ML_TASK] [INFO] For timestamp: 1.000000 (inference #: 2); label: <none>; threshold: 0.000000
103 14232 [ML_TASK] [INFO] ML_HEARD_OFF
104 14239 [ML_TASK] [INFO] For timestamp: 1.500000 (inference #: 3); label: off, score: 0.999030; threshold: 0.700000
105 14281 [ML_TASK] [INFO] ML UNKNOWN
106 14287 [ML_TASK] [INFO] For timestamp: 2.000000 (inference #: 4); label: <none>; threshold: 0.000000
107 14328 [ML_TASK] [INFO] For timestamp: 2.500000 (inference #: 5); label: <none>; threshold: 0.000000
108 14368 [ML_TASK] [INFO] ML_HEARD_GO
109 14375 [ML_TASK] [INFO] For timestamp: 3.000000 (inference #: 6); label: go, score: 0.998854; threshold: 0.700000
110 14417 [ML_TASK] [INFO] ML UNKNOWN
111 14423 [ML_TASK] [INFO] For timestamp: 3.500000 (inference #: 7); label: <none>; threshold: 0.000000
```

[kws-model]: https://github.com/ARM-software/ML-zoo/tree/9f506fe52b39df545f0e6c5ff9223f671bc5ae00/models/keyword_spotting/micronet_medium/tflite_int8
