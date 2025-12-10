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

> ⚠️ **If you’ve built a different application, run the following commands before proceeding**
```bash
git submodule deinit --all -f
git submodule update --init --recursive
```

To build the Speech-Recognition example, run the following command:

```bash
./tools/scripts/build.sh speech-recognition --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315/corstone320> --inference ETHOS --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```
* The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's path if you chose the **Auto-generate a new certificate** during the Thing creation. If you chose **Skip creating a certificate at this time** then these paths should locate the generated credential files that were created by the `./tools/scripts/generate_credentials.py` script in the previous step.

* The `audio` is used to select the input audio source whether it's preloaded into `ROM` or using Arm's Virtual Streaming Interface `VSI`.

* The `conn-stack` is used to select the connectivity stack to be used whether it's `FREERTOS_PLUS_TCP` or `IOT_VSOCKET`.

* The `psa-crypto-implementation` is used to select the library providing the PSA Crypto APIs implementation whether it's `TF-M` or `MBEDTLS`. For more information about the PSA Crypto APIs
implementation, please refer to [Mbed TLS document](../components/security/mbedtls/mbedtls.md#psa-crypto-apis-implementation).

Or, run the command below to perform a clean build:

```bash
./tools/scripts/build.sh speech-recognition --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315/corstone320> --inference ETHOS --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS> -c
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
./tools/scripts/run.sh speech-recognition --target <corstone300/corstone310/corstone315/corstone320> --audio <ROM/VSI>
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
[INF] Image version: v2.2.2
[INF] Jumping to the first image slot
[1;34mBooting TF-M v2.2.2+gdd2b7de[0m
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
4 23 [OTA Task] [INFO] OTA over MQTT, firmware versions:
5 29 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.2
6 35 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.20
7 42 [ML_TASK] [INFO] ML Task start
8 46 [DSP_TASK] [INFO] DSP Task start
9 50 [BLINK_TASK ] [INFO] Blink task started
10 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
11 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
12 1000 [IP-Task] vDHCP_RATimerReload: 250
13 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
14 1257 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
15 1257 [IP-Task] [INFO] Network is up
16 1257 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
17 1257 [IP-Task] vDHCP_RATimerReload: 6480000
18 1281 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
19 1293 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
20 6304 [MQTT Agent Task] DNS_ReadReply returns -11
21 6309 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
22 6317 [MQTT Agent Task] DNS[0xAFCC]: The answer to '<iot-core-endpoint>.amazonaws.com' (34.255.26.134) will be stored
23 6330 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
24 6344 [MQTT Agent Task] FreeRTOS_connect: 55808 to 22ff1a86ip:8883
25 6417 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
26 8242 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
27 8256 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.
28 8444 [MQTT Agent Task] [INFO] Successfully connected to the MQTT broker.
29 8452 [MQTT Agent Task] [INFO] Session present: 0
30 8458 [MQTT Agent Task] [INFO] Starting a clean MQTT Session.
31 8464 [OTA Agent Task] [INFO] Requesting job document.
32 8472 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/start-next to broker.
33 8624 [MQTT Agent Task] [INFO] Received job message callback, size 1080.
34 8634 [OTA Agent Task] [INFO] Extracted parameter: [jobid: AFR_OTA-ota-test-update-id-ca10de26-7243-4061-bba3-c4e850c328eb]
35 8647 [OTA Agent Task] [INFO] Extracted parameter: [streamname: AFR_OTA-1f553cde-b6fa-4882-8c27-b4212683a29e]
36 8659 [OTA Agent Task] [INFO] Extracted parameter: [filepath: non_secure image]
37 8668 [OTA Agent Task] [INFO] Extracted parameter: [filesize: 592447]
38 8675 [OTA Agent Task] [INFO] Extracted parameter: [fileid: 0]
39 8682 [OTA Agent Task] [INFO] Extracted parameter: [certfile: 0]
40 8689 [OTA Agent Task] [INFO] Extracted parameter: [sig-sha256-rsa: XmTvGHbk5N6UqdU1a0ZxOyxWP...]
41 8750 [OTA Agent Task] [INFO] Application version of the new image is not higher than the current image: New images are expected to have a higher version number.
42 9384 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-ca10de26-7243-4061-bba3-c4e850c328eb/update to broker.
43 9407 [OTA Agent Task] [INFO] No OTA job available.
44 9413 [OTA Agent Task] [INFO] Signal task inference start
45 9419 [OTA Agent Task] [INFO] DSP task start
46 9425 [ML_TASK] [INFO] Initial start of audio processing
INFO -2 [DSP_TASK] [INFO] Initial start of aud47 9432 [DSP_TASK] [INFO] Initial start of audio processing
INFO - E[DSP_TASK] [INFO] Init spe48 9443 [DSP_TASK] [INFO] Init speex
INFO - Ethos-U version info:
INFO - 	Arch:       v1.1.0
INFO - 	Driver:     v0.16.0
INFO - 	MACs/cc:    256
INFO - 	Cmd stream: v0
49 9466 [ML_TASK] [INFO] Ethos-U55 device initialised
50 9472 [ML_TASK] [INFO] Ethos-U version info:
51 9478 [ML_TASK] [INFO] 	Arch:       v1.1.0
52 9483 [ML_TASK] [INFO] 	Driver:     v0.16.0
53 9488 [ML_TASK] [INFO] 	MACs/cc:    256
54 9493 [ML_TASK] [INFO] 	Cmd stream: v0
INFO - Added ethos-u support to op resolver
INFO - Creating allocator using tensor arena at 0x602633b0
INFO - Allocating tensors
INFO - Model INPUT tensors:
INFO - 	tensor type is INT8
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
55 9582 [ML_TASK] [INFO] *** ML interface initialised
56 10138 [DSP_TASK] [INFO] ML Processing
57 10143 [ML_TASK] [INFO] Inference 1/2
58 10201 [ML_TASK] [INFO] Start running inference on an audio clip in local memory
[warning ][main@0][4655 ns] TA0 is not enabled!
[warning ][main@0][4655 ns] TA1 is not enabled!
59 10226 [ML_TASK] [INFO] Inference done
60 10383 [DSP_TASK] [INFO] ML Processing
61 10530 [ML_TASK] [INFO] Inference 2/2
62 10643 [ML_TASK] [INFO] Start running inference on an audio clip in local memory
63 10669 [ML_TASK] [INFO] Inference done
64 10674 [ML_TASK] [INFO] Final results:
65 10679 [ML_TASK] [INFO] Total number of inferences: 2
66 10685 [ML_TASK] [INFO] For timestamp: 0.100000 (inference #: 0); label: turn down the temperature in the bedroo
67 10698 [ML_TASK] [INFO] For timestamp: 0.100000 (inference #: 1); label: om
68 10707 [ML_TASK] [INFO] Complete recognition: turn down the temperature in the bedroom
69 10718 [ML_MQTT] [INFO] Attempting to publish (turn down the temperature in the bedroom) to the MQTT topic <mqtt-client-identifier>/ml/inference.
70 10762 [DSP_TASK] [INFO] ML Processing
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
[INF] Image version: v2.2.2
[INF] Jumping to the first image slot
[1;34mBooting TF-M v2.2.2+gdd2b7de[0m
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
7 38 [OTA Task] [INFO] OTA over MQTT, firmware versions:
8 44 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.2
9 50 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.10
10 57 [ML_TASK] [INFO] ML Task start
11 61 [DSP_TASK] [INFO] DSP Task start
12 66 [BLINK_TASK ] [INFO] Blink task started
13 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
14 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
15 1000 [IP-Task] vDHCP_RATimerReload: 250
16 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
17 1257 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
18 1257 [IP-Task] [INFO] Network is up
19 1257 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
20 1257 [IP-Task] vDHCP_RATimerReload: 6480000
21 1281 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
22 1293 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
23 6304 [MQTT Agent Task] DNS_ReadReply returns -11
24 6309 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
25 6317 [MQTT Agent Task] DNS[0xBDBD]: The answer to '<iot-core-endpoint>.amazonaws.com' (54.77.52.39) will be stored
26 6330 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
27 6344 [MQTT Agent Task] FreeRTOS_connect: 13805 to 364d3427ip:8883
28 6413 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
29 8035 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
30 8049 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.

...

36 8436 [MQTT Agent Task] [INFO] Received job message callback, size 1046.
37 8447 [OTA Agent Task] [INFO] Extracted parameter: [jobid: AFR_OTA-ota-test-update-id-ca10de26-7243-4061-bba3-c4e850c328eb]
38 8460 [OTA Agent Task] [INFO] Extracted parameter: [streamname: AFR_OTA-1f553cde-b6fa-4882-8c27-b4212683a29e]
39 8472 [OTA Agent Task] [INFO] Extracted parameter: [filepath: non_secure image]
40 8481 [OTA Agent Task] [INFO] Extracted parameter: [filesize: 592447]
41 8488 [OTA Agent Task] [INFO] Extracted parameter: [fileid: 0]
42 8495 [OTA Agent Task] [INFO] Extracted parameter: [certfile: 0]
43 8502 [OTA Agent Task] [INFO] Extracted parameter: [sig-sha256-rsa: XmTvGHbk5N6UqdU1a0ZxOyxWP...]
44 9398 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-1f553cde-b6fa-4882-8c27-b4212683a29e/data/json.
45 9444 [OTA Agent Task] [INFO] Received OTA job document.
46 9450 [OTA Agent Task] [INFO] Signal task inference stop
47 9456 [OTA Agent Task] [INFO] DSP task stop
48 9461 [OTA Agent Task] [INFO] Requesting file block.
49 9467 [OTA Agent Task] [INFO] Starting the download.

...

766 217715 [MQTT Agent Task] [INFO] Received data message callback, size 5495.
767 217723 [OTA Agent Task] [INFO] Received file block.
768 217731 [OTA Agent Task] [INFO] Downloaded block 143 of 145.
769 217738 [OTA Agent Task] [INFO] Requesting file block.
770 218475 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-1f553cde-b6fa-4882-8c27-b4212683a29e/get/json to broker.
771 218853 [MQTT Agent Task] [INFO] Received data message callback, size 3531.
772 218861 [OTA Agent Task] [INFO] Received file block.
773 218868 [OTA Agent Task] [INFO] Downloaded block 144 of 145.
774 218875 [OTA Agent Task] [INFO] Closing file.
775 219155 [OTA Agent Task] [INFO] Attempting to activate image.

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
[INF] Image version: v2.2.2
[INF] Jumping to the first image slot
[1;34mBooting TF-M v2.2.2+gdd2b7de[0m
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
4 23 [OTA Task] [INFO] OTA over MQTT, firmware versions:
5 29 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.2
6 35 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.20
```
