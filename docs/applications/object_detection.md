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

To build the Object-Detection example, run the following command:
```bash
./tools/scripts/build.sh object-detection --certificate_path <certificate pem's path> --private_key_path <private key pem's path> -t corstone315 --toolchain GNU --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```
- The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's path if you chose the **Auto-generate a new certificate** during the Thing creation. If you chose **Skip creating a certificate at this time** then these paths should locate the generated credential files that were created by the `./tools/scripts/generate_credentials.py` script in the previous step.

- The `toolchain` is used to select the `GNU`, that supports the `Mali-C55`.

* The `conn-stack` is used to select the connectivity stack to be used whether it's `FREERTOS_PLUS_TCP` or `IOT_VSOCKET`.

* The `psa-crypto-implementation` is used to select the library providing the PSA Crypto APIs implementation whether it's `TF-M` or `MBEDTLS`. For more information about the PSA Crypto APIs
implementation, please refer to [Mbed TLS document](../components/security/mbedtls/mbedtls.md#psa-crypto-apis-implementation).

Or, run the command below to perform a clean build:
```bash
./tools/scripts/build.sh object-detection --certificate_path <certificate pem's path> --private_key_path <private key pem's path> -t corstone315 --toolchain GNU --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS> -c
```

## Provisioning the device credentials into Protected Storage

Check [Device Provisioning](./device_provisioning/device_provisioning.md) for detailed information.

## Running the application

```bash
./tools/scripts/run.sh object-detection -t corstone315 --frames applications/object_detection/resources/test.frm
```
- The `frames` is used to select the input frames file.

### Expected output

```log
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
Booting TF-M v2.1.0
<NUL>[Sec Thread] Secure image initializing!
<NUL>[INF][Crypto] Provisioning entropy seed... complete.
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [None] [INFO] Signal task inference stop
5 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
6 41 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.20
7 54 [ML_TASK] [INFO] ML Task start
8 59 [BLINK_TASK ] [INFO] Blink task started
9 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
10 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
11 1000 [IP-Task] vDHCP_RATimerReload: 250
12 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
13 1260 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
14 1260 [IP-Task] [INFO] Network is up
15 1260 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
16 1260 [IP-Task] vDHCP_RATimerReload: 8640000
17 1291 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
18 1307 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
19 1322 [IP-Task] ipARP_REPLY from ac1433feip to ac143301ip end-point ac143301ip
20 6334 [MQTT Agent Task] DNS_ReadReply returns -11
21 6341 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
22 6356 [MQTT Agent Task] DNS[0xE135]: The answer to '<iot-core-endpoint>.amazonaws.com' (99.81.255.142) will be stored
23 6374 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
24 6392 [MQTT Agent Task] FreeRTOS_connect: 37762 to 6351ff8eip:8883
25 6492 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
26 8622 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
27 8640 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.
28 8993 [MQTT Agent Task] [INFO] MQTT connection established with the broker.
29 9004 [MQTT Agent Task] [INFO] Successfully connected to the MQTT broker.
30 9014 [MQTT Agent Task] [INFO] Session present: 0
31 9022 [MQTT Agent Task] [INFO] Starting a clean MQTT Session.
32 9031 [OTA Task ] [INFO]  Received: 0   Queued: 0   Processed: 0   Dropped: 0
33 9042 [OTA Agent Task] [INFO] Current State=[RequestingJob], Event=[Start], New state=[RequestingJob]
34 9413 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/jobs/notify-next.
35 9432 [OTA Agent Task] [INFO] Subscribed to MQTT topic: $aws/things/<mqtt-client-identifier>/jobs/notify-next
36 10163 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/jobs/$next/get.
37 10484 [MQTT Agent Task] [INFO] Ack packet deserialized with result: MQTTSuccess.
38 10496 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
39 10508 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/$next/get to broker.
40 10530 [OTA Agent Task] [WARN] OTA Timer handle NULL for Timerid=0, can't stop.
41 10542 [OTA Agent Task] [INFO] Current State=[WaitingForJob], Event=[RequestJobDocument], New state=[WaitingForJob]
42 11258 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
43 11272 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
44 11284 [MQTT Agent Task] [INFO] Received job message callback, size 1112.
45 11295 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobId: AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a]
46 11318 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.statusDetails.updatedBy: 10]
47 11335 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.streamname: AFR_OTA-eac26517-1848-4705-8242-97bda52cab40]
48 11358 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.protocols: ["MQTT"]]
49 11375 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filepath: non_secure image]
50 11389 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filesize: 1141641]
51 11402 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[fileid: 0]
52 11413 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[certfile: 0]
53 11426 [OTA Agent Task] [INFO] Extracted parameter [ sig-sha256-rsa: LUOGFk3dx7VoXvS7tJl5MJObgHtbCJgZ... ]
54 11442 [OTA Agent Task] [INFO] In self test mode.
55 11450 [OTA Agent Task] [INFO] New image has a higher version number than the current image: New image version=0.0.20, Previous image version=0.0.10
56 11471 [OTA Agent Task] [INFO] Image version is valid: Begin testing file: File ID=0
57 12044 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a/update.
58 12372 [MQTT Agent Task] [INFO] Ack packet deserialized with result: MQTTSuccess.
59 12383 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
60 12395 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
61 12410 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
62 12421 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a/update to broker.
63 12452 [OTA Agent Task] [INFO] Job parsing success: OtaJobParseErr_t=OtaJobParseErrNone, Job name=AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a
64 12475 [OTA Agent Task] [INFO] Signal task inference stop
65 12484 [OTA Agent Task] [INFO] Current State=[CreatingFile], Event=[ReceivedJobDocument], New state=[CreatingFile]
66 12500 [OTA Agent Task] [INFO] Beginning self-test.
67 12508 [OTA Agent Task] [INFO] Received OtaJobEventStartTest callback from OTA Agent.
68 13171 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a/update.
69 13498 [MQTT Agent Task] [INFO] Ack packet deserialized with result: MQTTSuccess.
70 13510 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
71 13521 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
72 13536 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
73 13548 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a/update to broker.
74 13579 [OTA Agent Task] [INFO] New image validation succeeded in self test mode.
75 13590 [OTA Agent Task] [INFO] OTA active state `4` from OTA Agent.
76 13600 [OTA Agent Task] [INFO] Signal task inference stop
77 13608 [OTA Agent Task] [WARN] OTA Timer handle NULL for Timerid=1, can't stop.
78 13620 [OTA Agent Task] [INFO] Current State=[WaitingForJob], Event=[StartSelfTest], New state=[WaitingForJob]
79 17048 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
80 17062 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPubAckSend.
81 17074 [MQTT Agent Task] [INFO] Received job message callback, size 24.
82 17085 [OTA Agent Task] [INFO] No active job available in received job document: OtaJobParseErr_t=OtaJobParseErrNoActiveJobs
83 17102 [OTA Agent Task] [INFO] Signal task inference start
84 17111 [OTA Agent Task] [INFO] Current State=[WaitingForJob], Event=[ReceivedJobDocument], New state=[CreatingFile]
85 17127 [ML_TASK] [INFO] Initial start of image processing
INFO - Ethos-U device initialised
INFO - Ethos-U version info:
INFO - 	Arch:       v1.1.0
INFO - 	Driver:     v0.16.0
INFO - 	MACs/cc:    256
INFO - 	Cmd stream: v0
86 17159 [ML_TASK] [INFO] Ethos-U65 device initialised
87 17167 [ML_TASK] [INFO] Ethos-U version info:
88 17173 [ML_TASK] [INFO] 	Arch:       v1.1.0
89 17180 [ML_TASK] [INFO] 	Driver:     v0.16.0
90 17187 [ML_TASK] [INFO] 	MACs/cc:    256
91 17192 [ML_TASK] [INFO] 	Cmd stream: v0
INFO - Added ethos-u support to op resolver
INFO - Creating allocator using tensor arena at 0x0x6006b440
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
INFO - Activation buffer (a.k.a tensor arena) size used: 370024
INFO - Number of operators: 1
INFO - 	Operator 0: ethos-u
INFO - Model address: 0x0x60000000INFO - Model size:      439360 bytes.INFO - Model info:
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
INFO - Activation buffer (a.k.a tensor arena) size used: 370024
INFO - Number of operators: 1
INFO - 	Operator 0: ethos-u
INFO - The model is optimised for Ethos-U NPU: yes.
92 17474 [ML_TASK] [INFO] *** ML interface initialised
93 17481 [ML_TASK] [INFO] ISP init
94 17486 [ML_TASK] [INFO] Starting HDLCD config!
95 17493 [ML_TASK] [INFO] Starting ISP init!
00T00:00:17.500Z GENERIC(ERR) :PINGISP
1:4830ab6c(58140) <-> 0x212f6f7c(58140)
2:48318e88(16384) <-> 0x21305298(16384)
00T00:00:17.517Z GENERIC(ERR) :PoNGISP
1:48322b2c(58140) <-> 0x212f6f7c(58140)
2:48330e48(16384) <-> 0x21305298(16384)
00T00:00:17.534Z GENERIC(ERR) :PINGMET
1:483054a8(4096) <-> 0x212f18b8(4096)
2:483095ac(4672) <-> 0x212f59bc(4672)
00T00:00:17.550Z GENERIC(ERR) :PoNGMET
1:483054a8(4096) <-> 0x212f18b8(4096)
2:4832156c(4672) <-> 0x212f59bc(4672)
00T00:00:17.567Z SENSOR(NOTICE) :Sensor initialization is complete, ID 0xFFFF resolution 576x576
00T00:00:17.581Z GENERIC(ERR) :DMA alloc: 0xca800, Memory left: 0x47b800
00T00:00:17.596Z CROP(NOTICE) :FR update: Crop: e 0 x 0, y 0, w 576, h 576, Downscaler: e 0, w 576, h 576
00T00:00:17.611Z CROP(NOTICE) :DS update: Crop: e 0 x 0, y 0, w 576, h 576, Downscaler: e 0, w 576, h 576
00T00:00:17.636Z GENERIC(CRIT) :-- CAMERA STREAM TRIGGER #0 --
abstract_port__vsync_s__setValue : (idx = 0) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #0 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #0 is ready.
00T00:00:17.646Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 0
00T00:00:17.646Z GENERIC(ERR) :Attempt to start a new frame before processing is done for the previous one. Skip this frame.
00T00:00:17.762Z GENERIC(CRIT) :-- CAMERA STREAM TRIGGER #1 --
abstract_port__vsync_s__setValue : (idx = 0) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #1 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #1 is ready.
00T00:00:17.772Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 1
00T00:00:17.888Z GENERIC(CRIT) :-- CAMERA STREAM TRIGGER #2 --
abstract_port__vsync_s__setValue : (idx = 0) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #2 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #2 is ready.
00T00:00:17.898Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 2
00T00:00:18.014Z GENERIC(CRIT) :-- CAMERA STREAM TRIGGER #3 --
abstract_port__vsync_s__setValue : (idx = 0) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #3 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #3 is ready.
00T00:00:18.024Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 3
00T00:00:18.140Z GENERIC(CRIT) :-- CAMERA STREAM TRIGGER #4 --
abstract_port__vsync_s__setValue : (idx = 0) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #4 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #4 is ready.
00T00:00:18.150Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 4
00T00:00:18.155Z GENERIC(CRIT) :-- aframes->frame_id = 1 --
00T00:00:18.165Z GENERIC(CRIT) :-- 576 X 576 @ 2 bytes per pixel --
96 18177 [acamera] [INFO] Displaying image...
97 18183 [acamera] [INFO] Image displayed
00T00:00:18.189Z GENERIC(CRIT) :-- aframes->frame_id = 1 --
00T00:00:18.199Z GENERIC(CRIT) :-- 576 X 576 @ 2 bytes per pixel --
98 18211 [acamera] [ERROR] Input frame too big for inference!
00T00:00:18.235Z GENERIC(CRIT) :-- CAMERA STREAM TRIGGER #5 --
abstract_port__vsync_s__setValue : (idx = 0) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #5 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #5 is ready.
00T00:00:18.245Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 5
00T00:00:18.252Z GENERIC(CRIT) :-- aframes->frame_id = 2 --
00T00:00:18.262Z GENERIC(CRIT) :-- 576 X 576 @ 2 bytes per pixel --
99 18274 [acamera] [INFO] Displaying image...
100 18280 [acamera] [INFO] Image displayed
00T00:00:18.286Z GENERIC(CRIT) :-- aframes->frame_id = 2 --
00T00:00:18.296Z GENERIC(CRIT) :-- 192 X 192 @ 2 bytes per pixel --
101 18308 [acamera] [INFO] Converting to Gray: 0x82f76000 -> 0x21224300
INFO - Running inference on image at addr 0x21224300
102 18340 [acamera] [INFO] Final results:
103 18346 [acamera] [INFO] Total number of inferences: 1
104 18354 [acamera] [INFO] Detected faces: 3
105 18360 [acamera] [INFO] 0) (0.994434) -> Detection box: {x=6,y=71,w=33,h=44}
106 18371 [acamera] [INFO] 1) (0.950913) -> Detection box: {x=108,y=71,w=13,h=21}
107 18383 [acamera] [INFO] 2) (0.884787) -> Detection box: {x=47,y=78,w=20,h=31}
108 18394 [acamera] [INFO] Complete recognition: Detected faces: 3
109 18534 [ML_MQTT] [INFO] Attempting to publish (Detected faces: 3) to the MQTT topic <mqtt-client-identifier>/ml/inference.
110 18557 [MQTT Agent Task] [INFO] Publishing message to <mqtt-client-identifier>/ml/inference.
00T00:00:18.575Z GENERIC(CRIT) :-- CAMERA STREAM TRIGGER #6 --
abstract_port__vsync_s__setValue : (idx = 0) Frame #6 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #6 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #6 is ready.
abstract_port__vsync_s__setValue : (idx = 0) Frame #6 is ready.
abstract_port__vsync_s__setValue : (idx = 1) Frame #6 is ready.
abstract_port__vsync_s__setValue : (idx = 2) Frame #6 is ready.
00T00:00:18.585Z GENERIC(CRIT) :[KeyMsg]: FS interrupt: 6
00T00:00:18.588Z GENERIC(CRIT) :-- aframes->frame_id = 3 --
00T00:00:18.598Z GENERIC(CRIT) :-- 576 X 576 @ 2 bytes per pixel --
111 18610 [acamera] [INFO] Displaying image...
112 18616 [acamera] [INFO] Image displayed
00T00:00:18.622Z GENERIC(CRIT) :-- aframes->frame_id = 3 --
00T00:00:18.633Z GENERIC(CRIT) :-- 192 X 192 @ 2 bytes per pixel --
113 18644 [acamera] [INFO] Converting to Gray: 0x837e6000 -> 0x21224300
INFO - Running inference on image at addr 0x21224300
114 18676 [acamera] [INFO] Final results:
115 18682 [acamera] [INFO] Total number of inferences: 1
116 18690 [acamera] [INFO] Detected faces: 4
117 18696 [acamera] [INFO] 0) (0.996794) -> Detection box: {x=6,y=71,w=33,h=44}
118 18707 [acamera] [INFO] 1) (0.986066) -> Detection box: {x=108,y=71,w=13,h=21}
119 18719 [acamera] [INFO] 2) (0.958923) -> Detection box: {x=47,y=78,w=20,h=31}
120 18730 [acamera] [INFO] 3) (0.546118) -> Detection box: {x=170,y=91,w=16,h=21}
121 18741 [acamera] [INFO] Complete recognition: Detected faces: 4
```

## Observing MQTT connectivity

Follow the instructions described in the [Observing MQTT connectivity](./aws_iot/aws_iot_cloud_connection.md) section.

## Firmware update with AWS

Follow the instructions described in the [Firmware update with AWS](./aws_iot/aws_iot_cloud_connection.md) section.

### Expected output

```log
[INF] Starting bootloader
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
Booting TF-M v2.1.0
<NUL>[Sec Thread] Secure image initializing!
<NUL>Creating an empty ITS flash layout.
Creating an empty PS flash layout.
[INF][Crypto] Provisioning entropy seed... complete.
0 0 [None] [INFO] PSA Framework version is: 257
1 0 [None] Write certificate...
2 0 [None] [INFO] Device key provisioning succeeded
3 0 [None] [INFO] OTA signing key provisioning succeeded
4 0 [None] [INFO] Signal task inference stop
5 0 [None] FreeRTOS_AddEndPoint: MAC: 44-21 IPv4: c0a80069ip
6 41 [OTA Task ] [INFO] OTA over MQTT, Application version from appFirmwareVersion 0.0.10
7 54 [ML_TASK] [INFO] ML Task start
8 59 [BLINK_TASK ] [INFO] Blink task started
9 1000 [IP-Task] DHCP-socket[44-21]: DHCP Socket Create
10 1000 [IP-Task] prvCreateDHCPSocket[44-21]: open, user count 1
11 1000 [IP-Task] vDHCP_RATimerReload: 250
12 1250 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
13 1260 [IP-Task] vDHCPProcess: offer ac143301ip for MAC address 44-21
14 1260 [IP-Task] [INFO] Network is up
15 1260 [IP-Task] prvCloseDHCPSocket[44-21]: closed, user count 0
16 1260 [IP-Task] vDHCP_RATimerReload: 8640000
17 1291 [MQTT Agent Task] [INFO] Creating a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
18 1307 [MQTT Agent Task] [INFO] Resolving host name: <iot-core-endpoint>.amazonaws.com.
19 1323 [IP-Task] ipARP_REPLY from ac1433feip to ac143301ip end-point ac143301ip
20 6334 [MQTT Agent Task] DNS_ReadReply returns -11
21 6341 [MQTT Agent Task] prvIncreaseDNS4Index: from 0 to 0
22 6352 [MQTT Agent Task] DNS[0xB324]: The answer to '<iot-core-endpoint>.amazonaws.com' (52.208.143.40) will be stored
23 6371 [MQTT Agent Task] [INFO] Initiating TCP connection with host: <iot-core-endpoint>.amazonaws.com:8883
24 6388 [MQTT Agent Task] FreeRTOS_connect: 25748 to 34d08f28ip:8883
25 6482 [MQTT Agent Task] [INFO] Initiating TLS handshake with host: <iot-core-endpoint>.amazonaws.com:8883
26 8653 [MQTT Agent Task] [INFO] Successfully created a TLS connection to <iot-core-endpoint>.amazonaws.com:8883.
27 8671 [MQTT Agent Task] [INFO] Creating an MQTT connection to the broker.
28 9099 [MQTT Agent Task] [INFO] MQTT connection established with the broker.
29 9110 [MQTT Agent Task] [INFO] Successfully connected to the MQTT broker.
30 9121 [MQTT Agent Task] [INFO] Session present: 0
31 9128 [MQTT Agent Task] [INFO] Starting a clean MQTT Session.
32 9137 [OTA Task ] [INFO]  Received: 0   Queued: 0   Processed: 0   Dropped: 0
33 9149 [OTA Agent Task] [INFO] Current State=[RequestingJob], Event=[Start], New state=[RequestingJob]
34 9633 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/jobs/notify-next.
35 9653 [OTA Agent Task] [INFO] Subscribed to MQTT topic: $aws/things/<mqtt-client-identifier>/jobs/notify-next
36 10383 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/jobs/$next/get.
37 10922 [MQTT Agent Task] [INFO] Ack packet deserialized with result: MQTTSuccess.
38 10934 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
39 10946 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
40 10961 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
41 10973 [MQTT Agent Task] [INFO] Received job message callback, size 1020.
42 10983 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/$next/get to broker.
43 11006 [OTA Agent Task] [WARN] OTA Timer handle NULL for Timerid=0, can't stop.
44 11017 [OTA Agent Task] [INFO] Current State=[WaitingForJob], Event=[RequestJobDocument], New state=[WaitingForJob]
45 11035 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobId: AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a]
46 11060 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.streamname: AFR_OTA-eac26517-1848-4705-8242-97bda52cab40]
47 11082 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[execution.jobDocument.afr_ota.protocols: ["MQTT"]]
48 11100 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filepath: non_secure image]
49 11114 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[filesize: 1141641]
50 11127 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[fileid: 0]
51 11138 [OTA Agent Task] [INFO] Extracted parameter: [key: value]=[certfile: 0]
52 11151 [OTA Agent Task] [INFO] Extracted parameter [ sig-sha256-rsa: LUOGFk3dx7VoXvS7tJl5MJObgHtbCJgZ... ]
53 11167 [OTA Agent Task] [INFO] Job document was accepted. Attempting to begin the update.
54 11180 [OTA Agent Task] [INFO] Job parsing success: OtaJobParseErr_t=OtaJobParseErrNone, Job name=AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a
55 11203 [OTA Agent Task] [INFO] Signal task inference stop
56 11253 [OTA Agent Task] [INFO] Setting OTA data interface.
57 11261 [OTA Agent Task] [INFO] Current State=[CreatingFile], Event=[ReceivedJobDocument], New state=[CreatingFile]
58 12464 [OTA Agent Task] [INFO] Subscribed to topic $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/data/cbor.
59 12490 [OTA Agent Task] [INFO] Current State=[RequestingFileBlock], Event=[CreateFile], New state=[RequestingFileBlock]
60 13214 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor.
61 13399 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor to broker.
62 13428 [OTA Agent Task] [INFO] Published to MQTT topic to request the next block: topic=$aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor
63 13459 [OTA Agent Task] [INFO] Current State=[WaitingForFileBlock], Event=[RequestFileBlock], New state=[WaitingForFileBlock]
64 14010 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
65 14024 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
66 14036 [MQTT Agent Task] [INFO] Received data message callback, size 4120.
67 14047 [OTA Agent Task] [INFO] Received valid file block: Block index=0, Size=4096
68 14059 [OTA Agent Task] [INFO] Number of blocks remaining: 278

...

3487 567949 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor.
3488 568137 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor to broker.
3489 568166 [OTA Agent Task] [INFO] Published to MQTT topic to request the next block: topic=$aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor
3490 568198 [OTA Agent Task] [INFO] Current State=[WaitingForFileBlock], Event=[RequestFileBlock], New state=[WaitingForFileBlock]
3491 569141 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
3492 569156 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
3493 569168 [MQTT Agent Task] [INFO] Received data message callback, size 4122.
3494 569179 [OTA Agent Task] [INFO] Received valid file block: Block index=277, Size=4096
3495 569192 [OTA Agent Task] [INFO] Number of blocks remaining: 1
3496 569201 [OTA Agent Task] [INFO] OTA active state `6` from OTA Agent.
3497 569212 [OTA Agent Task] [INFO] Signal task inference stop
3498 569221 [OTA Agent Task] [INFO] Current State=[WaitingForFileBlock], Event=[ReceivedFileBlock], New state=[WaitingForFileBlock]
3499 569872 [OTA Task ] [INFO]  Received: 278   Queued: 278   Processed: 278   Dropped: 0
3500 569983 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor.
3501 570169 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor to broker.
3502 570198 [OTA Agent Task] [INFO] Published to MQTT topic to request the next block: topic=$aws/things/<mqtt-client-identifier>/streams/AFR_OTA-eac26517-1848-4705-8242-97bda52cab40/get/cbor
3503 570230 [OTA Agent Task] [INFO] Current State=[WaitingForFileBlock], Event=[RequestFileBlock], New state=[WaitingForFileBlock]
3504 571172 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
3505 571187 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
3506 571199 [MQTT Agent Task] [INFO] Received data message callback, size 2979.
3507 571210 [OTA Agent Task] [INFO] Received valid file block: Block index=278, Size=2953
3508 571223 [OTA Agent Task] [INFO] Received final block of the update.
3509 571894 [OTA Agent Task] [INFO] Received entire update and validated the signature.
3510 572014 [MQTT Agent Task] [INFO] Publishing message to $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a/update.
3511 573260 [MQTT Agent Task] [INFO] Ack packet deserialized with result: MQTTSuccess.
3512 573272 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
3513 573284 [MQTT Agent Task] [INFO] De-serialized incoming PUBLISH packet: DeserializerResult=MQTTSuccess.
3514 573299 [MQTT Agent Task] [INFO] State record updated. New state=MQTTPublishDone.
3515 573312 [OTA Agent Task] [INFO] Sent PUBLISH packet to broker $aws/things/<mqtt-client-identifier>/jobs/AFR_OTA-ota-test-update-id-9ba0432f-e481-4fde-b3e5-a45bcf076e7a/update to broker.
3516 573343 [OTA Agent Task] [INFO] Received OtaJobEventActivate callback from OTA Agent.
```
