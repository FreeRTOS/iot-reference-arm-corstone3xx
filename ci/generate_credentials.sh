#!/bin/bash

# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

NAME="$(basename "$0")"
HERE="$(dirname "$0")"
ROOT="$(realpath $HERE/..)"
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
    cat >"$OUTPUT_PATH/aws_clientcredential_keys.h" <<EOF
#ifndef __AWS_CLIENTCREDENTIAL_KEYS_H__
#define __AWS_CLIENTCREDENTIAL_KEYS_H__
#define keyCLIENT_CERTIFICATE_PEM   "$IOT_OTA_CLIENT_CERT"
#define keyCLIENT_PRIVATE_KEY_PEM   "$IOT_OTA_CLIENT_PRIV"
#define keyJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM  ""
#define keyCLIENT_PUBLIC_KEY_PEM    "$IOT_OTA_CLIENT_PUB"
#define keyCA1_ROOT_CERTIFICATE_PEM "-----BEGIN CERTIFICATE-----\nMIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\nADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\nb24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\nMAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\nb3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\nca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\nIFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\nVOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\njgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\nAYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\nA4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\nU5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\nN+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\no/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\nrqXRfboQnoZsG4q5WTP468SQvvG5\n-----END CERTIFICATE-----"
#define keyCA3_ROOT_CERTIFICATE_PEM "-----BEGIN CERTIFICATE-----\nMIIBtjCCAVugAwIBAgITBmyf1XSXNmY/Owua2eiedgPySjAKBggqhkjOPQQDAjA5\nMQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24g\nUm9vdCBDQSAzMB4XDTE1MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkG\nA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJvb3Qg\nQ0EgMzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABCmXp8ZBf8ANm+gBG1bG8lKl\nui2yEujSLtf6ycXYqm0fc4E7O5hrOXwzpcVOho6AF2hiRVd9RFgdszflZwjrZt6j\nQjBAMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdDgQWBBSr\nttvXBp43rDCGB5Fwx5zEGbF4wDAKBggqhkjOPQQDAgNJADBGAiEA4IWSoxe3jfkr\nBqWTrBqYaGFy+uGh0PsceGCmQ5nFuMQCIQCcAu/xlJyzlvnrxir4tiz+OpAUFteM\nYyRIHN8wfdVoOw==\n-----END CERTIFICATE-----"
#endif
EOF
fi
