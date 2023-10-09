#!/bin/bash

# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

HERE="$(dirname "$0")"
ROOT="$(realpath $HERE/..)"
EXAMPLE=""
BUILD_PATH="build"
TARGET="corstone310"
FVP_TYPE="fvp"
FVP_BIN=""

set -e

function show_usage {
    cat <<EOF
Usage: $0 [options] example

Run an example.

Options:
    -h,--help   Show this help
    -p,--path   Build path
    -t,--target Target to run
    -f, --fvp_type  FVP type to use (fvp, vht)

Examples:
    blinky, aws-iot-example
EOF
}

SHORT=t:,f:,h
LONG=target:,fvp_type:,help
OPTS=$(getopt -n run --options $SHORT --longoptions $LONG -- "$@")

eval set -- "$OPTS"

while :
do
  case "$1" in
    -h | --help )
      show_usage
      exit 0
      ;;
    -t | --target )
      TARGET=$2
      shift 2
      ;;
    -f | --fvp_type )
      FVP_TYPE=$2
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

case "$TARGET" in
    corstone300 )
      if [ "$FVP_TYPE" == "fvp" ]; then
        FVP_BIN="FVP_Corstone_SSE-300_Ethos-U55"
      elif [ "$FVP_TYPE" == "vht" ]; then
        FVP_BIN="VHT_Corstone_SSE-300_Ethos-U55"
      else
        echo "Invalid FVP type <fvp|vht>"
        show_usage
        exit 2
      fi
      ;;
    corstone310 )
      if [ "$FVP_TYPE" == "fvp" ]; then
        FVP_BIN="FVP_Corstone_SSE-310"
      elif [ "$FVP_TYPE" == "vht" ]; then
        FVP_BIN="VHT_Corstone_SSE-310"
      else
        echo "Invalid FVP type <fvp|vht>"
        show_usage
        exit 2
      fi
      ;;
    *)
      echo "Invalid target <corstone300|corstone310>"
      show_usage
      exit 2
      ;;
esac

case "$1" in
    blinky)
        EXAMPLE="$1"
        MERGED_IMAGE_PATH="$BUILD_PATH/blinky_merged.elf"
        ;;
    aws-iot-example)
        EXAMPLE="$1"
        MERGED_IMAGE_PATH="$BUILD_PATH/aws-iot-example_merged.elf"
        ;;
    *)
        echo "Usage: $0 <blinky,aws-iot-example>" >&2
        exit 1
        ;;
esac

OPTIONS="-C mps3_board.visualisation.disable-visualisation=1 -C mps3_board.smsc_91c111.enabled=1 -C mps3_board.hostbridge.userNetworking=1 -C mps3_board.telnetterminal0.start_telnet=0 -C mps3_board.uart0.out_file="-"  -C mps3_board.uart0.unbuffered_output=1 --stat  -C mps3_board.DISABLE_GATING=1"

# Start the FVP
$FVP_BIN $OPTIONS -a $MERGED_IMAGE_PATH
