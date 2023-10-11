# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

import os
import sys
import time
import traceback as tb
import boto3
from boto3_type_annotations.iot import Client as IoTClient
from boto3_type_annotations.s3 import Client as S3Client
from typing import Callable

# Validate all expected environment variables
AWS_REGION = os.getenv("AWS_REGION")
if not AWS_REGION:
    raise ValueError("AWS_REGION is not set in environment")

OTA_ROLE_NAME = os.getenv("IOT_OTA_ROLE_NAME")
if not OTA_ROLE_NAME:
    raise ValueError("OTA_ROLE_NAME is not set in environment")

OTA_CERT_ID = os.getenv("IOT_OTA_CERT_ID")
if not OTA_CERT_ID:
    raise ValueError("OTA_CERT_ID is not set in environment")

# Constant used to register OTA
THING_NAME_PREFIX = "iotmsw-ci-test-thing-"
OTA_NAME_PREFIX = "ota-test-update-id-"
OTA_S3_BUCKET_PREFIX = "iotmsw-ci-test-bucket-"
OTA_JOB_PREFIX = "AFR_OTA-"
AWS_ACCOUNT = boto3.client("sts").get_caller_identity().get("Account")

# Create the AWS clients (iot and S3)
session = boto3.Session()
iot: IoTClient = boto3.client("iot", AWS_REGION)
s3: S3Client = boto3.client("s3", AWS_REGION)


def print_ex(ex):
    x = tb.extract_stack()[1]
    print(f"{x.filename}:{x.lineno}:{x.name}: {ex}", file=sys.stderr)


# Simplify processing list of resources with AWS. The processing function accepts
# The nextToken to use to fetch the list of resources and returns the nextToken from the
# response
def process_resources(resource_name: str, processing_fn: Callable[[str], str]) -> None:
    print(f"Processing {resource_name}")
    nextToken = ""
    while True:
        nextToken = processing_fn(nextToken)
        if nextToken == "":
            break


def process_updates(nextToken: str) -> str:
    response = iot.list_ota_updates(nextToken=nextToken)

    for ota_update in response["otaUpdates"]:
        update_id: str = ota_update["otaUpdateId"]
        if update_id.startswith(OTA_NAME_PREFIX):
            try:
                ota_response = iot.get_ota_update(otaUpdateId=update_id)
                if (
                    ota_response["otaUpdateInfo"]["otaUpdateStatus"]
                    == "DELETE_IN_PROGRESS"
                ):
                    print(f"Skip ota job {update_id}, delete in progress")
                else:
                    iot.delete_ota_update(
                        otaUpdateId=update_id, forceDeleteAWSJob=True, deleteStream=True
                    )
            except Exception as ex:
                print_ex(ex)
            else:
                print(f"Deleted {update_id}")
    return response.get("nextToken", "")


def process_jobs(nextToken: str) -> str:
    # Restrict response to 10 results as we cannot process more in one go
    response = iot.list_jobs(nextToken=nextToken, maxResults=10)

    if response["jobs"] == []:
        return ""

    for job in response["jobs"]:
        job_id: str = job["jobId"]
        if job_id.startswith(OTA_JOB_PREFIX + OTA_NAME_PREFIX):
            if job["status"] == "DELETION_IN_PROGRESS":
                print(f"Skiping Job {job_id}, deletion in progress")
            else:
                try:
                    iot.delete_job(jobId=job_id, force=True)
                except Exception as ex:
                    print_ex(ex)
                else:
                    print(f"Deleted {job_id}")
    # It takes about 40 seconds to delete 10 jobs
    time.sleep(40)
    return response.get("nextToken", "")


def process_streams(nextToken: str) -> str:
    response = iot.list_streams(nextToken=nextToken)
    for stream in response["streams"]:
        stream_name: str = stream["streamId"]
        if stream_name.startswith(OTA_JOB_PREFIX):
            try:
                iot.delete_stream(streamId=stream_name)
            except Exception as ex:
                print_ex(ex)
            else:
                print(f"Deleted stream {stream_name}")
    return response.get("nextToken", "")


def process_things(nextToken: str) -> str:
    response = iot.list_things(nextToken=nextToken)
    for thing in response["things"]:
        thing_name: str = thing["thingName"]
        if thing_name.startswith(THING_NAME_PREFIX):
            try:
                ota_principal = (
                    f"arn:aws:iot:{AWS_REGION}:{AWS_ACCOUNT}:cert/{OTA_CERT_ID}"
                )
                iot.detach_thing_principal(
                    thingName=thing_name, principal=ota_principal
                )
                iot.delete_thing(thingName=thing_name)
            except Exception as ex:
                print_ex(ex)
            else:
                print(f"Deleted {thing_name}")
    return response.get("nextToken", "")


def process_buckets(nextToken: str) -> str:
    response = s3.list_buckets(nextToken=nextToken)

    for bucket in response["Buckets"]:
        bucket_name: str = bucket["Name"]
        if bucket_name.startswith(OTA_S3_BUCKET_PREFIX):
            try:
                delete_all_bucket_objects(bucket_name)
            except Exception as e:
                raise Exception(
                    f"Failed to delete all objects from bucket `{bucket_name}`: {e}"
                )

            print(f"Deleted all objects from {bucket_name}")

            try:
                s3.delete_bucket(Bucket=bucket_name)
            except Exception as e:
                raise Exception(f"Failed to delete bucket `{bucket_name}`: {e}")

            print(f"Deleted S3 bucket {bucket_name}")

    return response.get("nextToken", "")


def delete_all_bucket_objects(bucket_name: str) -> None:
    """
    Delete all S3 bucket objects.

    Raises:
        Exception: If any of the S3 bucket deletions fail.
    """
    try:
        response = s3.list_objects_v2(Bucket=bucket_name)

        while True:
            for obj in response.get("Contents", []):
                s3.delete_object(Bucket=bucket_name, Key=obj["Key"])

            if not response.get("IsTruncated", False):
                break

            continuation_token = response.get("NextContinuationToken")

            response = s3.list_objects_v2(
                Bucket=bucket_name, ContinuationToken=continuation_token
            )
    except Exception as e:
        raise Exception(
            f"Failed to delete all objects from bucket `{bucket_name}`: {e}"
        )


process_resources("Update", process_updates)
process_resources("Stream", process_streams)
process_resources("Jobs", process_jobs)
process_resources("Things", process_things)
process_resources("Bucket", process_buckets)
