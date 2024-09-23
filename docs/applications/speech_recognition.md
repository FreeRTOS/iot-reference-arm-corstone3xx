# Speech Recognition with OTA support

## Introduction

The Speech Recognition application demonstrates identifying sentences on a voice input by utilizing ML accelerator [Ethos-U55](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u55). The application encompasses [TrustedFirmware-M](https://www.trustedfirmware.org/projects/tf-m/) running on the secure side of the Armv8-M processor, while the ML inference engine (tensorflow-lite) and the model running on the non-secure side of the Armv8-M processor.

When a sentence is inferred, it is printed on the terminal and sent to the configured cloud provider.

The following audio source configurations are supported:
* ROM.
* VSI (Virtual Streaming Interface).

The sixth LED blinks at a regular interval to indicate that the system is alive and waits for input.

## Development environment

The [document](../development_environment/introduction.md)
describes the steps to setup the development environment. Ensure that, it is
setup correctly before proceeding.

## Setting up AWS connectivity

Follow the instructions described in [Setting Up AWS Connectivity](./aws_iot/setting_up_aws_connectivity.md).

## Building the application

To build the Speech-Recognition example, run the following command:

```bash
./tools/scripts/build.sh speech-recognition --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315> --inference ETHOS --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```
* The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's path if you chose the **Auto-generate a new certificate** during the Thing creation. If you chose **Skip creating a certificate at this time** then these paths should locate the generated credential files that were created by the `./tools/scripts/generate_credentials.py` script in the previous step.

* The `audio` is used to select the input audio source whether it's preloaded into `ROM` or using Arm's Virtual Streaming Interface `VSI`.

* The `conn-stack` is used to select the connectivity stack to be used whether it's `FREERTOS_PLUS_TCP` or `IOT_VSOCKET`.

* The `psa-crypto-implementation` is used to select the library providing the PSA Crypto APIs implementation whether it's `TF-M` or `MBEDTLS`. For more information about the PSA Crypto APIs
implementation, please refer to [Mbed TLS document](../components/security/mbedtls/mbedtls.md#psa-crypto-apis-implementation).

Or, run the command below to perform a clean build:

```bash
./tools/scripts/build.sh speech-recognition --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315> --inference ETHOS --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS> -c
```

## Provisioning the device credentials into Protected Storage

Check [Device Provisioning](./device_provisioning/device_provisioning.md) for detailed information.

## Running the application

### Note:
If you would like to run the speech recognition application using VSI configuration as the input audio source, you must run the [setup_python_vsi.sh](../../tools/scripts/setup_python_vsi.sh) script to setup the needed Python environment for VSI prior to running the application:

```bash
./tools/scripts/setup_python_vsi.sh
```

To run the Speech-Recognition example, run the following command:

```bash
./tools/scripts/run.sh speech-recognition --target <corstone300/corstone310/corstone315> --audio <ROM/VSI>
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
5 34 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.10
6 47 [DSP_TASK] [INFO] DSP start
7 52 [BLINK_TASK ] [INFO] Blink task started
8 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
9 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
10 1000 [IP-Task] vDHCP_RATimerReload: 250
11 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
12 1260 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
13 1260 [IP-Task] [INFO] Network is up
14 1260 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
15 1260 [IP-Task] vDHCP_RATimerReload: 8640000
16 1291 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
17 1307 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
18 1322 [IP-Task] ipARP_REPLY from ac1433feip to ac143301ip end-point ac143301ip
19 6333 [MQTT Agent Task] DNS_ReadReply returns -11
20 6340 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
21 6356 [MQTT Agent Task] DNS[0x5946]: The answer to '<iot-core-endpoint>.amazonaws.com' (52.208.184.196) will be stored
22 6375 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
23 6392 [MQTT Agent Task] FreeRTOS_connect: 45227 to 34d0b8c4ip:8883
24 6505 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
25 8791 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
26 8808 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.
27 9243 [MQTT Agent Task] [INFO] MQTT connection established with the broker.
28 9254 [MQTT Agent Task] [INFO] Successfully connected to the MQTT broker.
29 9265 [MQTT Agent Task] [INFO] Session present: 0
30 9272 [MQTT Agent Task] [INFO] Starting a clean MQTT Session.
31 9281 [OTA Task ] [INFO]  Received: 0   Queued: 0   Processed: 0   Dropped: 0
32 9292 [OTA Agent Task] [WARN] Index: 0. OTA event id: 0
33 9300 [OTA Agent Task] [WARN] Index: 1. OTA event id: 2
36 10857 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/jobs/notify-next.
37 10876 [OTA Agent Task] [INFO] Subscribed to MQTT topic: $aws/things/<mqtt-client-identifier>/jobs/notify-next
40 11844 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
41 11859 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
42 11870 [MQTT Agent Task] [INFO] Received incoming publish message Task 0 publishing message 0
44 12634 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/jobs/$next/get.
45 13021 [MQTT Agent Task] [INFO] Ack packet deserialized with result: MQTTSuccess.
46 13032 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
47 13044 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/$next/get to broker.
48 13066 [OTA Agent Task] [WARN] OTA Timer handle NULL for Timerid=0, can't stop.
49 13121 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
50 13135 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
51 13147 [MQTT Agent Task] [INFO] Received job message callback, size 1016.
52 13157 [OTA Agent Task] [WARN] Index: 3. OTA event id: 3
53 13166 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobId: AFR_OTA-ota-test-update-id-6c887a31-0f18-42b0-977c-aae255199d36]
54 13189 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.streamname: AFR_OTA-c35caad0-f43b-4043-a7ca-5c60e27616d0]
55 13211 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.protocols: ["MQTT"]]
56 13228 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filepath: non_secure image]
57 13242 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filesize: 3081945]
58 13254 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[fileid: 0]
59 13265 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[certfile: 0]
60 13277 [OTA Agent Task] [INFO] Extracted parameter [ sig-sha256-rsa: ORLHoYQfNvBa0Z+xLLi2IDSwTdmPRHNQ... ]
61 13293 [OTA Agent Task] [INFO] Job document was accepted. Attempting to begin the update.
62 13306 [OTA Agent Task] [INFO] Job parsing success: OtaJobParseErr_t=OtaJobParseErrNone, Job name=AFR_OTA-ota-test-update-id-6c887a31-0f18-42b0-977c-aae255199d36
63 13328 [OTA Agent Task] [WARN] Received an unhandled callback event from OTA Agent, event = 6
64 13342 [OTA Agent Task] [INFO] Signal task inference start
65 13350 [OTA Agent Task] [INFO] DSP task start
66 13357 [ML_TASK] [INFO] Initial start of audio processing
67 13367 [ML_TASK] [INFO] Ethos-U55 device initialised
68 13374 [DSP_TASK] [INFO] Init speex
69 13381 [ML_TASK] [INFO] Ethos-U version info:
70 13389 [ML_TASK] [INFO] 	Arch:       v1.1.0
71 13397 [ML_TASK] [INFO] 	Driver:     v0.16.0
72 13405 [ML_TASK] [INFO] 	MACs/cc:    256
73 13413 [ML_TASK] [INFO] 	Cmd stream: v0
INFO - Added ethos-u support to op resolver
INFO - Creating allocask] [INFO] Setting OTA data interface74 13443 [OTA Agent Task] [INFO] Setting OTA data interface.
INFO - A [OTA Agent Task] 75 13458 [OTA Agent Task] [INFO] Signal task inference start
INFO - 1 [OTA Agent Task] [IN76 13471 [OTA Agent Task] [INFO] DSP task start
INFO - 	 [OTA Agent Task] [W77 13482 [OTA Agent Task] [WARN] Index: 5. OTA event id: 4
INFO - 	tensor occupies 11544 bytes with dimensions
INFO - 		0:   1
INFO - 		1: 296
INFO - 		2:  39
INFO - Quant dimension: 0
INFO - Scale[0] = 0.171293
INFO - ZeroPoint[0] = 4
INFO - Model OUTPUT tensors:
INFO - 	tensor type is INT8
INFO - 	tensor occupies 4292 bytes with dimensions
INFO - 		0:   1
INFO - 		1:   1
INFO - 		2: 148
INFO - 		3:  29
INFO - Quant dimension: 0
INFO - Scale[0] = 0.495078
INFO - ZeroPoint[0] = 5
INFO - Activation buffer (a.k.a tensor arena) size used: 436092
INFO - Number of operators: 1
INFO - 	Operator 0: ethos-u
78 13628 [ML_TASK] [INFO] *** ML interface initialised
79 14072 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-c35caad0-f43b-4043-a7ca-5c60e27616d0/data/cbor.
80 14099 [OTA Agent Task] [WARN] Index: 7. OTA event id: 5
81 14240 [DSP_TASK] [INFO] ML Processing
82 14246 [ML_TASK] [INFO] Inference 1/2
83 14309 [ML_TASK] [INFO] Start running inference on an audio clip in local memory
[warning ][main@0][3706 ns] TA0 is not enabled!
[warning ][main@0][3706 ns] TA1 is not enabled!
84 14342 [ML_TASK] [INFO] Inference done
85 14490 [DSP_TASK] [INFO] ML Processing
86 14648 [ML_TASK] [INFO] Inference 2/2
87 14768 [ML_TASK] [INFO] Start running inference on an audio clip in local memory
88 14801 [ML_TASK] [INFO] Inference done
89 14807 [ML_TASK] [INFO] Final results:
90 14814 [ML_TASK] [INFO] Total number of inferences: 2
91 14822 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-c35caad0-f43b-4043-a7ca-5c60e27616d0/get/cbor.
92 14848 [ML_TASK] [INFO] For timestamp: 0.100000 (inference #: 0); label: turn down the temperature in the bedroo
93 14865 [ML_TASK] [INFO] For timestamp: 0.100000 (inference #: 1); label: om
94 14877 [ML_TASK] [INFO] Complete recognition: turn down the temperature in the bedroom
95 14890 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-c35caad0-f43b-4043-a7ca-5c60e27616d0/get/cbor to broker.
96 14919 [ML_MQTT] [INFO] Attempting to publish (turn down the temperature in the bedroom) to the MQTT topic <mqtt-client-identifier>/ml/inference.
97 14953 [DSP_TASK] [INFO] ML Processing
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
5 34 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.10
6 47 [DSP_TASK] [INFO] DSP start
7 52 [BLINK_TASK ] [INFO] Blink task started
8 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
9 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
10 1000 [IP-Task] vDHCP_RATimerReload: 250
11 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
12 1260 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
13 1260 [IP-Task] [INFO] Network is up
14 1260 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
15 1260 [IP-Task] vDHCP_RATimerReload: 8640000
16 1291 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
17 1307 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
18 1322 [IP-Task] ipARP_REPLY from ac1433feip to ac143301ip end-point ac143301ip
19 6333 [MQTT Agent Task] DNS_ReadReply returns -11
20 6340 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
21 6356 [MQTT Agent Task] DNS[0x5946]: The answer to '<iot-core-endpoint>.amazonaws.com' (52.208.184.196) will be stored
22 6375 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
23 6392 [MQTT Agent Task] FreeRTOS_connect: 45227 to 34d0b8c4ip:8883
24 6505 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
25 8791 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
26 8808 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.

...

49 13121 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
50 13135 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
51 13147 [MQTT Agent Task] [INFO] Received job message callback, size 1016.
52 13157 [OTA Agent Task] [WARN] Index: 3. OTA event id: 3
53 13166 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobId: AFR_OTA-ota-test-update-id-6c887a31-0f18-42b0-977c-aae255199d36]
54 13189 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.streamname: AFR_OTA-c35caad0-f43b-4043-a7ca-5c60e27616d0]
55 13211 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.protocols: ["MQTT"]]
56 13228 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filepath: non_secure image]
57 13242 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filesize: 3081945]
58 13254 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[fileid: 0]
59 13265 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[certfile: 0]
60 13277 [OTA Agent Task] [INFO] Extracted parameter [ sig-sha256-rsa: ORLHoYQfNvBa0Z+xLLi2IDSwTdmPRHNQ... ]
61 13293 [OTA Agent Task] [INFO] Job document was accepted. Attempting to begin the update.

...

8740 1522485 [MQTT Agent Task] [INFO] Received incoming publish message Task 0 publishing message 255
8742 1522520 [OTA Agent Task] [WARN] Index: 9. OTA event id: 6
8744 1522554 [OTA Agent Task] [INFO] Received final block of the update.
8745 1524214 [OTA Agent Task] [INFO] Received entire update and validated the signature.

...

8752 1525595 [OTA Agent Task] [INFO] Received OtaJobEventActivate callback from OTA Agent.
[INF] Starting bootloader
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
[INF] Boot source: primary slot
[INF] Image index: 1, Swap type: test
[INF] Starting swap using scratch algorithm.
[INF] Primary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x1
[INF] Scratch: magic=bad, swap_type=0x1, copy_done=0x2, image_ok=0x2
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
5 34 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.20
```
