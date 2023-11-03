#!/bin/bash

# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Build an example.

NAME="$(basename "$0")"
HERE="$(dirname "$0")"
ROOT="$(realpath $HERE/../..)"
EXAMPLE=""
CLEAN=0
BUILD_PATH="$ROOT/build"
TARGET="corstone310"
TARGET_PROCESSOR=""
TOOLCHAIN="ARMCLANG"
TOOLCHAIN_FILE=""
BUILD=1
INTEGRATION_TESTS=0
CERTIFICATE_PATH=""
PRIVATE_KEY_PATH=""

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
        cmake \
        -G Ninja --toolchain=$TOOLCHAIN_FILE \
        -B $BUILD_PATH \
        -S $PATH_TO_SOURCE \
        -DCMAKE_SYSTEM_PROCESSOR=$TARGET_PROCESSOR \
        -DARM_CORSTONE_BSP_TARGET_PLATFORM=$TARGET \
        -DAWS_CLIENT_PRIVATE_KEY_PEM_PATH=$PRIVATE_KEY_PATH \
        -DAWS_CLIENT_CERTIFICATE_PEM_PATH=$CERTIFICATE_PATH \
        -DIOT_REFERENCE_ARM_CORSTONE3XX_SOURCE_DIR=$ROOT


        if [[ $BUILD -ne 0 && $INTEGRATION_TESTS -eq 0 ]]; then
            echo "Building $EXAMPLE" >&2
            cmake --build $BUILD_PATH --target "$EXAMPLE"
        elif [[ $BUILD -ne 0 && $INTEGRATION_TESTS -eq 1 ]]; then
            echo "Building $EXAMPLE-tests" >&2
            cmake --build $BUILD_PATH --target "$EXAMPLE-tests"
        fi
    )
}

function show_usage {
    cat <<EOF
Usage: $0 [options] example

Download dependencies, apply patches and build an example.

Options:
    -h,--help                   Show this help
    -c,--clean                  Clean build
    -t,--target                 Build target (corstone300 or corstone310)
    --toolchain                 Compiler (GNU or ARMCLANG)
    -q, --integration-tests     Build FreeRTOS integration tests
    --configure-only Create build tree but do not build
    --certificate_path          The full path for the AWS device certificate
    --private_key_path          The full path for the AWS device private key
Examples:
    blinky, aws-iot-example
EOF
}

if [[ $# -eq 0 ]]; then
    show_usage >&2
    exit 1
fi

SHORT=t:,c,h,q
LONG=target:,toolchain:,clean,help,configure-only,certificate_path:,private_key_path:,integration-tests
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
    -t | --target )
      TARGET=$2
      shift 2
      ;;
    --toolchain )
      TOOLCHAIN=$2
      shift 2
      ;;
    --certificate_path )
      CERTIFICATE_PATH=$2
      shift 2
      ;;
    --private_key_path )
      PRIVATE_KEY_PATH=$2
      shift 2
      ;;
    -q | --integration-tests )
      INTEGRATION_TESTS=1
      shift
      ;;
    --configure-only )
      BUILD=0
      shift
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
    aws-iot-example)
        EXAMPLE="$1"
        PATH_TO_SOURCE="$ROOT/applications/aws-iot-example"
        ;;
    *)
        echo "Missing example <blinky,aws-iot-example>"
        show_usage
        exit 2
        ;;
esac

case "$TARGET" in
    corstone300 )
      TARGET_PROCESSOR="cortex-m55"
      ;;
    corstone310 )
      TARGET_PROCESSOR="cortex-m85"
      ;;
    *)
      echo "Invalid target <corstone300|corstone310>"
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

if [ ! -f "$CERTIFICATE_PATH" ]; then
    echo "The --certificate_path must be set to an existing file."
    show_usage
    exit 2
fi

if [ ! -f "$PRIVATE_KEY_PATH" ]; then
    echo "The --private_key_path must be set to an existing file."
    show_usage
    exit 2
fi

build_with_cmake
