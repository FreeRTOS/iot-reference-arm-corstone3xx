#!/bin/bash

# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Build an example.

NAME="$(basename "$0")"
HERE="$(dirname "$0")"
ROOT="$(realpath $HERE/../..)"
EXAMPLE=""
CLEAN=0
BUILD_PATH="$(realpath $ROOT/build)"
TARGET="corstone315"
TARGET_PROCESSOR=""
ML_INFERENCE_ENGINE="ETHOS"
ETHOS_U_NPU_ID=""
ETHOS_U_NPU_NUM_MACS=""
AUDIO_SOURCE="ROM"
TOOLCHAIN="GNU"
TOOLCHAIN_FILE=""
BUILD=1
CERTIFICATE_PATH=""
PRIVATE_KEY_PATH=""
CONNECTIVITY_STACK="FREERTOS_PLUS_TCP"
PSA_CRYPTO_IMPLEMENTATION="TF-M"

set -e

function build_with_cmake {
    CMAKE="$(which cmake)"
    if [[ ! -f "$CMAKE" ]]; then
        echo "${NAME}: cmake is not in PATH" >&2
        exit 1
    fi

    if [[ $CLEAN -ne 0 ]]; then
        echo "Clean building $EXAMPLE" >&2
        rm -rf $BUILD_PATH
    else
        echo "Building $EXAMPLE" >&2
    fi

    (
        set -ex

        # Note: A bug in CMake force us to set the toolchain here
        cmake_args=()
        cmake_args+=(-G Ninja --toolchain=$TOOLCHAIN_FILE)
        cmake_args+=(-B $BUILD_PATH)
        cmake_args+=(-S $PATH_TO_SOURCE)
        cmake_args+=(-DCMAKE_SYSTEM_PROCESSOR=$TARGET_PROCESSOR)
        cmake_args+=(-DARM_CORSTONE_BSP_TARGET_PLATFORM=$TARGET)
        cmake_args+=(-DAWS_CLIENT_PRIVATE_KEY_PEM_PATH=$PRIVATE_KEY_PATH)
        cmake_args+=(-DAWS_CLIENT_CERTIFICATE_PEM_PATH=$CERTIFICATE_PATH)
        cmake_args+=(-DIOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR=$ROOT)
        cmake_args+=(-DML_INFERENCE_ENGINE=$ML_INFERENCE_ENGINE)
        cmake_args+=(-DAUDIO_SOURCE=$AUDIO_SOURCE)
        cmake_args+=(-DCONNECTIVITY_STACK=$CONNECTIVITY_STACK)
        cmake_args+=(-DPSA_CRYPTO_IMPLEMENTATION=$PSA_CRYPTO_IMPLEMENTATION)
        if [ ! -z "$ETHOS_U_NPU_ID" ]; then
          cmake_args+=(-DETHOS_U_NPU_ID=$ETHOS_U_NPU_ID)
        else
          cmake_args+=(-UETHOS_U_NPU_ID)
        fi
        if [ ! -z "$ETHOS_U_NPU_NUM_MACS" ]; then
          cmake_args+=(-DETHOS_U_NPU_NUM_MACS=$ETHOS_U_NPU_NUM_MACS)
        else
          cmake_args+=(-UETHOS_U_NPU_NUM_MACS)
        fi

        cmake "${cmake_args[@]}"

        echo "Building $EXAMPLE" >&2
        cmake --build $BUILD_PATH --target "$EXAMPLE"
    )
}

function show_usage {
    cat <<EOF
Usage: $0 [options] example

Apply patches and build an example.

Options:
    -h,--help                   Show this help
    -p,--path                   Path to the build directory
    -c,--clean                  Clean build
    -t,--target                 Build target (corstone300 | corstone310 | corstone315)
    -i,--inference              ML Inference engine selection (ETHOS | SOFTWARE)
    -s,--audio                  Audio source (ROM | VSI)
    -n | --npu-id               Ethos NPU model identifier (U55 | U65)
    --npu-mac                   Number of 8x8 MACs performed per cycle by the NPU (32 | 64 | 128 | 256 | 512)
    -T,--toolchain                 Compiler (GNU or ARMCLANG)
    -C,--certificate_path          Path to the AWS device certificate
    -P,--private_key_path          Path to the AWS device private key
    --conn-stack                   Connectivity stack selection (FREERTOS_PLUS_TCP | IOT_VSOCKET)
    --psa-crypto-implementation    PSA Crypto APIs implementation selection (TF-M, MBEDTLS)
Examples:
    blinky, freertos-iot-libraries-tests, keyword-detection, object-detection, speech-recognition
EOF
}

if [[ $# -eq 0 ]]; then
    show_usage >&2
    exit 1
fi

SHORT=t:,i:,T:,s:,c,h,C:,P:p:,n:
LONG=target:,inference:,toolchain:,audio:,clean,help,configure-only,certificate_path:,private_key_path:,path:,npu-id:,npu-mac:,conn-stack:,psa-crypto-implementation:
OPTS=$(getopt -n build --options $SHORT --longoptions $LONG -- "$@")

eval set -- "$OPTS"

while :
do
  case "$1" in
    -h | --help )
      show_usage
      exit 0
      ;;
    -c | --clean )
      CLEAN=1
      shift
      ;;
    -p | --path )
      BUILD_PATH=$(realpath "$2")
      shift 2
      ;;
    -t | --target )
      TARGET=$2
      shift 2
      ;;
    -i | --inference )
      ML_INFERENCE_ENGINE=$2
      shift 2
      ;;
    -n | --npu-id )
      ETHOS_U_NPU_ID=$2
      shift 2
      ;;
    --npu-mac )
      ETHOS_U_NPU_NUM_MACS=$2
      shift 2
      ;;
    -s | --audio )
      AUDIO_SOURCE=$2
      shift 2
      ;;
    -T | --toolchain )
      TOOLCHAIN=$2
      shift 2
      ;;
    -C | --certificate_path )
      CERTIFICATE_PATH=$(realpath "$2")
      shift 2
      ;;
    -P | --private_key_path )
      PRIVATE_KEY_PATH=$(realpath "$2")
      shift 2
      ;;
    --conn-stack )
      CONNECTIVITY_STACK=$2
      shift 2
      ;;
    --psa-crypto-implementation )
      PSA_CRYPTO_IMPLEMENTATION=$2
      shift 2
      ;;
    --)
      shift;
      break
      ;;
    *)
      echo "Unexpected option: $1"
      show_usage
      exit 2
      ;;
  esac
done

case "$1" in
    blinky)
        EXAMPLE="$1"
        PATH_TO_SOURCE="$ROOT/applications/blinky"
        ;;
    freertos-iot-libraries-tests)
        EXAMPLE="$1"
        PATH_TO_SOURCE="$ROOT/applications/freertos_iot_libraries_tests"
        ;;
    keyword-detection)
        EXAMPLE="$1"
        PATH_TO_SOURCE="$ROOT/applications/keyword_detection"
        ;;
    speech-recognition)
        EXAMPLE="$1"
        PATH_TO_SOURCE="$ROOT/applications/speech_recognition"
        ;;
    object-detection)
        EXAMPLE="$1"
        PATH_TO_SOURCE="$ROOT/applications/object_detection"
        ;;
    *)
        echo "Missing example <blinky,freertos-iot-libraries-tests,keyword-detection,object-detection,speech-recognition>"
        show_usage
        exit 2
        ;;
esac

case "$ML_INFERENCE_ENGINE" in
    ETHOS | SOFTWARE )
        ;;
    *)
        echo "Invalid inference selection <ETHOS | SOFTWARE>"
        show_usage
        exit 2
        ;;
esac

case "$ETHOS_U_NPU_ID" in
    U55 | U65 | "" )
        ;;
    *)
        echo "Invalid NPU type <U55 | U65>"
        show_usage
        exit 2
        ;;
esac

case "$ETHOS_U_NPU_NUM_MACS" in
    32 | 64 | 128 | 256 | 512 | "" )
        ;;
    *)
        echo "Invalid NPU MAC value <32 | 64 | 128 | 256 | 512>"
        show_usage
        exit 2
        ;;
esac

case "$AUDIO_SOURCE" in
    ROM )
        ;;
    VSI )
        ;;
    *)
        echo "Invalid audio source selection <ROM | VSI>"
        show_usage
        exit 2
        ;;
esac

case "$TARGET" in
    corstone300 )
      TARGET_PROCESSOR="cortex-m55"
      ;;
    corstone310 | corstone315 )
      TARGET_PROCESSOR="cortex-m85"
      ;;
    *)
      echo "Invalid target <corstone300 | corstone310 | corstone315>"
      show_usage
      exit 2
      ;;
esac

case "$TOOLCHAIN" in
    ARMCLANG )
      TOOLCHAIN_FILE="$ROOT/components/tools/open_iot_sdk_toolchain/library/toolchain-armclang.cmake"
      ;;
    GNU )
      TOOLCHAIN_FILE="$ROOT/components/tools/open_iot_sdk_toolchain/library/toolchain-arm-none-eabi-gcc.cmake"
      ;;
    * )
      echo "Invalid toolchain <ARMCLANG|GNU>"
      show_usage
      exit 2
      ;;
esac

case "$CONNECTIVITY_STACK" in
    FREERTOS_PLUS_TCP )
        ;;
    IOT_VSOCKET )
        ;;
    *)
        echo "Invalid connectivity stack selection <FREERTOS_PLUS_TCP | IOT_VSOCKET>"
        show_usage
        exit 2
        ;;
esac

case "$PSA_CRYPTO_IMPLEMENTATION" in
    TF-M )
        ;;
    MBEDTLS )
        ;;
    *)
        echo "Invalid PSA Crypto APIs selection <TF-M | MBEDTLS>"
        show_usage
        exit 2
        ;;
esac

if [ "$EXAMPLE" != "blinky" ] && [ ! -f "$CERTIFICATE_PATH" ]; then
    echo "The --certificate_path must be set to an existing file."
    show_usage
    exit 2
fi

if [ "$EXAMPLE" != "blinky" ] && [ ! -f "$PRIVATE_KEY_PATH" ]; then
    echo "The --private_key_path must be set to an existing file."
    show_usage
    exit 2
fi

if [ "$EXAMPLE" == "object-detection" ] && [ "$TARGET" != "corstone315" ]; then
    echo "Error: Invalid combination of example and target. object-detection only supports corstone315" >&2
    exit 2
fi

build_with_cmake
