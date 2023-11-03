# AWS FreeRTOS MQTT example with OTA support

## Introduction

This AWS FreeRTOS MQTT example demonstrates how to develop cloud connected
applications and update them securely by integrating the modular [FreeRTOS kernel](https://www.freertos.org/RTOS.html)
and [libraries](https://www.freertos.org/libraries/categories.html) and
utilizing hardware enforced security based on [Arm TrustZone (Armv8-M)](https://www.arm.com/technologies/trustzone-for-cortex-m).

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

## Prerequisites

Follow the instructions described in [Prerequisites](prerequisites.md) and
ensure that all the prerequisites are met before continuing.

## Setting up AWS connectivity

To connect to the AWS cloud service you will need to setup an IoT Thing and
then set the AWS credentials of the IoT Thing within the Application.
You will need to create an [AWS Account](https://aws.amazon.com/premiumsupport/knowledge-center/create-and-activate-aws-account/)
if you don’t already have one.

### AWS account IoT setup

The instructions below will allow the application to send messages to the cloud
via MQTT as well as enable an over-the-air update.

  > Note: Due to AWS restrictions, you must ensure that when logging into the
    [AWS IoT console](https://console.aws.amazon.com/iotv2/) you are using
    the same **Region** as where you created your AMI instance.  This
    restriction is documented within the [MQTT Topic](https://docs.aws.amazon.com/iot/latest/developerguide/topics.html)
    page in the AWS documentation.

### Creating an IoT thing for your device

1. Login to your account and browse to the [AWS IoT console](https://console.aws.amazon.com/iotv2/).
   * If this takes you to AWS Console, click on **Services** above and then
     click on **IoT Core**.
   * Ensure your **Region** is correct.
1. In the left navigation pane **Manage** section, expand **All devices** then
   select **Things**.
   * These instructions assume you do not have any IoT things registered in
     your account.
   * Press the **Create things** button.
1. On the **Create things** page, select **Create single thing** and press the **Next** button.
1. On the **Specify thing properties** page, type a **Thing name** for your
   thing (for example `MyThing_eu_west_1`), and then press the **Next** button.
   * Adding the region name to your thing name helps to remind you which region
     the thing and topic is attached to.
   * You will need to add the thing name later to your C code.
   * There is no need to add any **Additional configuration**
1. If you want to use your own self-signed client certificate then on the **Configure device certificate** page, choose **Skip creating a certificate at this time** and then press the **Next** button. If you want to use auto-generated certificate then choose **Auto-generate a new certificate** and then press the **Next** button.
1. Skip the **Attach policies to certificate** page for now.
   * You will attach a certificate to the thing in a later step.
1. If you use auto-generated certificates then download all the keys and certificates by choosing the **Download**
   links for each.
   * Click on all the **Download** buttons and store these files in a secure
     location as you will use them later.
   * Make note of the certificate ID. You need it later to attach a policy to
     your certificate.
   * Click on **Done** once all items have downloaded.

   If you use self-signed client certificate then you will upload the certificate that is generated in the next step.


### Generating and registering your own device certificate
AWS IoT Core authenticates device connections with the help of X.509 certificates. The steps below describes how to generate self-signed device certificate and then register it with AWS IoT Core.

1. Run the ```./Tools/scripts/generate_credentials.py``` Python script, that's going to generate a private key
   and a certificate that's signed  with this key.
   * Optionally you can specify metadata for the certificate. Use the ```-h``` flag for the python script to see the available options.
  ```bash
  python ./Tools/scripts/generate_credentials.py --certificate_valid_time <validity duration in days > \
                                                --certificate_country_name <Country Name (2 letter code)> \
                                                --certificate_state_province_name <State or Province Name (full name)> \
                                                --certificate_locality_name <Locality Name (eg, city)> \
                                                --certificate_org_name <Organization Name (eg, company)> \
                                                --certificate_org_unit_name <Organizational Unit Name (eg, section)> \
                                                --certificate_email_address_name <Email Address> \
                                                --certificate_out_path <output path> \
                                                --private_key_out_path <output path>
  ```
1. In the left navigation panel **Manager** section of the AWS IoT console,
   expand **Security**, and then select **Certificates**.
1. On the **Certificates** page, press the **Add certificates** button and select **Register certificates**.
1. Select the **CA is not registered with AWS IoT** option and upload the **certificate.pem** that's generated in the previous step.
1. Select the checkbox next to the uploaded certificate and then click on the **Activate** button.
1. Click on the **Register** button.
   * At this point, the certificate is registered and activated. In the **Security** > **Certificates** menu, you can see the new certificate.
1. Go to the  **Security** > **Certificates** menu and select the newly registered certificate.
1. On the **Things** tab click on the **Attach to things** button and select your Thing.


### Creating a policy and attach it to your certificate

1. In the left navigation pane **Manage** section of the AWS IoT console,
   expand **Security**, and then select **Policies**.
1. On the **Policies** page, press the **Create Policy** button.
   * These instructions assume you do not have any **Policies** registered
     in your account,
1. On the **Create Policy** page
   * Enter a name for the policy in the **Policy properties** section.
   * In the **Policy document** section, add 3 new policy statements to have a
     total of 4 policy statements with the following values:
     * **Policy effect** - select **Allow** for all statements.
     * **Policy Action** - select one of the actions below for each statement.
       * **iot:Connect**
       * **iot:Publish**
       * **iot:Subscribe**
       * **iot:Receive**
   * The **Policy resource** field requires an **ARN**. Sometimes this box will
     be auto-filled with your credentials.
     * If no value exists, use the following format:    (arn:aws:iot:**region:account-id:\***)
       * region (e.g. eu-west-1)
       * account-id ... This is your **Account ID Number**.
         * You can usually see this in the drop down on the top right corner
           where your login name is shown.
       * e.g. *arn:aws:iot:eu-west-1:012345678901:*
     * Replace the part, or add, after the last colon (`:`) with `*`.
       * e.g. *arn:aws:iot:eu-west-1:012345678901:\**
     * Press the **Create** button.

   > NOTE - The examples in this document are intended for development
     environments only.  All devices in your production fleet must have
     credentials with privileges that authorize only intended actions on
     specific resources. The specific permission policies can vary for your use
     case. Identify the permission policies that best meet your business and
     security requirements.  For more information, refer to [Security Best
     practices in AWS IoT Core](https://docs.aws.amazon.com/iot/latest/developerguide/security-best-practices.html).

1. In the left navigation pane **manage** section of the AWS IoT console,
   expand **Security**, and then select **Certificates**. You should see the
   certificate for the thing you created earlier.
   * Use the ID in front of the certificate and key files that you downloaded
     after creating the thing to identify your certificate.
   * Select your certificate name to take you to your Certificate page.
1. Expand the **Actions** drop down list and select **Attach policy**.
   Alternatively, press the **Attach policies** button in **Policies** tab.
1. In the **Attach policies to the certificate** dialog box
   * Choose the policy you created earlier.
     * Even though you may enable more than one policy, for now we use the
       single policy you created earlier.
   * Press the **Attach policies** button.
   * You will now see your policy listed in the **Policies** tab on the
     Certificate page.

### Configuring the application to connect to your AWS account
Now that you have created an AWS Thing and attached the certificates and
policies to it, the representative values must be added to your application to
ensure connectivity with your AWS account.

Within the application directory that you are using, edit the
`configs/aws_configs/aws_clientcredential.h` file and set values for specified
user defines called out below.

`clientcredentialMQTT_BROKER_ENDPOINT`

* Set this to the Device data endpoint name of your amazon account.
* To find this go to the navigation pane of the [AWS IoT console](https://console.aws.amazon.com/iotv2/),
  choose **Settings** (bottom left hand corner).
* On the **Settings** page, in the **Device data endpoint** section of the page
  look for **Endpoint**.  (e.g. `a3xyzzyx-ats.iot.us-east-2.amazonaws.com`).
  * Note the region may be different than these instructions.  It should match
    where your thing and policy were created due to the MQTT Topic restrictions
    discussed above.

`clientcredentialIOT_THING_NAME`

* Set this to the name of the thing you set (e.g. MyThing).

Save and close the file.

The device certificate PEM and private key PEM will be set during the build configuration.

## Building the application

To build the AWS FreeRTOS MQTT example, run the following command:

```bash
./Tools/scripts/build.sh aws-iot-example --certificate_path <certificate pem's path> --private_key_path <private key pem's path>
```
* The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's path if you chose the **Auto-generate a new certificate** during the Thing creation. If you chose **Skip creating a certificate at this time** then these paths should locate the generated credential files that were created by the `./Tools/scripts/generate_credentials.py` script in the previous step.

Or, run the command below to perform a clean build:

```bash
./Tools/scripts/build.sh aws-iot-example -c
```

This will build the example with the Arm Compiler (armclang) by default, which is
included in the [Arm Virtual Hardware instance](./setting_up_arm_virtual_hardware.md)
on AWS. If you would like to build it with the Arm GNU Toolchain (arm-none-eabi-gcc)
[installed by yourself](./development_environment.md), append the extra option
`--toolchain GNU` to the build command above.


## Provisioning the device credentials into Protected Storage
During the build process a ```provisioning_data.bin``` is built into the ```build/applications/aws-iot-example/provisioning_data``` directory.
This binary contains the device credentials (private key and certificate).

If the content of the .pem files that were used during the build process (passed with ```--certificate_path``` and ```--private_key_path```) changed, then ```cmake --build build -j -- provisioning_data``` rebuilds this provisioning binary.

The binary has to be loaded to the ```0x210FF000``` address so the ```aws-iot-example``` can detect that a provisioning bundle is present and writes the credentials into the Protected Storage. (The run.sh script automatically does this.)

## Running the application

To run the AWS FreeRTOS MQTT example, run the following command:

```bash
./Tools/scripts/run.sh aws-iot-example
```

### Expected output

```console
[INF] Starting bootloader
[INF] Beginning BL2 provisioning
[WRN] TFM_DUMMY_PROVISIONING is not suitable for production! This device is NOT SECURE
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
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
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.10
5 0 [OTA Task ] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
6 30 [OTA Task ] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
7 74 [OTA Task ] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
8 1677 [OTA Task ] [INFO] Creating an MQTT connection to the broker.
9 1768 [OTA Task ] [INFO] Packet received. ReceivedBytes=2.
10 1768 [OTA Task ] [INFO] CONNACK session present bit not set.
11 1768 [OTA Task ] [INFO] Connection accepted.
12 1768 [OTA Task ] [INFO] Received MQTT CONNACK successfully from broker.
13 1768 [OTA Task ] [INFO] MQTT connection established with the broker.
14 1768 [OTA Task ] [INFO] Session present: 0
15 1768 [OTA Task ] [INFO]  Received: 0   Queued: 0   Processed: 0   Dropped: 0
16 1768 [OTA Agent Task] [WARN] Index: 0. OTA event id: 0
17 1769 [OTA Agent Task] [WARN] Index: 1. OTA event id: 2
18 1849 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=3.
19 1849 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/jobs/notify-next.
20 1849 [OTA Agent Task] [INFO] Subscribed to MQTT topic: $aws/things/<mqtt-client-identifier>/jobs/notify-next
21 2599 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/jobs/$next/get.
22 2649 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=2.
23 2649 [MQTT Agent Task] [INFO] Ack packet deserialized with result: MQTTSuccess.
24 2649 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
25 2649 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=134.
27 2650 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
26 2649 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/$next/get to broker.
28 2650 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
29 2650 [OTA Agent Task] [WARN] OTA Timer handle NULL for Timerid=0, can't stop.
30 2650 [MQTT Agent Task] [INFO] Received job message callback, size 69.
31 2650 [OTA Agent Task] [WARN] Index: 3. OTA event id: 3
32 2652 [OTA Agent Task] [WARN] Failed to parse JSON document as AFR_OTA job: DocParseErr_t=16
33 2652 [OTA Agent Task] [INFO] No active job available in received job document: OtaJobParseErr_t=OtaJobParseErrNoActiveJobs
34 2652 [OTA Agent Task] [WARN] Received an unhandled callback event from OTA Agent, event = 8
35 2652 [OTA Agent Task] [INFO] Emtpy job document found
36 2653 [OTA Agent Task] [INFO] Empty job docuemnt found for event=[ReceivedJobDocument]
37 3490 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=3.
38 3490 [MQTT PUB SUB] [INFO] Subscribed to topic pubsub/<mqtt-client-identifier>/task_0.
39 3490 [MQTT PUB SUB] [INFO] Successfully subscribed to topic: pubsub/<mqtt-client-identifier>/task_0
40 4240 [MQTT Agent Task] [INFO] Publishing message to pubsub/<mqtt-client-identifier>/task_0.
41 4240 [MQTT PUB SUB] [INFO] Sent PUBLISH packet to broker pubsub/<mqtt-client-identifier>/task_0 to broker.
42 4240 [MQTT PUB SUB] [INFO] Successfully sent QoS 0 publish to topic: pubsub/<mqtt-client-identifier>/task_0 (PassCount:1, FailCount:0).
43 4290 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=70.
44 4290 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
45 4290 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
46 4290 [MQTT Agent Task] [INFO] Received incoming publish message Task 0 publishing message 0
47 6790 [MQTT Agent Task] [INFO] Publishing message to pubsub/<mqtt-client-identifier>/task_0.
48 6790 [MQTT PUB SUB] [INFO] Sent PUBLISH packet to broker pubsub/<mqtt-client-identifier>/task_0 to broker.
49 6790 [MQTT PUB SUB] [INFO] Successfully sent QoS 0 publish to topic: pubsub/<mqtt-client-identifier>/task_0 (PassCount:2, FailCount:0).
50 6830 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=70.
51 6830 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
52 6830 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
53 6830 [MQTT Agent Task] [INFO] Received incoming publish message Task 0 publishing message 1
```

## Observing MQTT connectivity

To see messages being sent by the application:
1. Login to your account and browse to the [AWS IoT console](https://console.aws.amazon.com/iotv2/).
1. In the left navigation panel, choose **Manage**, and then choose **Things**.
1. Select the thing you created, and open the **Activity** tab. This will show
   the application connecting and subscribing to a topic.
1. Click on the **MQTT test client** button. This will open a new page.
1. Click on **Subscribe to a topic**.
1. In the **Subscription topic** field enter the topic name
   `pubsub/<mqtt-client-identifier>/task_0`
    > `mqtt-client-identifier` value is defined in
      `configs/aws_configs/aws_clientcredential.h` as
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
`build/applications/aws-iot-example/aws-iot-example-update_signed.bin` for the
aws-iot-example application. The updated binary is already signed and it is the
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
1. Follow the instructions at: [Create an OTA user policy](https://docs.aws.amazon.com/freertos/latest/userguide/create-ota-user-policy.html)
1. Go to AWS IoT web interface and choose **Manage** and then **Jobs**
1. Click the create job button and select **Create FreeRTOS OTA update job**
1. Give it a name and click next
1. Select the device to update (the Thing you created in earlier steps)
1. Select `MQTT` transport only
1. Select **Use my custom signed file**
1. Select upload new file and select the signed update binary
   (`build/applications/aws-iot-example/aws-iot-example-update_signed.bin`)
1. Select the S3 bucket you created in step 1. to upload the binary to
1. Paste the signature string that is echoed during the build of the example
   (it is also available in
   `build/applications/aws-iot-example/update-signature.txt`).
1. Select `SHA-256` and `RSA` algorithms.
1. For **Path name of code signing certificate on device** put in `0`
   (the path is not used)
1. For **Path name of file on device** put in `non_secure image`
1. As the role, select the OTA role you created in step 2.
1. Click next
1. Create an ID for you Job
1. Add a description
1. **Job type**, select
   *Your job will complete after deploying to the selected devices/groups
   (snapshot).*
1. Click next, your update job is ready and running - next time your
   application connects it will perform the update.

### Expected output

```bash
[INF] Starting bootloader
[INF] Beginning BL2 provisioning
[WRN] TFM_DUMMY_PROVISIONING is not suitable for production! This device is NOT SECURE
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
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
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.10
5 0 [OTA Task ] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
6 30 [OTA Task ] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
7 74 [OTA Task ] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
8 1677 [OTA Task ] [INFO] Creating an MQTT connection to the broker.

...

27 2688 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=942.
28 2689 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
29 2689 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
30 2689 [MQTT Agent Task] [INFO] Received job message callback, size 847.
31 2689 [OTA Agent Task] [WARN] Index: 3. OTA event id: 3
32 2692 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobId: AFR_OTA-ota-test-update-id-17f83b9c-c7df-4267-a102-ebb39b0e7fca]
33 2702 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.streamname: AFR_OTA-d38ac95e-ad29-4565-b082-745995474046]
34 2705 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.protocols: ["MQTT"]]
35 2708 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filepath: non_secure image]
36 2709 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filesize: 309929]
37 2710 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[fileid: 0]
38 2711 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[certfile: 0]
39 2716 [OTA Agent Task] [INFO] Extracted parameter [ sig-sha256-rsa: ITDCoxOIvzRNG/sEP2JytlRVQ6qOnoa2... ]
40 2718 [OTA Agent Task] [INFO] Job document was accepted. Attempting to begin the update.

...

60 6240 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=4224.
63 6240 [MQTT Agent Task] [INFO] Received data message callback, size 4120.

...

873 93950 [MQTT Agent Task] [INFO] Packet received. ReceivedBytes=2122.
876 93951 [MQTT Agent Task] [INFO] Received data message callback, size 2018.
878 93952 [OTA Agent Task] [INFO] Received final block of the update.
879 94275 [OTA Agent Task] [INFO] Received entire update and validated the signature.

...

895 95572 [OTA Agent Task] [INFO] Received OtaJobEventActivate callback from OTA Agent.
[INF] Starting bootloader
[WRN] This device was provisioned with dummy keys. This device is NOT SECURE
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Swap type: test
[INF] Starting swap using scratch algorithm.
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
[INF] Boot source: primary slot
[INF] Swap type: none
[INF] Bootloader chainload address offset: 0x0
[INF] Jumping to the first image slot
[WRN] <NUL>This device was provisioned with dummy keys. <NUL>This device is NOT SECURE<NUL>
<NUL>[Sec Thread] Secure image initializing!
<NUL>Booting TF-M v1.8.0
<NUL>[INF][Crypto] Provisioning entropy seed... complete.
[DBG][Crypto] Initialising mbed TLS 3.4.0 as PSA Crypto backend library... complete.
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.20

...

41 2768 [OTA Agent Task] [INFO] In self test mode.
42 2768 [OTA Agent Task] [INFO] New image has a higher version number than the current image: New image version=0.0.20, Previous image version=0.0.10
43 2769 [OTA Agent Task] [INFO] Image version is valid: Begin testing file: File ID=0
58 4381 [OTA Agent Task] [INFO] Beginning self-test.
59 4382 [OTA Agent Task] [INFO] Received OtaJobEventStartTest callback from OTA Agent.
74 6002 [OTA Agent Task] [INFO] New image validation succeeded in self test mode.
```

## Running AWS IoT Core Device Advisor tests

Device Advisor is a cloud-based, fully managed test capability for validating
IoT devices during device software development. Follow the [link](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor.html)
to learn more about the device advisor.

### Creating an IoT thing

Follow the instructions described in the sections listed below to create an IoT
thing for your device and attaching a policy to it.

* [IoT thing](#creating-an-iot-thing-for-your-device)
* [IoT policy](#creating-a-policy-and-attach-it-to-your-certificate)

### Creating roles and policies

Follow the instructions described in the sections listed below in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-setting-up.html):

* Create an IAM role to use as your device role
* Create a custom-managed policy for an IAM user to use Device Advisor
* Create an IAM user to use Device Advisor (AWS recommendation)

### Creating AWS IoT Core Qualification test suite

Follow the instructions described the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-create-suite)
to create AWS IoT Core Qualification test suite.

### Configuring the application to connect to AWS IoT Core Device Advisor

Now that you have created an AWS Thing and attached the certificates and
policies to it, the representative values must be added to your application to
ensure connectivity with AWS IoT Core Device Advisor.

Set the macro `appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE` in
`configs/app_config/app_config.h` to 1.

Within the application directory that you are using, edit the
`configs/aws_configs/aws_clientcredential.h` file and set values for specified
user defines called out below.

`clientcredentialMQTT_BROKER_ENDPOINT`

* Set this to the device advisor endpoint. You can get the device advisor end
  point either via AWS CLI or AWS IoT console:
  * AWS CLI
    * To get the device advisor end point via [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-getting-started.html),
      run:
        ```bash
        aws iotdeviceadvisor get-endpoint
        ```
  * AWS IoT console
    * To get the device advisor end point via AWS IoT console, follow the
      instructions in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/da-console-guide.html).
      The device advisor end point is shown at the step 12.

`clientcredentialIOT_THING_NAME`

* Set this to the name of the thing you set (e.g. MyThing).

Save and close the file.


Next insert the keys that are in the certificates you have downloaded when you
created the thing. Edit the file
`configs/aws_configs/aws_clientcredential_keys.h` replacing the existing keys
with yours.

`keyCLIENT_CERTIFICATE_PEM`

* Replace with contents from
  `<your-thing-certificate-unique-string>-certificate.pem.crt`.

`keyCLIENT_PRIVATE_KEY_PEM`

* Replace with contents from
  `<your-thing-certificate-unique-string>-private.pem.key`.

### Building the application

To build the application, run the following command:

```bash
./Tools/scripts/build.sh aws-iot-example
```

Or, run the command below to perform a clean build:

```bash
./Tools/scripts/build.sh aws-iot-example -c
```

This will build the example with the Arm Compiler (armclang) by default, which is
included in the [Arm Virtual Hardware instance](./setting_up_arm_virtual_hardware.md)
on AWS. If you would like to build it with the Arm GNU Toolchain (arm-none-eabi-gcc)
[installed by yourself](./development_environment.md), append the extra option
`--toolchain GNU` to the build command above.

### Running the application

We need to start the device advisor tests before running the application. If
not then, device advisor rejects the connection requests from the application.

Follow the instructions described in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-run-test-suite)
to start the Creating AWS IoT Core Qualification test suite created in [section](#creating-aws-iot-core-qualification-test-suite).

Now run the application by running the following command:

```bash
./Tools/scripts/run.sh aws-iot-example
```

Once the device advisor has completed all the tests, you can download the AWS
IoT Core Qualification report by following the instructions in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-qualification-report).
