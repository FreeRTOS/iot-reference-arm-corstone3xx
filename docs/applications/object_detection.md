# Object Detection with ISP and OTA support

## Introduction

The Object Detection application demonstrates face detection on a `.frm` input frames file by utilizing ML accelerator [Ethos-U65](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u65) and Versatile Image Signal Processor for Computer Vision [Mali-C55 ISP](https://www.arm.com/products/silicon-ip-multimedia/image-signal-processor/mali-c55). The application encompasses [TrustedFirmware-M](https://www.trustedfirmware.org/projects/tf-m/) running on the secure side, while the ML inference engine (tensorflow-lite) and the model running on the non-secure side of the Armv8-M processor.

### The following inference configurations are supported:
- ETHOS (uses Ethos-U65)
- ISP (uses Mali-C55)

### The following frame configurations are supported:
- FRM (From File)

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

To build the Object-Detection example, run the following command:
```bash
./tools/scripts/build.sh object-detection --certificate_path <certificate pem's path> --private_key_path <private key pem's path> -t <corstone315/corstone320> --toolchain GNU --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```
- The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's path if you chose the **Auto-generate a new certificate** during the Thing creation. If you chose **Skip creating a certificate at this time** then these paths should locate the generated credential files that were created by the `./tools/scripts/generate_credentials.py` script in the previous step.

- The `toolchain` is used to select the `GNU`, that supports the `Mali-C55`.

* The `conn-stack` is used to select the connectivity stack to be used whether it's `FREERTOS_PLUS_TCP` or `IOT_VSOCKET`.

* The `psa-crypto-implementation` is used to select the library providing the PSA Crypto APIs implementation whether it's `TF-M` or `MBEDTLS`. For more information about the PSA Crypto APIs
implementation, please refer to [Mbed TLS document](../components/security/mbedtls/mbedtls.md#psa-crypto-apis-implementation).

Or, run the command below to perform a clean build:
```bash
./tools/scripts/build.sh object-detection --certificate_path <certificate pem's path> --private_key_path <private key pem's path> -t <corstone315/corstone320> --toolchain GNU --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS> -c
```

## Provisioning the device credentials into Protected Storage

Check [Device Provisioning](./device_provisioning/device_provisioning.md) for detailed information.

## Running the application

```bash
./tools/scripts/run.sh object-detection -t <corstone315/corstone320> --frames applications/object_detection/resources/test.frm
```
- The `frames` is used to select the input frames file.

### Expected output

```log
[INF] Starting TF-M BL1_1
[INF] Jumping to BL1_2
[INF] Starting TF-M BL1_2
[INF] Attempting to boot image 0
[INF] BL2 image decrypted successfully
[INF] BL2 image validated successfully
[INF] Jumping to BL2
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
1 0 [None] [INFO] Signal task inference stop
2 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
3 0 [IP-Task] vIPSetDHCP_RATimerEnableState: Off
4 0 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
5 27 [OTA Task] [INFO] OTA over MQTT, firmware versions:
6 33 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.2
7 40 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.20
8 47 [ML_TASK] [INFO] ML Task start
9 51 [BLINK_TASK ] [INFO] Blink task started
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
22 6317 [MQTT Agent Task] DNS[0xB667]: The answer to '<iot-core-endpoint>.amazonaws.com' (54.247.164.78) will be stored
23 6331 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
24 6344 [MQTT Agent Task] FreeRTOS_connect: 6371 to 36f7a44eip:8883
25 6420 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
26 8264 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
27 8277 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.
28 8479 [MQTT Agent Task] [INFO] Successfully connected to the MQTT broker.
29 8487 [MQTT Agent Task] [INFO] Session present: 0
30 8492 [MQTT Agent Task] [INFO] Starting a clean MQTT Session.
31 8499 [OTA Agent Task] [INFO] Requesting job document.
32 8507 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/start-next to broker.
33 8673 [MQTT Agent Task] [INFO] Received job message callback, size 1080.
34 8684 [OTA Agent Task] [INFO] Extracted parameter: [jobid: AFR_OTA-ota-test-update-id-ff61f445-de82-4d18-b0d7-e848797356fa]
35 8697 [OTA Agent Task] [INFO] Extracted parameter: [streamname: AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2]
36 8709 [OTA Agent Task] [INFO] Extracted parameter: [filepath: non_secure image]
37 8717 [OTA Agent Task] [INFO] Extracted parameter: [filesize: 564703]
38 8725 [OTA Agent Task] [INFO] Extracted parameter: [fileid: 0]
39 8732 [OTA Agent Task] [INFO] Extracted parameter: [certfile: 0]
40 8739 [OTA Agent Task] [INFO] Extracted parameter: [sig-sha256-rsa: NuNyhq1NrCPe6Mpd0eAmf605MbCYBW4obec...]
41 8800 [OTA Agent Task] [INFO] Application version of the new image is not higher than the current image: New images are expected to have a higher version number.
42 9433 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-ff61f445-de82-4d18-b0d7-e848797356fa/update to broker.
43 9456 [OTA Agent Task] [INFO] No OTA job available.
44 9462 [OTA Agent Task] [INFO] Signal task inference start
45 9468 [ML_TASK] [INFO] Initial start of image processing
INFO - Initialising Ethos-U device@0x40004000
INFO - Ethos-U device initialised
INFO - Ethos-U version info:
INFO - 	Arch:       v1.1.0
INFO - 	Driver:     v0.16.0
INFO - 	MACs/cc:    256
INFO - 	Cmd stream: v0
46 9497 [ML_TASK] [INFO] Ethos-U55 device initialised
47 9502 [ML_TASK] [INFO] Ethos-U version info:
48 9507 [ML_TASK] [INFO] 	Arch:       v1.1.0
49 9512 [ML_TASK] [INFO] 	Driver:     v0.16.0
50 9517 [ML_TASK] [INFO] 	MACs/cc:    256
51 9521 [ML_TASK] [INFO] 	Cmd stream: v0
INFO - Added ethos-u support to op resolver
INFO - Creating allocator using tensor arena at 0x6006b440
INFO - Allocating tensors
INFO - Model INPUT tensors:
INFO - 	tensor type is INT8
INFO - 	tensor occupies 36864 bytes with dimensions
INFO - 		0:   1
INFO - 		1: 192
INFO - 		2: 192
INFO - 		3:   1
INFO - Quant dimension: 0
INFO - Scale[0] = 0.003921
INFO - ZeroPoint[0] = -128
INFO - Model OUTPUT tensors:
INFO - 	tensor type is INT8
INFO - 	tensor occupies 648 bytes with dimensions
INFO - 		0:   1
INFO - 		1:   6
INFO - 		2:   6
INFO - 		3:  18
INFO - Quant dimension: 0
INFO - Scale[0] = 0.134084
INFO - ZeroPoint[0] = 47
INFO - 	tensor type is INT8
INFO - 	tensor occupies 2592 bytes with dimensions
INFO - 		0:   1
INFO - 		1:  12
INFO - 		2:  12
INFO - 		3:  18
INFO - Quant dimension: 0
INFO - Scale[0] = 0.185359
INFO - ZeroPoint[0] = 10
INFO - Activation buffer (a.k.a tensor arena) size used: 370056
INFO - Number of operators: 1
INFO - 	Operator 0: ethos-u
INFO - Model address: 0x60000000INFO - Model size:      439360 bytes.INFO - Model info:
INFO - Model INPUT tensors:
INFO - 	tensor type is INT8
INFO - 	tensor occupies 36864 bytes with dimensions
INFO - 		0:   1
INFO - 		1: 192
INFO - 		2: 192
INFO - 		3:   1
INFO - Quant dimension: 0
INFO - Scale[0] = 0.003921
INFO - ZeroPoint[0] = -128
INFO - Model OUTPUT tensors:
INFO - 	tensor type is INT8
INFO - 	tensor occupies 648 bytes with dimensions
INFO - 		0:   1
INFO - 		1:   6
INFO - 		2:   6
INFO - 		3:  18
INFO - Quant dimension: 0
INFO - Scale[0] = 0.134084
INFO - ZeroPoint[0] = 47
INFO - 	tensor type is INT8
INFO - 	tensor occupies 2592 bytes with dimensions
INFO - 		0:   1
INFO - 		1:  12
INFO - 		2:  12
INFO - 		3:  18
INFO - Quant dimension: 0
INFO - Scale[0] = 0.185359
INFO - ZeroPoint[0] = 10
INFO - Activation buffer (a.k.a tensor arena) size used: 370056
INFO - Number of operators: 1
INFO - 	Operator 0: ethos-u
INFO - The model is optimised for Ethos-U NPU: yes.
52 9732 [ML_TASK] [INFO] *** ML interface initialised
53 9738 [ML_TASK] [INFO] ISP init
54 9742 [ML_TASK] [INFO] Starting HDLCD config!
55 9747 [ML_TASK] [INFO] Starting ISP init!
00T00:00:09.752Z GENERIC(ERR) :PINGISP
1:4830ab6c(58140) <-> 212f7234(58140)
2:48318e88(16384) <-> 21305550(16384)
00T00:00:09.764Z GENERIC(ERR) :PoNGISP
1:48322b2c(58140) <-> 212f7234(58140)
2:48330e48(16384) <-> 21305550(16384)
00T00:00:09.776Z GENERIC(ERR) :PINGMET
1:483054a8(4096) <-> 212f1b70(4096)
2:483095ac(4672) <-> 212f5c74(4672)
00T00:00:09.787Z GENERIC(ERR) :PoNGMET
1:483054a8(4096) <-> 212f1b70(4096)
2:4832156c(4672) <-> 212f5c74(4672)
00T00:00:09.800Z SENSOR(NOTICE) :Sensor initialization is complete, ID 0xFFFF resolution 576x576
00T00:00:09.810Z GENERIC(ERR) :DMA alloc: 0xca800, Memory left: 0x47b800
00T00:00:09.820Z CROP(NOTICE) :FR update: Crop: e 0 x 0, y 0, w 576, h 576, Downscaler: e 0, w 576, h 576
00T00:00:09.831Z CROP(NOTICE) :DS update: Crop: e 0 x 0, y 0, w 576, h 576, Downscaler: e 0, w 576, h 576
00T00:00:09.853Z GENERIC(CRIT) :[1;34m-- CAMERA STREAM TRIGGER #0 --[1;0m
abstract_port__vsync_s__setValue : (idx = 0) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #0 is ready.
00T00:00:09.860Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 0
00T00:00:09.860Z GENERIC(ERR) :Attempt to start a new frame before processing is done for the previous one. Skip this frame.
00T00:00:09.976Z GENERIC(CRIT) :[1;34m-- CAMERA STREAM TRIGGER #1 --[1;0m
abstract_port__vsync_s__setValue : (idx = 0) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #1 is ready.
00T00:00:09.983Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 1
00T00:00:10.099Z GENERIC(CRIT) :[1;34m-- CAMERA STREAM TRIGGER #2 --[1;0m
abstract_port__vsync_s__setValue : (idx = 0) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #2 is ready.
00T00:00:10.106Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 2
00T00:00:10.222Z GENERIC(CRIT) :[1;34m-- CAMERA STREAM TRIGGER #3 --[1;0m
abstract_port__vsync_s__setValue : (idx = 0) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #3 is ready.
00T00:00:10.229Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 3
00T00:00:10.345Z GENERIC(CRIT) :[1;34m-- CAMERA STREAM TRIGGER #4 --[1;0m
abstract_port__vsync_s__setValue : (idx = 0) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #4 is ready.
00T00:00:10.352Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 4
00T00:00:10.358Z GENERIC(CRIT) :[1;33m-- aframes->frame_id = 1 --[1;0m
00T00:00:10.366Z GENERIC(CRIT) :[1;33m-- 576 X 576 @ 2 bytes per pixel --[1;0m
56 10374 [acamera] [INFO] Displaying image...
57 10379 [acamera] [INFO] Image displayed
00T00:00:10.384Z GENERIC(CRIT) :[1;33m-- aframes->frame_id = 1 --[1;0m
00T00:00:10.392Z GENERIC(CRIT) :[1;33m-- 576 X 576 @ 2 bytes per pixel --[1;0m
58 10400 [acamera] [ERROR] Input frame too big for inference!
00T00:00:10.422Z GENERIC(CRIT) :[1;34m-- CAMERA STREAM TRIGGER #5 --[1;0m
abstract_port__vsync_s__setValue : (idx = 0) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #5 is ready.
00T00:00:10.429Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 5
00T00:00:10.438Z GENERIC(CRIT) :[1;33m-- aframes->frame_id = 2 --[1;0m
00T00:00:10.446Z GENERIC(CRIT) :[1;33m-- 576 X 576 @ 2 bytes per pixel --[1;0m
59 10454 [acamera] [INFO] Displaying image...
60 10459 [acamera] [INFO] Image displayed
00T00:00:10.464Z GENERIC(CRIT) :[1;33m-- aframes->frame_id = 2 --[1;0m
00T00:00:10.471Z GENERIC(CRIT) :[1;33m-- 192 X 192 @ 2 bytes per pixel --[1;0m
61 10480 [acamera] [INFO] Converting to Gray: 0x82f76000 -> 0x21286b00
INFO - Running inference on image at addr 0x21286b00
[warning ][main@0][4647 ns] TA0 is not enabled!
[warning ][main@0][4647 ns] TA1 is not enabled!
62 10506 [acamera] [INFO] Final results:
63 10511 [acamera] [INFO] Total number of inferences: 1
64 10517 [acamera] [INFO] Detected faces: 2
65 10521 [acamera] [INFO] 0) (0.999633) -> Detection box: {x=72,y=11,w=39,h=53}
66 10530 [acamera] [INFO] 1) (0.907045) -> Detection box: {x=32,y=73,w=38,h=58}
67 10538 [acamera] [INFO] Complete recognition: Detected faces: 2
```

## Observing MQTT connectivity

Follow the instructions described in the [Observing MQTT connectivity](./aws_iot/aws_iot_cloud_connection.md) section.

## Firmware update with AWS

Follow the instructions described in the [Firmware update with AWS](./aws_iot/aws_iot_cloud_connection.md) section.

### Expected output

```log
[INF] Starting TF-M BL1_1
[INF] Beginning provisioning
[INF] TP mode set complete, system will now reset.
[INF] Starting TF-M BL1_1
[INF] Beginning provisioning
[INF] Waiting for CM provisioning bundle
[INF] Enabling secure provisioning mode, system will now reset.
[INF] Starting TF-M BL1_1
[INF] Beginning provisioning
[INF] Waiting for CM provisioning bundle
[INF] Running CM provisioning bundle
[INF] Starting TF-M BL1_1
[INF] Beginning provisioning
[INF] Waiting for DM provisioning bundle
[INF] Enabling secure provisioning mode, system will now reset.
[INF] Starting TF-M BL1_1
[INF] Beginning provisioning
[INF] Waiting for DM provisioning bundle
[INF] Running DM provisioning bundle
[INF] Starting TF-M BL1_1
[INF] Jumping to BL1_2
[INF] Starting TF-M BL1_2
[INF] Attempting to boot image 0
[INF] BL2 image decrypted successfully
[INF] BL2 image validated successfully
[INF] Jumping to BL2
[INF] Starting bootloader
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
4 0 [None] [INFO] Signal task inference stop
5 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
6 0 [IP-Task] vIPSetDHCP_RATimerEnableState: Off
7 0 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
8 43 [OTA Task] [INFO] OTA over MQTT, firmware versions:
9 49 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.2
10 55 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.10
11 62 [ML_TASK] [INFO] ML Task start
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
25 6317 [MQTT Agent Task] DNS[0x4EBF]: The answer to '<iot-core-endpoint>.amazonaws.com' (34.251.115.106) will be stored
26 6331 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
27 6344 [MQTT Agent Task] FreeRTOS_connect: 32416 to 22fb736aip:8883
28 6413 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
29 8257 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
30 8271 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.
31 8485 [MQTT Agent Task] [INFO] Successfully connected to the MQTT broker.
32 8493 [MQTT Agent Task] [INFO] Session present: 0
33 8498 [MQTT Agent Task] [INFO] Starting a clean MQTT Session.
34 8505 [OTA Agent Task] [INFO] Requesting job document.
35 8513 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/start-next to broker.
36 8690 [MQTT Agent Task] [INFO] Received job message callback, size 1046.
37 8701 [OTA Agent Task] [INFO] Extracted parameter: [jobid: AFR_OTA-ota-test-update-id-ff61f445-de82-4d18-b0d7-e848797356fa]
38 8714 [OTA Agent Task] [INFO] Extracted parameter: [streamname: AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2]
39 8726 [OTA Agent Task] [INFO] Extracted parameter: [filepath: non_secure image]
40 8734 [OTA Agent Task] [INFO] Extracted parameter: [filesize: 564703]
41 8742 [OTA Agent Task] [INFO] Extracted parameter: [fileid: 0]
42 8749 [OTA Agent Task] [INFO] Extracted parameter: [certfile: 0]
43 8756 [OTA Agent Task] [INFO] Extracted parameter: [sig-sha256-rsa: NuNyhq1NrCPe6Mpd0eAmf605MbCYBW4obec...]
44 9677 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2/data/json.
45 9722 [OTA Agent Task] [INFO] Received OTA job document.
46 9729 [OTA Agent Task] [INFO] Signal task inference stop
47 9735 [OTA Agent Task] [INFO] Requesting file block.
48 9741 [OTA Agent Task] [INFO] Starting the download.
49 10429 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2/get/json to broker.
50 11173 [MQTT Agent Task] [INFO] Received data message callback, size 5493.
51 11181 [OTA Agent Task] [INFO] Received file block.
52 11189 [OTA Agent Task] [INFO] Downloaded block 0 of 138.
53 11196 [OTA Agent Task] [INFO] Requesting file block.
54 11933 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2/get/json to broker.
55 12678 [MQTT Agent Task] [INFO] Received data message callback, size 5493.
56 12686 [OTA Agent Task] [INFO] Received file block.
57 12694 [OTA Agent Task] [INFO] Downloaded block 1 of 138.
58 12701 [OTA Agent Task] [INFO] Requesting file block.

...

715 203443 [MQTT Agent Task] [INFO] Received data message callback, size 5495.
716 203451 [OTA Agent Task] [INFO] Received file block.
717 203459 [OTA Agent Task] [INFO] Downloaded block 133 of 138.
718 203466 [OTA Agent Task] [INFO] Requesting file block.
719 204203 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2/get/json to broker.
720 204884 [MQTT Agent Task] [INFO] Received data message callback, size 5495.
721 204892 [OTA Agent Task] [INFO] Received file block.
722 204900 [OTA Agent Task] [INFO] Downloaded block 134 of 138.
723 204907 [OTA Agent Task] [INFO] Requesting file block.
724 205644 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2/get/json to broker.
725 206332 [MQTT Agent Task] [INFO] Received data message callback, size 5495.
726 206340 [OTA Agent Task] [INFO] Received file block.
727 206348 [OTA Agent Task] [INFO] Downloaded block 135 of 138.
728 206355 [OTA Agent Task] [INFO] Requesting file block.
729 207092 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2/get/json to broker.
730 207775 [MQTT Agent Task] [INFO] Received data message callback, size 5495.
731 207783 [OTA Agent Task] [INFO] Received file block.
732 207791 [OTA Agent Task] [INFO] Downloaded block 136 of 138.
733 207798 [OTA Agent Task] [INFO] Requesting file block.
734 208535 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-46bfd071-34ab-4150-880e-0cc15be203d2/get/json to broker.
735 209013 [MQTT Agent Task] [INFO] Received data message callback, size 4767.
736 209022 [OTA Agent Task] [INFO] Received file block.
737 209029 [OTA Agent Task] [INFO] Downloaded block 137 of 138.
738 209036 [OTA Agent Task] [INFO] Closing file.
739 209304 [OTA Agent Task] [INFO] Attempting to activate image.

...

[INF] Starting TF-M BL1_1
[INF] Jumping to BL1_2
[INF] Starting TF-M BL1_2
[INF] Attempting to boot image 0
[INF] BL2 image decrypted successfully
[INF] BL2 image validated successfully
[INF] Jumping to BL2
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
1 0 [None] [INFO] Signal task inference stop
2 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
3 0 [IP-Task] vIPSetDHCP_RATimerEnableState: Off
4 0 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
5 27 [OTA Task] [INFO] OTA over MQTT, firmware versions:
6 33 [OTA Task] [INFO] Secure Component (ID 0) version=2.2.2
7 40 [OTA Task] [INFO] Non-Secure Component (ID 1) version=0.0.20
```
