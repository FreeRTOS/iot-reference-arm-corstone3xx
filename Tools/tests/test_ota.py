# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

from timeit import default_timer as timer
from pytest import fixture
from aws_test_util import Flags, create_aws_resources, cleanup_aws_resources


@fixture(scope="function")
def aws_resources(build_path, credentials_path):
    flags = Flags(build_path, credentials_path)
    flags = create_aws_resources(flags)
    try:
        # Caller won't actually do anything with this, but we have to yield something.
        yield flags
    finally:
        cleanup_aws_resources(flags)


def test_ota(aws_resources, fvp):
    # Traces expected in the output
    expectations = [
        "Application version from appFirmwareVersion 0.0.1",
        "Application version from appFirmwareVersion 0.0.2",
    ]

    fails = [
        "Failed to provision device private key",
        "Failed job document content check",
        "Failed to execute state transition handler",
        "Failed to send blink_event message to ui_msg_queue",
    ]

    index = 0
    start = timer()
    current_time = timer()

    # Timeout for the test is 15 minutes
    while (current_time - start) < (15 * 60):
        line = fvp.stdout.readline()
        if not line:
            break
        line = line.decode("utf-8")
        line = line.rstrip()
        print(line)
        if expectations[index] in line:
            index += 1
            if index == len(expectations):
                break
        for x in fails:
            assert x not in line
        current_time = timer()

    assert index == len(expectations)
