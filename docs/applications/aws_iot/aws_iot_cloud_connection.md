# AWS IoT cloud connection

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
Corstone devices to receive new images from AWS IoT Core, authenticate using
TF-M before deploying the image as the active image. The secure (TF-M) and the
non-secure (FreeRTOS kernel and the application) images can be updated
separately.

Every time the device boots, MCUBoot (bootloader) verifies that the image
signature is valid before booting the image. Since the secure (TF-M) and the
non-secure (FreeRTOS kernel and the application) images are singed separately,
MCUBoot verifies that both image signatures are valid before booting. If either
of the verification fails, then MCUBoot stops the booting process.

## Observing MQTT connectivity

To see messages being sent by the application:
1. Login to your account and browse to the [AWS IoT console](https://console.aws.amazon.com/iotv2/).
1. In the left navigation panel, choose **Manage**, followed by **All devices**, and then choose **Things**.
1. Select the thing you created, and open the **Activity** tab. This will show
   the application connecting and subscribing to a topic.
1. Click on the **MQTT test client** button. This will open a new page.
1. Click on **Subscribe to a topic**.
1. In the **Subscription topic** field enter the topic name
   `<mqtt-client-identifier>/ml/inference.`
    > `mqtt-client-identifier` value is defined in
      `applications/<application_name>/configs/aws_configs/aws_clientcredential.h` as
      `clientcredentialIOT_THING_NAME`.
1. In the **MQTT payload display** combo box select `Display payloads as
   strings (more accurate)`
1. Click the **Subscribe** button. The messages will be shown below within
   this same page.

## Firmware update with AWS

The application will check for updates from the AWS Cloud.
If an update is available, the application will download the
new firmware, and then apply the new firmware if the version number indicates
the image is newer. To make such a version available you need to prepare the
update binary (this is part of the build process) and create an OTA job on AWS.

### Creating updated firmware

As part of the application build process, an updated firmware image will be
created that will only differ in version number. That is enough to demonstrate
the OTA process using a newly created image.

If you want to add other changes you should copy the original binary elsewhere
before running the build again with your changes as the same build directory
is used for both.  This is to ensure you have the original binary to compare
against any new version you build.

For example, the updated binary is placed in
`build/keyword-detection-update_signed.bin` for the
keyword-detection application. The updated binary is already signed and it is the
file you will need to upload to the Amazon S3 bucket in the next section.

Upon completion of the build and signing process the
<ins>signature string will be echoed to the terminal</ins>. This will be needed
in the next step.

### Creating AWS IoT firmware update job

1. Follow the instructions at:
   [Create an Amazon S3 bucket to store your update](https://docs.aws.amazon.com/freertos/latest/userguide/dg-ota-bucket.html)
   * Use the default options wherever you have a choice.
   * For simplicity, use the same region for the bucket as where your Instance
    is located.
1. Follow the instructions at: [Create an OTA Update service role](https://docs.aws.amazon.com/freertos/latest/userguide/create-service-role.html)
1. Go to AWS IoT web interface and choose **Manage** followed by **Remote actions**, and then **Jobs**
1. Click the create job button and select **Create FreeRTOS OTA update job**
1. Give it a name and click next
1. Select the device to update (the Thing you created in earlier steps)
1. Select `MQTT` transport only
1. Select **Use my custom signed file**
1. Select upload new file and select the signed update binary
   (`build/${APPLICATION_NAME}-update_signed.bin`)
1. Select the S3 bucket you created in step 1. to upload the binary to
1. Paste the signature string that is echoed during the build of the example
   (it is also available in
   `build/update-signature.txt`).
1. Select `SHA-256` and `RSA` algorithms.
1. For **Path name of code signing certificate on device** put in `0`
   (the path is not used)
1. For **Path name of file on device** put in `non_secure image`
1. As the role, select the OTA role you created in step 2.
1. Click next
1. **Job type**, select
   *Your job will complete after deploying to the selected devices/groups
   (snapshot).*
1. Click next, now your update job is ready and running
   * If your application is connected to AWS, the update will begin immediately. If not, next time your application connects to AWS it will perform the update.
