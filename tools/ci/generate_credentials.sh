#!/bin/bash

# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

NAME="$(basename "$0")"
HERE="$(dirname "$0")"
ROOT="$(realpath $HERE/../..)"
FORCE=0
OUTPUT_PATH="$ROOT/credentials"

set -e

function show_usage {
    cat <<EOF
Usage: $0 [options]

Generate aws credentials

Options:
    -h,--help   Show this help
    -f,--force  Regenerate credentials
    -p,--path Path of the credentials
EOF
}

SHORT=p:,f,h
LONG=path:,force,help
OPTS=$(getopt -n generate_credentials --options $SHORT --longoptions $LONG -- "$@")

eval set -- "$OPTS"

while :
do
  case "$1" in
    -h | --help )
      show_usage
      exit 0
      ;;
    -f | --force )
      FORCE=1
      shift
      ;;
    -p | --path )
      OUTPUT_PATH=$ROOT/$2
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

# Create credentials path
mkdir -p $OUTPUT_PATH

TEST_ID_PATH="$OUTPUT_PATH/test-id.txt"

# Create random test ID.
if [[ -f $TEST_ID_PATH && "$FORCE" == "0" ]]; then
    TEST_ID=`cat $TEST_ID_PATH`
    echo "Reusing test ID $TEST_ID" >&2
else
    TEST_ID=$(uuidgen)
    echo -n $TEST_ID >$TEST_ID_PATH
    echo "Generated test ID $TEST_ID" >&2
fi

# Generate AWS client credential configs for OTA test.
if [[ ! -z "$IOT_OTA_ENDPOINT" && ! -z "$IOT_OTA_CLIENT_CERT" && ! -z "$IOT_OTA_CLIENT_PRIV" && ! -z "$IOT_OTA_CLIENT_PUB" ]]; then
    cat >"$OUTPUT_PATH/aws_clientcredential.h" <<EOF
#ifndef __AWS_CLIENTCREDENTIAL__H__
#define __AWS_CLIENTCREDENTIAL__H__
#define clientcredentialMQTT_BROKER_ENDPOINT         "$IOT_OTA_ENDPOINT"
#define clientcredentialIOT_THING_NAME               "iotmsw-ci-test-thing-$TEST_ID"
#define clientcredentialMQTT_BROKER_PORT             8883
#define clientcredentialGREENGRASS_DISCOVERY_PORT    8443
#define clientcredentialWIFI_SSID                    ""
#define clientcredentialWIFI_PASSWORD                ""
#define clientcredentialWIFI_SECURITY                eWiFiSecurityWPA2
#endif
EOF
    cat >"private_key.pem" <<EOF
$IOT_OTA_CLIENT_PRIV
EOF
    cat >"certificate.pem" <<EOF
$IOT_OTA_CLIENT_CERT
EOF
fi
