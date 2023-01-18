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
TARGET="corstone300"
TARGET_PROCESSOR=""
BUILD=1

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
        cmake -G Ninja -S . -B $BUILD_PATH --toolchain=toolchains/toolchain-armclang.cmake -DCMAKE_SYSTEM_PROCESSOR=$TARGET_PROCESSOR -DARM_CORSTONE_BSP_TARGET_PLATFORM=$TARGET -DEXAMPLE=$EXAMPLE
        if [[ $BUILD -ne 0 ]]; then
            cmake --build $BUILD_PATH --target $EXAMPLE
        fi
    )
}

function show_usage {
    cat <<EOF
Usage: $0 [options] example

Download dependencies, apply patches and build an example.

Options:
    -h,--help        Show this help
    -c,--clean       Clean build
    -t,--target      Build target (corstone300 or corstone310)
    --configure-only Create build tree but do not build

Examples:
    blinky
EOF
}

if [[ $# -eq 0 ]]; then
    show_usage >&2
    exit 1
fi

SHORT=t:,c,h
LONG=target:,clean,help,configure-only
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
        ;;
    *)
        echo "Missing example <blinky>"
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

build_with_cmake
