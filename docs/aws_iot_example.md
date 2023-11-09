# AWS FreeRTOS MQTT example with OTA support

## Introduction

This AWS FreeRTOS MQTT example demonstrates how to develop cloud connected
applications and update them securely by integrating the modular [FreeRTOS kernel](https://www.freertos.org/RTOS.html)
and [libraries](https://www.freertos.org/libraries/categories.html) and
utilizing hardware enforced security based on [Arm TrustZone (Armv8-M)](https://www.arm.com/technologies/trustzone-for-cortex-m).

## Prerequisites

Follow the instructions described in [Prerequisites](prerequisites.md) and
ensure that all the prerequisites are met before continuing.

## Setting up AWS connectivity

Follow the instructions descrived in [Setting Up AWS Connectivity](./setting_up_aws_connectivity.md).

## Building the application

To build the AWS FreeRTOS MQTT example, run the following command:

```bash
./tools/scripts/build.sh aws-iot-example --certificate_path <certificate pem's path> --private_key_path <private key pem's path>
```
* The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's path if you chose the **Auto-generate a new certificate** during the Thing creation. If you chose **Skip creating a certificate at this time** then these paths should locate the generated credential files that were created by the `./tools/scripts/generate_credentials.py` script in the previous step.

Or, run the command below to perform a clean build:

```bash
./tools/scripts/build.sh aws-iot-example -c
```

This will build the example with the Arm Compiler (armclang) by default, which is
included in the [Arm Virtual Hardware instance](./setting_up_arm_virtual_hardware.md)
on AWS. If you would like to build it with the Arm GNU Toolchain (arm-none-eabi-gcc)
[installed by yourself](./development_environment.md), append the extra option
`--toolchain GNU` to the build command above.


## Provisioning the device credentials into Protected Storage
During the build process a ```provisioning_data.bin``` is built into the ```build/applications/aws_iot_example/provisioning_data``` directory.
This binary contains the device credentials (private key and certificate).

If the content of the .pem files that were used during the build process (passed with ```--certificate_path``` and ```--private_key_path```) changed, then ```cmake --build build -j -- provisioning_data``` rebuilds this provisioning binary.

The binary has to be loaded to the ```0x210FF000``` address so the ```aws-iot-example``` can detect that a provisioning bundle is present and writes the credentials into the Protected Storage. (The run.sh script automatically does this.)

## Running the application

To run the AWS FreeRTOS MQTT example, run the following command:

```bash
./tools/scripts/run.sh aws-iot-example
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
Follow the instructions descrived in the [Observing MQTT connectivity](./aws_iot_cloud_connection.md) section.

## Firmware update with AWS
Follow the instructions descrived in the [Firmware update with AWS](./aws_iot_cloud_connection.md) section.

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
Follow the instructions descrived in [Running AWS IoT Core Device Advisor tests](./running_aws_iot_core_device_advisor_tests.md).
