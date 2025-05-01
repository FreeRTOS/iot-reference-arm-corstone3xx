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
./tools/scripts/build.sh keyword-detection --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315/corstone320> --inference <ETHOS/SOFTWARE> --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```
* The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's path if you chose the **Auto-generate a new certificate** during the Thing creation. If you chose **Skip creating a certificate at this time** then these paths should locate the generated credential files that were created by the `./tools/scripts/generate_credentials.py` script in the previous step.

* The `inference` is used to select the inference type whether it's `ETHOS` or `SOFTWARE`.

* The `audio` is used to select the input audio source whether it's preloaded into `ROM` or using Arm's Virtual Streaming Interface `VSI`.

* The `conn-stack` is used to select the connectivity stack to be used whether it's `FREERTOS_PLUS_TCP` or `IOT_VSOCKET`.

* The `psa-crypto-implementation` is used to select the library providing the PSA Crypto APIs implementation whether it's `TF-M` or `MBEDTLS`. For more information about the PSA Crypto APIs
implementation, please refer to [Mbed TLS document](../components/security/mbedtls/mbedtls.md#psa-crypto-apis-implementation).

Or, run the command below to perform a clean build:

```bash
./tools/scripts/build.sh keyword-detection --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315/corstone320> --inference <ETHOS/SOFTWARE> --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS> -c
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
./tools/scripts/run.sh keyword-detection --target <corstone300/corstone310/corstone315/corstone320> --audio <ROM/VSI>
```

### Expected output

```log
[INF] Starting bootloader
[INF] PSA Crypto init done, sig_type: EC-P256, using builtin keys
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
[INF] Image version: v2.2.0
[INF] Jumping to the first image slot
[1;34mBooting TF-M v2.2.0+gdd2b7de[0m
<NUL>[1;34m[Sec Thread] Secure image initializing![0m
<NUL>TF-M Float ABI: Hard
<NUL>Lazy stacking enabled
<NUL>[INF][PS] Encryption alg: 0x5500200
[INF][Crypto] Provision entropy seed...
[INF][Crypto] Provision entropy seed... [0;32mcomplete[0m.
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
2 0 [IP-Task] vIPSetDHCP_RATimerEnableState: Off
3 0 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
4 23 [BLINK_TASK ] [INFO] Blink task started
5 27 [OTA Task] [INFO] OTA over MQTT, firmware versions:
6 33 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.0
7 40 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.20
8 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
9 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
10 1000 [IP-Task] vDHCP_RATimerReload: 250
11 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
12 1257 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
13 1257 [IP-Task] [INFO] Network is up
14 1257 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
15 1257 [IP-Task] vDHCP_RATimerReload: 6480000
16 1281 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
17 1293 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
18 6304 [MQTT Agent Task] DNS_ReadReply returns -11
19 6309 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
20 6317 [MQTT Agent Task] DNS[0xD64C]: The answer to '<iot-core-endpoint>.amazonaws.com' (63.35.107.16) will be stored
21 6331 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
22 6344 [MQTT Agent Task] FreeRTOS_connect: 58661 to 3f236b10ip:8883
23 6415 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
24 8238 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
25 8251 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.
26 8439 [MQTT Agent Task] [INFO] Successfully connected to the MQTT broker.
27 8447 [MQTT Agent Task] [INFO] Session present: 0
28 8453 [MQTT Agent Task] [INFO] Starting a clean MQTT Session.
29 8460 [OTA Agent Task] [INFO] Requesting job document.
30 8468 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/start-next to broker.
31 8626 [MQTT Agent Task] [INFO] Received job message callback, size 1080.
32 8637 [OTA Agent Task] [INFO] Extracted parameter: [jobid: AFR_OTA-ota-test-update-id-f1af3c74-44c9-4c6a-a820-90c0109a3fd2]
33 8650 [OTA Agent Task] [INFO] Extracted parameter: [streamname: AFR_OTA-8ba5c498-5521-4380-a01b-f830ab9d427f]
34 8662 [OTA Agent Task] [INFO] Extracted parameter: [filepath: non_secure image]
35 8670 [OTA Agent Task] [INFO] Extracted parameter: [filesize: 596736]
36 8678 [OTA Agent Task] [INFO] Extracted parameter: [fileid: 0]
37 8685 [OTA Agent Task] [INFO] Extracted parameter: [certfile: 0]
38 8692 [OTA Agent Task] [INFO] Extracted parameter: [sig-sha256-rsa: ipfV1xmxH/WZYKHRPu9PqVet...]
39 8753 [OTA Agent Task] [INFO] Application version of the new image is not higher than the current image: New images are expected to have a higher version number.
40 9386 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-f1af3c74-44c9-4c6a-a820-90c0109a3fd2/update to broker.
41 9409 [OTA Agent Task] [INFO] No OTA job available.
42 9415 [OTA Agent Task] [INFO] Signal task inference start
43 9421 [ML_TASK] [INFO] Initial start of audio processing
INFO - Initialising Ethos-U device@0x40004000
INFO - Ethos-U device initialised
INFO - Ethos-U version info:
INFO - 	Arch:       v1.1.0
INFO - 	Driver:     v0.16.0
INFO - 	MACs/cc:    256
INFO - 	Cmd stream: v0
44 9450 [ML_TASK] [INFO] Ethos-U55 device initialised
45 9455 [ML_TASK] [INFO] Ethos-U version info:
46 9460 [ML_TASK] [INFO] 	Arch:       v1.1.0
47 9465 [ML_TASK] [INFO] 	Driver:     v0.16.0
48 9470 [ML_TASK] [INFO] 	MACs/cc:    256
49 9474 [ML_TASK] [INFO] 	Cmd stream: v0
INFO - Added ethos-u support to op resolver
INFO - Creating allocator using tensor arena at 0x6004c940
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
50 9554 [ML_TASK] [INFO] *** ML interface initialised
51 9569 [ML_TASK] [INFO] Running inference on an audio clip in local memory
[warning ][main@0][4669 ns] TA0 is not enabled!
[warning ][main@0][4669 ns] TA1 is not enabled!
52 9609 [ML_TASK] [INFO] ML_HEARD_ON
53 9613 [ML_MQTT] [INFO] Attempting to publish (on) to the MQTT topic <mqtt-client-identifier>/ml/inference.
54 9628 [ML_TASK] [INFO] For timestamp: 0.000000 (inference #: 0); label: on, score: 0.996127; threshold: 0.700000
55 9657 [ML_TASK] [INFO] For timestamp: 0.500000 (inference #: 1); label: on, score: 0.962542; threshold: 0.700000
56 9686 [ML_TASK] [INFO] ML UNKNOWN
57 9690 [ML_TASK] [INFO] For timestamp: 1.000000 (inference #: 2); label: <none>; threshold: 0.000000
58 9718 [ML_TASK] [INFO] ML_HEARD_OFF
59 9722 [ML_TASK] [INFO] For timestamp: 1.500000 (inference #: 3); label: off, score: 0.999030; threshold: 0.700000
60 9751 [ML_TASK] [INFO] ML UNKNOWN
61 9755 [ML_TASK] [INFO] For timestamp: 2.000000 (inference #: 4); label: <none>; threshold: 0.000000
62 9783 [ML_TASK] [INFO] For timestamp: 2.500000 (inference #: 5); label: <none>; threshold: 0.000000
63 9811 [ML_TASK] [INFO] ML_HEARD_GO
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
[INF] PSA Crypto init done, sig_type: EC-P256, using builtin keys
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Image index: 1, Swap type: none
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Image index: 0, Swap type: none
[INF] Bootloader chainload address offset: 0x0
[INF] Image version: v2.2.0
[INF] Jumping to the first image slot
[1;34mBooting TF-M v2.2.0+gdd2b7de[0m
<NUL>[1;34m[Sec Thread] Secure image initializing![0m
<NUL>TF-M Float ABI: Hard
<NUL>Lazy stacking enabled
<NUL>Creating an empty ITS flash layout.
Creating an empty PS flash layout.
[INF][PS] Encryption alg: 0x5500200
[INF][Crypto] Provision entropy seed...
[INF][Crypto] Provision entropy seed... [0;32mcomplete[0m.
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
5 0 [IP-Task] vIPSetDHCP_RATimerEnableState: Off
6 0 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
7 38 [BLINK_TASK ] [INFO] Blink task started
8 42 [OTA Task] [INFO] OTA over MQTT, firmware versions:
9 48 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.0
10 55 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.10
11 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
12 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
13 1000 [IP-Task] vDHCP_RATimerReload: 250
14 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
15 1257 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
16 1257 [IP-Task] [INFO] Network is up
17 1257 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
18 1257 [IP-Task] vDHCP_RATimerReload: 6480000
19 1281 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
20 1293 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
21 6304 [MQTT Agent Task] DNS_ReadReply returns -11
22 6309 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
23 6316 [MQTT Agent Task] DNS[0xEEF9]: The answer to '<iot-core-endpoint>.amazonaws.com' (54.194.168.245) will be stored
24 6330 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
25 6344 [MQTT Agent Task] FreeRTOS_connect: 55035 to 36c2a8f5ip:8883
26 6412 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
27 8034 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
28 8047 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.

...

34 8435 [MQTT Agent Task] [INFO] Received job message callback, size 1046.
35 8445 [OTA Agent Task] [INFO] Extracted parameter: [jobid: AFR_OTA-ota-test-update-id-f1af3c74-44c9-4c6a-a820-90c0109a3fd2]
36 8459 [OTA Agent Task] [INFO] Extracted parameter: [streamname: AFR_OTA-8ba5c498-5521-4380-a01b-f830ab9d427f]
37 8470 [OTA Agent Task] [INFO] Extracted parameter: [filepath: non_secure image]
38 8479 [OTA Agent Task] [INFO] Extracted parameter: [filesize: 596736]
39 8487 [OTA Agent Task] [INFO] Extracted parameter: [fileid: 0]
40 8493 [OTA Agent Task] [INFO] Extracted parameter: [certfile: 0]
41 8500 [OTA Agent Task] [INFO] Extracted parameter: [sig-sha256-rsa: ipfV1xmxH/WZYKHRPu9PqVet...]
42 9411 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-8ba5c498-5521-4380-a01b-f830ab9d427f/data/json.
43 9456 [OTA Agent Task] [INFO] Received OTA job document.
44 9462 [OTA Agent Task] [INFO] Signal task inference stop
45 9468 [OTA Agent Task] [INFO] Requesting file block.
46 9474 [OTA Agent Task] [INFO] Starting the download.

...

768 218735 [MQTT Agent Task] [INFO] Received data message callback, size 5495.
769 218743 [OTA Agent Task] [INFO] Received file block.
770 218751 [OTA Agent Task] [INFO] Downloaded block 144 of 146.
771 218758 [OTA Agent Task] [INFO] Requesting file block.
772 219495 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-8ba5c498-5521-4380-a01b-f830ab9d427f/get/json to broker.
773 219890 [MQTT Agent Task] [INFO] Received data message callback, size 3787.
774 219898 [OTA Agent Task] [INFO] Received file block.
775 219905 [OTA Agent Task] [INFO] Downloaded block 145 of 146.
776 219912 [OTA Agent Task] [INFO] Closing file.
777 220196 [OTA Agent Task] [INFO] Attempting to activate image.

...

[INF] Starting bootloader
[INF] PSA Crypto init done, sig_type: EC-P256, using builtin keys
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
[INF] Image version: v2.2.0
[INF] Jumping to the first image slot
[1;34mBooting TF-M v2.2.0+gdd2b7de[0m
<NUL>[1;34m[Sec Thread] Secure image initializing![0m
<NUL>TF-M Float ABI: Hard
<NUL>Lazy stacking enabled
<NUL>[INF][PS] Encryption alg: 0x5500200
[INF][Crypto] Provision entropy seed...
[INF][Crypto] Provision entropy seed... [0;32mcomplete[0m.
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
2 0 [IP-Task] vIPSetDHCP_RATimerEnableState: Off
3 0 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
4 23 [BLINK_TASK ] [INFO] Blink task started
5 27 [OTA Task] [INFO] OTA over MQTT, firmware versions:
6 33 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.0
7 40 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.20
```

## Running AWS IoT Core Device Advisor tests
Follow the instructions described in [Running AWS IoT Core Device Advisor tests](./device_advisor/running_aws_iot_core_device_advisor_tests.md).
