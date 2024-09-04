#!/bin/bash

# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

HERE="$(dirname "$0")"
ROOT="$(realpath $HERE/../..)"
EXAMPLE=""
BUILD_PATH="$(realpath $ROOT/build)"
TARGET="corstone315"
AUDIO_SOURCE="ROM"
NPU_ID=""
FVP_BIN="FVP_Corstone_SSE-315"
FRAMES=""
OPTIONS=""
DISPLAY=true
DISPLAY_OPTIONS=""

set -e

function show_usage {
    cat <<EOF
Usage: $0 [options] example

Run an example.

Options:
    -h,--help       Show this help
    -p,--path       Build path
    -t,--target     Target to run
    -s,--audio      Audio source (ROM, VSI)
    -n,--npu-id     NPU ID to use (U55, U65)
    -f,--frames     Path to camera frames for the ISP to stream
    -d,--debug      Path to debugger plugin to start FVP
    -D,--display    Is display (GUI) available

Examples:
    blinky, keyword-detection, speech-recognition, object-detection
EOF
}

SHORT=t:,n:,s:,h,p:,f:,d:,D:
LONG=target:,npu-id:,audio:,help,path:,frames:,debug:,display:
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
      BUILD_PATH=$(realpath "$2")
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
    -f | --frames )
      FRAMES=$(realpath "$2")
      shift 2
      ;;
    -d | --debug )
      GDB_PLUGIN=$(realpath "$2")
      if [ ! -f $GDB_PLUGIN ]; then
        echo "Error: Please provide the path to a `GDBRemoteConnection.so` file"
        exit 2
      fi
      shift 2
      ;;
    -D | --display )
      DISPLAY=$2
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
    object-detection)
        EXAMPLE="$1"
        MERGED_IMAGE_PATH="$BUILD_PATH/object-detection_merged.elf"
        ;;
    *)
        show_usage
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
        case "$TARGET" in
            corstone300 | corstone310 )
              AVH_AUDIO_OPTIONS="-C mps3_board.v_path=$ROOT/tools/scripts/"
              ;;
            corstone315 )
              AVH_AUDIO_OPTIONS="-C mps4_board.v_path=$ROOT/tools/scripts/"
              ;;
        esac
        export PYTHONHOME="/opt/python/3.9.18"
        ;;
    *)
      echo "Invalid audio source <ROM | VSI>"
      show_usage
      exit 2
      ;;
esac

if [ -f "$GDB_PLUGIN" ]; then
  OPTIONS="--allow-debug-plugin --plugin $GDB_PLUGIN"
fi

if [ "$DISPLAY" = false ]; then
  DISPLAY_OPTIONS="-C vis_hdlcd.disable_visualisation=1"
fi

case "$TARGET" in
    corstone300 | corstone310 )
      OPTIONS="$OPTIONS \
      -C mps3_board.visualisation.disable-visualisation=1 \
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
      OPTIONS="$OPTIONS $DISPLAY_OPTIONS \
      -C mps4_board.visualisation.disable-visualisation=1 \
      -C core_clk.mul=200000000 \
      -C mps4_board.smsc_91c111.enabled=1 \
      -C mps4_board.hostbridge.userNetworking=1 \
      -C mps4_board.telnetterminal0.start_telnet=0 \
      -C mps4_board.uart0.out_file="-" \
      -C mps4_board.uart0.unbuffered_output=1 \
      -C mps4_board.subsystem.ethosu.extra_args="--fast" \
      -C mps4_board.isp_c55_capture_ds.do_capture=0 \
      -C mps4_board.isp_c55_capture_fr.do_capture=0 \
      -C mps4_board.isp_c55_camera.image_file=${FRAMES} \
      --stat"
      ;;
esac

if [ "$EXAMPLE" == "object-detection" ] && [ "$TARGET" != "corstone315" ]; then
    echo "Error: Invalid combination of example and target. object-detection only supports corstone315" >&2
    exit 2
fi

# Start the FVP
$FVP_BIN -a $MERGED_IMAGE_PATH $OPTIONS $AVH_AUDIO_OPTIONS
