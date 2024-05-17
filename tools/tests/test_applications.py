# Copyright 2023-2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

from timeit import default_timer as timer
from pytest import fixture
import subprocess
from aws_test_util import Flags, create_aws_resources, cleanup_aws_resources


@fixture(scope="function")
def setup_resources(build_artefacts_path, credentials_path, signed_update_bin_name):
    """
    Setup resources needed to run the test.

    build_artefacts_path: Path to all the artefacts needed to create AWS resources.
    credentials_path: Path to AWS credentials.
    signed_update_bin_name: Name of the binary to be used for the OTA update.
    """

    flags = Flags(build_artefacts_path, credentials_path, signed_update_bin_name)
    flags = create_aws_resources(flags)
    try:
        # Caller won't actually do anything with this, but we have to yield something.
        yield flags
    finally:
        cleanup_aws_resources(flags)


def test_applications(
    setup_resources,
    fvp_process: subprocess.Popen,
    pass_output_file: str,
    fail_output_file: str,
    timeout_seconds: int,
) -> None:
    """
    Compare the actual output on the FVP with the expectations in
    pass and fail output files for the applications.

    setup_resources: Input coming out as a result of executing of setup_resources
                  function defined above.
    fvp_process (subprocess.Popen): FVP execution process
    pass_output_file (str): Path to the file containing the output when application
                            runs without errors.
    fail_output_file (str): Path to the file containing the output when application
                            runs with errors.
    timeout_seconds (int): Timeout in seconds to wait before terminating the test.
    """
    with open(pass_output_file, "r") as file:
        pass_output = file.readlines()
        pass_output = [line.replace("\n", "") for line in pass_output]
    with open(fail_output_file, "r") as file:
        fail_output = file.readlines()
        fail_output = [line.replace("\n", "") for line in fail_output]

    index = 0
    start = timer()
    current_time = timer()

    while (current_time - start) < (timeout_seconds):
        line = fvp_process.stdout.readline()
        if not line:
            break
        line = line.decode("utf-8", errors="ignore").rstrip()
        print(line)
        if pass_output[index] in line:
            index += 1
            if index == len(pass_output):
                break
        for x in fail_output:
            if x in line:
                break
        current_time = timer()

    assert index == len(pass_output)
