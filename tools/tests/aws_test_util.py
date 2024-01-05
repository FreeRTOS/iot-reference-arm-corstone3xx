# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

import os
from pathlib import Path
import sys
import time
import traceback as tb

import boto3

AWS_REGION = os.getenv("AWS_REGION")
OTA_ROLE_NAME = os.getenv("IOT_OTA_ROLE_NAME")
OTA_CERT_ID = os.getenv("IOT_OTA_CERT_ID")

if not AWS_REGION:
    raise ValueError("AWS_REGION is not set in environment")

session = boto3.Session()
iot = boto3.client("iot", AWS_REGION)
s3 = boto3.client("s3", AWS_REGION)

if not OTA_ROLE_NAME:
    raise ValueError("OTA_ROLE_NAME is not set in environment")

if not OTA_CERT_ID:
    raise ValueError("OTA_CERT_ID is not set in environment")


def read_whole_file(path, mode="r"):
    with open(path, mode) as fp:
        return fp.read()


class Flags:
    def __init__(self, build_artefacts_path, credentials_dir, signed_update_bin_name):
        self.BUILD_ARTEFACTS_PATH = build_artefacts_path
        self.TEST_ID = read_whole_file(Path(credentials_dir) / "test-id.txt").strip()
        self.AWS_ACCOUNT = boto3.client("sts").get_caller_identity().get("Account")
        self.OTA_THING_NAME = "iotmsw-ci-test-thing-" + self.TEST_ID
        self.OTA_S3_BUCKET = "iotmsw-ci-test-bucket-" + self.TEST_ID
        self.OTA_POLICY_NAME = "iotmsw-ci-test-policy-" + self.TEST_ID
        self.OTA_BINARY = signed_update_bin_name
        self.OTA_BINARY_PATH = Path(self.BUILD_ARTEFACTS_PATH) / self.OTA_BINARY
        self.OTA_ROLE_ARN = f"arn:aws:iam::{self.AWS_ACCOUNT}:role/{OTA_ROLE_NAME}"
        self.OTA_UPDATE_PROTOCOLS = ["MQTT"]
        self.OTA_UPDATE_TARGET_SELECTION = "SNAPSHOT"
        self.OTA_UPDATE_FILES = [
            {
                "fileName": "non_secure image",
                "fileLocation": {
                    "s3Location": {"bucket": self.OTA_S3_BUCKET, "key": self.OTA_BINARY}
                },
                "codeSigning": {
                    "customCodeSigning": {
                        "signature": {
                            "inlineDocument": bytearray(
                                read_whole_file(
                                    (
                                        Path(self.BUILD_ARTEFACTS_PATH)
                                        / "update-signature.txt"
                                    )
                                ).strip(),
                                "utf-8",
                            )
                        },
                        "certificateChain": {"certificateName": "0"},
                        "hashAlgorithm": "SHA256",
                        "signatureAlgorithm": "RSA",
                    },
                },
            }
        ]
        self.bucket = None
        self.file = None
        self.thing = None
        self.update = None


def print_ex(ex):
    x = tb.extract_stack()[1]
    print(f"{x.filename}:{x.lineno}:{x.name}: {ex}", file=sys.stderr)


def wait_for_status(id, action):
    success_status = action.upper() + "_COMPLETE"
    failure_status = action.upper() + "_FAILED"
    while True:
        res = None
        try:
            res = iot.get_ota_update(otaUpdateId=id)
        except Exception as e:
            print(e)
            break
        else:
            status = res["otaUpdateInfo"]["otaUpdateStatus"]
            if status == success_status:
                break
            elif status == failure_status:
                raise ValueError(
                    "Failed to {} OTA update: {}".format(
                        action, res["otaUpdateInfo"]["errorInfo"]["message"]
                    )
                )
            print("OTA update status:", status)
            time.sleep(5)


def create_aws_resources(flags: Flags):
    try:
        # Create test bucket.
        flags.bucket = flags.OTA_S3_BUCKET
        s3.create_bucket(
            Bucket=flags.OTA_S3_BUCKET,
            ACL="private",
            CreateBucketConfiguration={"LocationConstraint": AWS_REGION},
        )
        print("Created new S3 bucket", flags.OTA_S3_BUCKET)

        # Push firmware to test bucket
        flags.file = open(flags.OTA_BINARY_PATH, "rb")
        s3.put_object(Bucket=flags.OTA_S3_BUCKET, Key=flags.OTA_BINARY, Body=flags.file)
        flags.file.close()
        print(
            "Added",
            flags.OTA_BINARY,
            "to S3 bucket",
            flags.OTA_S3_BUCKET,
            file=sys.stderr,
        )

        # Create test thing with policy attached.
        flags.thing = iot.create_thing(thingName=flags.OTA_THING_NAME)["thingArn"]
        ota_principal = (
            f"arn:aws:iot:{AWS_REGION}:{flags.AWS_ACCOUNT}:cert/{OTA_CERT_ID}"
        )
        iot.attach_thing_principal(
            thingName=flags.OTA_THING_NAME,
            principal=ota_principal,
        )
        print("Created OTA thing", flags.OTA_THING_NAME)

        # Create update
        flags.update = "ota-test-update-id-" + flags.TEST_ID
        iot.create_ota_update(
            otaUpdateId=flags.update,
            targets=[flags.thing],
            protocols=flags.OTA_UPDATE_PROTOCOLS,
            targetSelection=flags.OTA_UPDATE_TARGET_SELECTION,
            files=flags.OTA_UPDATE_FILES,
            roleArn=flags.OTA_ROLE_ARN,
        )
        wait_for_status(flags.update, "create")
        print("Created update", flags.update, file=sys.stderr)
        return flags
    except Exception as e:
        cleanup_aws_resources(flags)
        raise e


def cleanup_aws_resources(flags: Flags):
    if flags.update:
        try:
            iot.delete_ota_update(
                otaUpdateId=flags.update, forceDeleteAWSJob=True, deleteStream=True
            )
            wait_for_status(flags.update, "delete")
        except Exception as ex:
            print_ex(ex)
        else:
            print("Deleted update", flags.update, file=sys.stderr)
            flags.update = None
    if flags.thing:
        ota_principal = (
            f"arn:aws:iot:{AWS_REGION}:{flags.AWS_ACCOUNT}:cert/{OTA_CERT_ID}"
        )
        try:
            iot.detach_thing_principal(
                thingName=flags.OTA_THING_NAME,
                principal=ota_principal,
            )
            iot.delete_thing(thingName=flags.OTA_THING_NAME)
        except Exception as ex:
            print_ex(ex)
        else:
            print("Deleted thing", flags.thing, file=sys.stderr)
            flags.thing = None
    if flags.file:
        try:
            s3.delete_object(Bucket=flags.OTA_S3_BUCKET, Key=flags.OTA_BINARY)
        except Exception as ex:
            print_ex(ex)
        else:
            print(
                "Deleted S3 object {} from {}".format(
                    flags.OTA_BINARY, flags.OTA_S3_BUCKET
                ),
                file=sys.stderr,
            )
            flags.file = None
    if flags.bucket:
        try:
            s3.delete_bucket(Bucket=flags.OTA_S3_BUCKET)
        except Exception as ex:
            print_ex(ex)
        else:
            print("Deleted S3 bucket", flags.OTA_S3_BUCKET, file=sys.stderr)
            flags.bucket = None
