#!/bin/bash

# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

HERE="$(dirname "$0")"
ROOT="$(realpath $HERE/../..)"
EXAMPLE=""
BUILD_PATH="build"
TARGET="corstone315"
AUDIO_SOURCE="ROM"
NPU_ID="U65"
FVP_BIN="FVP_Corstone_SSE-315"
FRAMES=""

set -e

function show_usage {
    cat <<EOF
Usage: $0 [options] example

Run an example.

Options:
    -h,--help   Show this help
    -p,--path   Build path
    -t,--target Target to run
    -s, --audio Audio source (ROM, VSI)
    -n, --npu-id  NPU ID to use (U55, U65)

Examples:
    blinky, keyword-detection, speech-recognition
EOF
}

SHORT=t:,n:,s:,h,p:
LONG=target:,npu-id:,audio:,help,path:
OPTS=$(getopt -n run --options $SHORT --longoptions $LONG -- "$@")

eval set -- "$OPTS"

while :
do
  case "$1" in
    -h | --help )
      show_usage
      exit 0
      ;;
    -p | --path )
      BUILD_PATH=$2
      shift 2
      ;;
    -t | --target )
      TARGET=$2
      shift 2
      ;;
    -n | --npu-id )
      NPU_ID=$2
      shift 2
      ;;
    -s | --audio )
      AUDIO_SOURCE=$2
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
      if [ "$NPU_ID" == "" ] || [ "$NPU_ID" == "U55" ]; then
        FVP_BIN="FVP_Corstone_SSE-300_Ethos-U55"
      elif [ "$NPU_ID" == "U65" ]; then
        FVP_BIN="FVP_Corstone_SSE-300_Ethos-U65"
      else
        echo "Invalid NPU ID <U55|U65>"
        show_usage
        exit 2
      fi
      ;;
    corstone310 )
      if [ "$NPU_ID" == "" ] || [ "$NPU_ID" == "U55" ]; then
        FVP_BIN="FVP_Corstone_SSE-310"
      elif [ "$NPU_ID" == "U65" ]; then
        FVP_BIN="FVP_Corstone_SSE-310_Ethos-U65"
      else
        echo "Invalid NPU ID <U55|U65>"
        show_usage
        exit 2
      fi
      ;;
    corstone315 )
      FVP_BIN="FVP_Corstone_SSE-315"
      ;;
    *)
      echo "Invalid target <corstone300 | corstone310 | corstone315>"
      show_usage
      exit 2
      ;;
esac

case "$1" in
    blinky)
        EXAMPLE="$1"
        MERGED_IMAGE_PATH="$BUILD_PATH/blinky_merged.elf"
        ;;
    keyword-detection)
        EXAMPLE="$1"
        MERGED_IMAGE_PATH="$BUILD_PATH/keyword-detection_merged.elf"
        ;;
    speech-recognition)
        EXAMPLE="$1"
        MERGED_IMAGE_PATH="$BUILD_PATH/speech-recognition_merged.elf"
        ;;
    *)
        echo "Usage: $0 <blinky,keyword-detection,speech-recognition>" >&2
        exit 1
        ;;
esac

case "$AUDIO_SOURCE" in
    ROM )
      ;;
    VSI )
        if [ "$EXAMPLE" == "speech-recognition" ] || [ "$EXAMPLE" == "keyword-detection" ]; then
            export APP_UNDERSCORED=$(echo ${EXAMPLE} | tr '-' '_')
            export AVH_AUDIO_FILE=$ROOT/applications/$APP_UNDERSCORED/resources/test.wav
        fi
        AVH_AUDIO_OPTIONS="-C mps3_board.v_path=$ROOT/tools/scripts/"
        export PYTHONHOME="/opt/python/3.9.18"
        ;;
    *)
      echo "Invalid audio source <ROM | VSI>"
      show_usage
      exit 2
      ;;
esac

case "$TARGET" in
    corstone300 | corstone310 )
      OPTIONS="-C mps3_board.visualisation.disable-visualisation=1 \
      -C core_clk.mul=200000000 \
      -C mps3_board.smsc_91c111.enabled=1 \
      -C mps3_board.hostbridge.userNetworking=1 \
      -C mps3_board.telnetterminal0.start_telnet=0 \
      -C mps3_board.uart0.out_file="-" \
      -C mps3_board.uart0.unbuffered_output=1 \
      -C ethosu.extra_args="--fast" \
      --stat \
      -C mps3_board.DISABLE_GATING=1"
      ;;
    corstone315 )
      OPTIONS="-C mps4_board.visualisation.disable-visualisation=1 \
      -C core_clk.mul=200000000 \
      -C mps4_board.smsc_91c111.enabled=1 \
      -C mps4_board.hostbridge.userNetworking=1 \
      -C mps4_board.telnetterminal0.start_telnet=0 \
      -C mps4_board.uart0.out_file="-" \
      -C mps4_board.uart0.unbuffered_output=1 \
      -C mps4_board.subsystem.ethosu.extra_args="--fast" \
      --stat"
      ;;
esac

# Start the FVP
$FVP_BIN -a $MERGED_IMAGE_PATH $OPTIONS $AVH_AUDIO_OPTIONS
