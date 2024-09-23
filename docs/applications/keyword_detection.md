# Keyword Detection with OTA support

## Introduction

The Keyword Detection application demonstrates identifying keywords on a voice input either by utilizing ML accelerator [Ethos-U55](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u55) or by using software. The application encompasses [TrustedFirmware-M](https://www.trustedfirmware.org/projects/tf-m/) running on the secure side of the Armv8-M processor, while the ML inference engine (tensorflow-lite) and the model running on the non-secure side of the Armv8-M processor.

The following inference configurations are supported:
* ETHOS (uses Ethos-U55).
* SOFTWARE.

The following audio source configurations are supported:
* ROM.
* VSI (Virtual Streaming Interface).

Depending on the keyword recognized, LEDs of the system are turned ON or OFF:

- LED1:
  - _Yes_: on
  - _No_: off
- LED2:
  - _Go_: on
  - _Stop_: off
- LED3:
  - _Up_: on
  - _Down_: off
- LED4:
  - _Left_: on
  - _Right_: off
- LED5:
  - _On_: on
  - _Off_: off

The sixth LED blinks at a regular interval to indicate that the system is alive and waits for input.

## Development environment

The [document](../development_environment/introduction.md)
describes the steps to setup the development environment. Ensure that, it is
setup correctly before proceeding.

## Setting up AWS connectivity

Follow the instructions described in [Setting Up AWS Connectivity](./aws_iot/setting_up_aws_connectivity.md).

## Building the application

To build the Keyword-Detection example, run the following command:

```bash
./tools/scripts/build.sh keyword-detection --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315> --inference <ETHOS/SOFTWARE> --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```
* The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's path if you chose the **Auto-generate a new certificate** during the Thing creation. If you chose **Skip creating a certificate at this time** then these paths should locate the generated credential files that were created by the `./tools/scripts/generate_credentials.py` script in the previous step.

* The `inference` is used to select the inference type whether it's `ETHOS` or `SOFTWARE`.

* The `audio` is used to select the input audio source whether it's preloaded into `ROM` or using Arm's Virtual Streaming Interface `VSI`.

* The `conn-stack` is used to select the connectivity stack to be used whether it's `FREERTOS_PLUS_TCP` or `IOT_VSOCKET`.

* The `psa-crypto-implementation` is used to select the library providing the PSA Crypto APIs implementation whether it's `TF-M` or `MBEDTLS`. For more information about the PSA Crypto APIs
implementation, please refer to [Mbed TLS document](../components/security/mbedtls/mbedtls.md#psa-crypto-apis-implementation).

Or, run the command below to perform a clean build:

```bash
./tools/scripts/build.sh keyword-detection --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315> --inference <ETHOS/SOFTWARE> --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS> -c
```

## Provisioning the device credentials into Protected Storage

Check [Device Provisioning](./device_provisioning/device_provisioning.md) for detailed information.

## Running the application

### Note:
If you would like to run the keyword detection application using VSI configuration as the input audio source, you must run the [setup_python_vsi.sh](../../tools/scripts/setup_python_vsi.sh) script to setup the needed Python environment for VSI prior to running the application:

```bash
./tools/scripts/setup_python_vsi.sh
```

To run the Keyword-Detection example, run the following command:

```bash
./tools/scripts/run.sh keyword-detection --target <corstone300/corstone310/corstone315> --audio <ROM/VSI>
```

### Expected output

```log
[INF] Starting bootloader
[INF] Beginning provisioning
[INF] Waiting for provisioning bundle
[INF] Running provisioning bundle
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Image index: 1, Swap type: none
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Image index: 0, Swap type: none
[INF] Bootloader chainload address offset: 0x0
[INF] Jumping to the first image slot
[1;34m[Sec Thread] Secure image initializing![0m
<NUL>[1;34mBooting TF-M v2.1.0[0m
<NUL>Creating an empty ITS flash layout.
Creating an empty PS flash layout.
[INF][Crypto] Provisioning entropy seed... [0;32mcomplete.[0m
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
5 35 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.10
6 47 [BLINK_TASK ] [INFO] Blink task started
7 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
8 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
9 1000 [IP-Task] vDHCP_RATimerReload: 250
10 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
11 1259 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
12 1260 [IP-Task] [INFO] Network is up
13 1260 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
14 1260 [IP-Task] vDHCP_RATimerReload: 8640000
15 1291 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
16 1307 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
17 1322 [IP-Task] ipARP_REPLY from ac1433feip to ac143301ip end-point ac143301ip
18 6333 [MQTT Agent Task] DNS_ReadReply returns -11
19 6340 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
20 6348 [MQTT Agent Task] DNS[0xA1ED]: The answer to '<iot-core-endpoint>.amazonaws.com' (52.209.194.122) will be stored
21 6367 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
22 6385 [MQTT Agent Task] FreeRTOS_connect: 43065 to 34d1c27aip:8883
23 6424 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
24 8218 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
25 8236 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.
26 8338 [MQTT Agent Task] [INFO] MQTT connection established with the broker.
27 8349 [MQTT Agent Task] [INFO] Successfully connected to the MQTT broker.
28 8360 [MQTT Agent Task] [INFO] Session present: 0
29 8367 [MQTT Agent Task] [INFO] Starting a clean MQTT Session.
30 8376 [OTA Task ] [INFO]  Received: 0   Queued: 0   Processed: 0   Dropped: 0
31 8388 [OTA Agent Task] [WARN] Index: 0. OTA event id: 0
32 8397 [OTA Agent Task] [WARN] Index: 1. OTA event id: 2
35 9279 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/jobs/notify-next.
36 9300 [OTA Agent Task] [INFO] Subscribed to MQTT topic: $aws/things/<mqtt-client-identifier>/jobs/notify-next
39 10105 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
40 10119 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
41 10131 [MQTT Agent Task] [INFO] Received incoming publish message Task 0 publishing message 0
43 10894 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/jobs/$next/get.
44 10986 [MQTT Agent Task] [INFO] Ack packet deserialized with result: MQTTSuccess.
45 10998 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
46 11010 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
47 11025 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
48 11037 [MQTT Agent Task] [INFO] Received job message callback, size 1015.
49 11047 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/$next/get to broker.
50 11070 [OTA Agent Task] [WARN] OTA Timer handle NULL for Timerid=0, can't stop.
51 11082 [OTA Agent Task] [WARN] Index: 3. OTA event id: 3
52 11091 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobId: AFR_OTA-ota-test-update-id-4e88b5e8-9984-4036-ab41-06bfe00f09fa]
53 11115 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.streamname: AFR_OTA-57e0fd88-2666-401d-b2d5-0d11f3d3ab22]
54 11138 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.protocols: ["MQTT"]]
55 11156 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filepath: non_secure image]
56 11171 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filesize: 880417]
57 11184 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[fileid: 0]
58 11196 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[certfile: 0]
59 11208 [OTA Agent Task] [INFO] Extracted parameter [ sig-sha256-rsa: FK3UxyXZT1+Xb0WOHskeiYPJLH/RA/9h... ]
60 11225 [OTA Agent Task] [INFO] Job document was accepted. Attempting to begin the update.
61 11239 [OTA Agent Task] [INFO] Job parsing success: OtaJobParseErr_t=OtaJobParseErrNone, Job name=AFR_OTA-ota-test-update-id-4e88b5e8-9984-4036-ab41-06bfe00f09fa
62 11262 [OTA Agent Task] [WARN] Received an unhandled callback event from OTA Agent, event = 6
63 11276 [ML_TASK] [INFO] Initial start of audio processing
64 11284 [ML_TASK] [INFO] Ethos-U55 device initialised
65 11292 [ML_TASK] [INFO] Ethos-U version info:
66 11298 [ML_TASK] [INFO] 	Arch:       v1.1.0
67 11305 [ML_TASK] [INFO] 	Driver:     v0.16.0
68 11311 [ML_TASK] [INFO] 	MACs/cc:    256
69 11317 [ML_TASK] [INFO] 	Cmd stream: v0
INFO - Added ethos-u support to op resolver
INFO - Creating allocator using tensor arena at 0x6004c900
INFO - Allocating tensors
INFO - Model INPUT tensors:
INFO - 	tensor type is INT8
INFO - 	tensor occupies 490 bytes with dimensions
INFO - 		0:   1
INFO - 		1:  49
INFO - 		2:  10
INFO - 		3:   1
INFO - Quant dimension: 0
INFO - Scale[0] = 0.201095
INFO - ZeroPoint[0] = -5
INFO - Model OUTPUT tensors:
INFO - 	tensor type is INT8
INFO - 	tensor occupies 12 bytes with dimensions
INFO - 		0:   1
INFO - 		1:  12
INFO - Quant dimension: 0
INFO - Scale[0] = 0.056054
INFO - ZeroPoint[0] = -54
INFO - Activation buffer (a.k.a tensor arena) size used: 104508
INFO - Number of operators: 1
INFO - 	Operator 0: ethos-u
70 11423 [ML_TASK] [INFO] *** ML interface initialised
71 11443 [ML_TASK] [INFO] Running inference on an audio clip in local memory
72 11467 [OTA Agent Task] [INFO] Signal task inference start
[warning ][main@0][3706 ns] TA0 is not enabled!
[warning ][main@0][3706 ns] TA1 is not enabled!
73 11490 [ML_TASK] [INFO] For timestamp: 0.000000 (inference #: 0); label: on, score: 0.996127; threshold: 0.700000
74 11507 [ML_MQTT] [INFO] Attempting to publish (on) to the MQTT topic <mqtt-client-identifier>/ml/inference.
75 11528 [BLINK_TASK ] [INFO] ML_HEARD_ON
76 11545 [ML_TASK] [INFO] For timestamp: 0.500000 (inference #: 1); label: on, score: 0.962542; threshold: 0.700000
77 11579 [ML_TASK] [INFO] For timestamp: 1.000000 (inference #: 2); label: <none>; threshold: 0.000000
78 11595 [BLINK_TASK ] [INFO] ML UNKNOWN
79 11612 [ML_TASK] [INFO] For timestamp: 1.500000 (inference #: 3); label: off, score: 0.999030; threshold: 0.700000
80 11628 [BLINK_TASK ] [INFO] ML_HEARD_OFF
81 11636 [OTA Agent Task] [INFO] Setting OTA data interface.
82 11652 [OTA Agent Task] [INFO] Signal task inference start
83 11660 [ML_TASK] [INFO] For timestamp: 2.000000 (inference #: 4); label: <none>; threshold: 0.000000
85 11675 [OTA Agent Task] [WARN] Index: 5. OTA event id: 4
84 11674 [BLINK_TASK ] [INFO] ML UNKNOWN
86 11697 [ML_TASK] [INFO] For timestamp: 2.500000 (inference #: 5); label: <none>; threshold: 0.000000
87 11729 [ML_TASK] [INFO] For timestamp: 3.000000 (inference #: 6); label: go, score: 0.998854; threshold: 0.700000
88 11745 [BLINK_TASK ] [INFO] ML_HEARD_GO
```

## Observing MQTT connectivity

Follow the instructions described in the [Observing MQTT connectivity](./aws_iot/aws_iot_cloud_connection.md) section.

## Firmware update with AWS

Follow the instructions described in the [Firmware update with AWS](./aws_iot/aws_iot_cloud_connection.md) section.

### Expected output

```log
[INF] Starting bootloader
[INF] Beginning provisioning
[INF] Waiting for provisioning bundle
[INF] Running provisioning bundle
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Image index: 1, Swap type: none
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Image index: 0, Swap type: none
[INF] Bootloader chainload address offset: 0x0
[INF] Jumping to the first image slot
[1;34m[Sec Thread] Secure image initializing![0m
<NUL>[1;34mBooting TF-M v2.1.0[0m
<NUL>Creating an empty ITS flash layout.
Creating an empty PS flash layout.
[INF][Crypto] Provisioning entropy seed... [0;32mcomplete.[0m
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
5 35 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.10
6 47 [BLINK_TASK ] [INFO] Blink task started
7 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
8 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
9 1000 [IP-Task] vDHCP_RATimerReload: 250
10 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
11 1259 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
12 1260 [IP-Task] [INFO] Network is up
13 1260 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
14 1260 [IP-Task] vDHCP_RATimerReload: 8640000
15 1291 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
16 1307 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
17 1322 [IP-Task] ipARP_REPLY from ac1433feip to ac143301ip end-point ac143301ip
18 6333 [MQTT Agent Task] DNS_ReadReply returns -11
19 6340 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
20 6348 [MQTT Agent Task] DNS[0xA1ED]: The answer to '<iot-core-endpoint>.amazonaws.com' (52.209.194.122) will be stored
21 6367 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
22 6385 [MQTT Agent Task] FreeRTOS_connect: 43065 to 34d1c27aip:8883
23 6424 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
24 8218 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
25 8236 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.

...

48 11037 [MQTT Agent Task] [INFO] Received job message callback, size 1015.
49 11047 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/$next/get to broker.
50 11070 [OTA Agent Task] [WARN] OTA Timer handle NULL for Timerid=0, can't stop.
51 11082 [OTA Agent Task] [WARN] Index: 3. OTA event id: 3
52 11091 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobId: AFR_OTA-ota-test-update-id-4e88b5e8-9984-4036-ab41-06bfe00f09fa]
53 11115 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.streamname: AFR_OTA-57e0fd88-2666-401d-b2d5-0d11f3d3ab22]
54 11138 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.protocols: ["MQTT"]]
55 11156 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filepath: non_secure image]
56 11171 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filesize: 880417]
57 11184 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[fileid: 0]
58 11196 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[certfile: 0]
59 11208 [OTA Agent Task] [INFO] Extracted parameter [ sig-sha256-rsa: FK3UxyXZT1+Xb0WOHskeiYPJLH/RA/9h... ]
60 11225 [OTA Agent Task] [INFO] Job document was accepted. Attempting to begin the update.

...

2374 398303 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-57e0fd88-2666-401d-b2d5-0d11f3d3ab22/get/cbor.
2375 398369 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-57e0fd88-2666-401d-b2d5-0d11f3d3ab22/get/cbor to broker.
2376 399016 [OTA Task ] [INFO]  Received: 214   Queued: 214   Processed: 214   Dropped: 0
2377 399373 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
2378 399388 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
2379 399400 [MQTT Agent Task] [INFO] Received data message callback, size 3898.
2380 399411 [OTA Agent Task] [WARN] Index: 9. OTA event id: 6
2381 399421 [OTA Agent Task] [INFO] Received final block of the update.
2382 400422 [OTA Agent Task] [INFO] Received entire update and validated the signature.

...

2387 401573 [OTA Agent Task] [INFO] Received OtaJobEventActivate callback from OTA Agent.
[INF] Starting bootloader
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Image index: 1, Swap type: test
[INF] Starting swap using scratch algorithm.
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
[INF] Boot source: primary slot
[INF] Image index: 0, Swap type: none
[INF] Bootloader chainload address offset: 0x0
[INF] Jumping to the first image slot
[1;34m[Sec Thread] Secure image initializing![0m
<NUL>[1;34mBooting TF-M v2.1.0[0m
<NUL>[INF][Crypto] Provisioning entropy seed... [0;32mcomplete.[0m
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
5 35 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.20

...

41 2768 [OTA Agent Task] [INFO] In self test mode.
42 2768 [OTA Agent Task] [INFO] New image has a higher version number than the current image: New image version=0.0.20, Previous image version=0.0.10
43 2769 [OTA Agent Task] [INFO] Image version is valid: Begin testing file: File ID=0
58 4381 [OTA Agent Task] [INFO] Beginning self-test.
59 4382 [OTA Agent Task] [INFO] Received OtaJobEventStartTest callback from OTA Agent.
74 6002 [OTA Agent Task] [INFO] New image validation succeeded in self test mode.
```

## Running AWS IoT Core Device Advisor tests
Follow the instructions described in [Running AWS IoT Core Device Advisor tests](./device_advisor/running_aws_iot_core_device_advisor_tests.md).
