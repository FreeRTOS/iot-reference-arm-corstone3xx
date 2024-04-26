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

## Configuration

You need to specify a list of Trusted Firmware-M configuration options in `ARM_CORSTONE_BSP_TARGET_PLATFORM_TFM_CMAKE_ARGS` CMake variable, for example:

```cmake
set(ARM_CORSTONE_BSP_TARGET_PLATFORM_TFM_CMAKE_ARGS
    -DMCUBOOT_LOG_LEVEL=INFO
)
```

> You must always set `TFM_PLATFORM` according to your target platform. For example, `arm/mps4/corstone315` is the `TFM_PLATFORM` value for Corstone-315 target. The official list of supported platforms is documented in the [TF-M Platforms] webpage.

> Additionally, your application may require more options to be set depending on what features it needs:
> * For all available options, see the [Configuration] documentation page
> * For an example of setting multiple options, check how the `ARM_CORSTONE_BSP_TARGET_PLATFORM_TFM_CMAKE_ARGS` CMake variable is defined in [this `CMakeLists.txt`](../../../../bsp/CMakeLists.txt)

> For our reference platforms, Corstone-315, Corstone-310, and Corstone-300, their `TFM_PLATFORM` respective values are
> `arm/mps4/corstone315`, `arm/mps3/corstone310/fvp`, `arm/mps3/corstone300/fvp`.

## Integration

### Build dependency

Trusted Firmware-M must be built before your application, because your application depends on the NS interface (described in the [Overview](#overview) section) and the BL2 signing scripts, both of which are generated as parts of the Trusted Firmware-M build process. To ensure the order is correct, call `add_dependencies()` in your
`CMakeLists.txt`:

```cmake
add_dependencies(my_application trusted_firmware-m-build)
```

> Replace `my_application` with the actual name of your application executable.

### Linking

You need to link your application against the `tfm-ns-interface` library so that your application can make API calls (such as cryptography, protected storage, etc.) to the secure firmware.

### Image signing

Your non-secure application image must be signed using the signing script from Trusted Firmware-M. In the signed image, the executable binary is prepended with a header area containing information such as the image size, version, checksum, signature, etc. The bootloader uses this information to validate the image during the boot process.

To sign your application image, you can include the CMake module [`SignTfmImage`](../../../../components/security/trusted_firmware-m/integration/cmake/SignTfmImage.cmake) and call the helper function `iot_reference_arm_corstone3xx_tf_m_sign_image()` in your `CMakeLists.txt`:

```cmake
list(APPEND CMAKE_MODULE_PATH ${IOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR}/components/security/trusted_firmware-m/integration/cmake)
include(SignTfmImage)

iot_reference_arm_corstone3xx_tf_m_sign_image(
    my_application
    my_application_signed
    ${MCUBOOT_IMAGE_VERSION_NS}
    TRUE
)
```

This will generate a signed image, `my_application_signed.bin`, in your build directory.

> Replace `my_application` with the actual name of your application executable.
>
> Replace `MCUBOOT_IMAGE_VERSION_NS` with a version of your choice.

You can merge the bootloader, the secure image, the non-secure application,image, secure provisioning bundle binary, non-secure provisioning bundle binary, and DDR binary into a single `.elf` image to ease loading of the code onto the target. To do this:

```cmake
iot_reference_arm_corstone3xx_tf_m_merge_images(
    my_application
    <NS_PROVISIONING_BUNDLE_LOAD_ADDRESS>
    <location_of_non_secure_provisioning_bundle_binary>
    <NS_DDR4_IMAGE_LOAD_ADDRESS>
    <location_of_ddr_binary>
)
```

This will generate a merged image, `my_application_merged.elf`, in your build directory.

> Replace `my_application` with the actual name of your application executable. Also, replace the following:
> * `NS_PROVISIONING_BUNDLE_LOAD_ADDRESS` with the actual non-secure provisioning bundle load address.
> * `location_of_non_secure_provisioning_bundle_binary` with the actual location of non-secure provisioning bundle binary.
> * `NS_DDR4_IMAGE_LOAD_ADDRESS` with the actual DDR image load address.
> * `location_of_ddr_binary` with the actual location of DDR image binary.

### Running

After building your application, you can run it on an FVP. For instance, if you would like to use `FVP_Corstone_SSE-315` for running an application built for `Corstone-315` target:

* Load the merged `.elf` image, after you have called `iot_reference_arm_corstone3xx_tf_m_merge_images()` during [image signing](#image-signing).

    ```bash
    FVP_Corstone_SSE-315 -a <build_directory>/my_application_merged.elf <FVP_OPTIONS> <AVH_AUDIO_OPTIONS>
    ```
> Replace `FVP_OPTIONS` with the options you wish to use with your FVP while running your application. For inspiration, please have a look on the `OPTIONS` shell variable available in the [run script](../../../../tools/scripts/run.sh).
>
> In case your application uses Arm's Virtual Streaming Interface (VSI), replace  `AVH_AUDIO_OPTIONS` with the options you wish to use with your FVP while running your application with VSI feature. For inspiration, please have a look on `AVH_AUDIO_OPTIONS` shell variable available in the [run script](../../../../tools/scripts/run.sh).

## Examples

To see the full context of the information in the sections above, you are advised to take a look at:
* How TF-M is integrated into Arm FreeRTOS Reference Integration project using [this CMakeLists.txt](https://github.com/FreeRTOS/iot-reference-arm-corstone3xx/blob/main/components/security/trusted_firmware-m/integration/CMakeLists.txt) to generate `tfm-ns-interface` library which is linked to the desired application's executable.
* One of our reference applications [CMakeLists.txt](../../../../applications/blinky/CMakeLists.txt)

## Documentation

For more details of how to use Trusted Firmware-M, see its [official documentation].

## Support

If you encounter any issues or have questions regarding the integration of Trusted Firmware M into your IoT project, feel free to reach out to the Arm support community or consult the official documentation for assistance.

[Configuration]: https://tf-m-user-guide.trustedfirmware.org/configuration
[official documentation]: https://tf-m-user-guide.trustedfirmware.org/
[TF-M Platforms]: https://tf-m-user-guide.trustedfirmware.org/platform
[Trusted Firmware-M]: https://www.trustedfirmware.org/projects/tf-m/
