# Copyright (c) 2023-2024 Arm Limited. All rights reserved.
# SPDX-License-Identifier: MIT

import subprocess
from timeit import default_timer as timer
from pytest import fixture
from aws_test_util import Flags, create_aws_resources, cleanup_aws_resources
import re

# If you have failing test cases to ignore,
# specify the test suites and test cases as shown below.
# test_cases_to_ignore = {
#     "test_suite_1, test_case_1": "ignore_reason_1",
#     "test_suite_2, test_case_2": "ignore_reason_2",
#     "test_suite_n, test_case_n": "ignore_reason_n",
# }
test_cases_to_ignore = {}


@fixture(scope="function")
def setup_resources(
    build_artefacts_path: str, credentials_path: str, signed_update_bin_name: str
):
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


def test_integration(
    setup_resources,
    fvp_process: subprocess.Popen,
    timeout_seconds: int,
):
    """
    Compare the actual output on the FVP with the expectations in
    pass and fail output files for the applications.

    setup_resources: Input coming out as a result of executing of setup_resources
                  function defined above.
    fvp_process (subprocess.Popen): FVP execution process
    timeout_seconds (int): Timeout in seconds to wait before terminating the test.
    """
    end_string = "RunQualificationTest returned"

    start = timer()
    current_time = timer()

    regex = re.compile(r"(\d+) Tests (\d+) Failures (\d+) Ignored")
    regex_test_name = re.compile(r"TEST\((.*)\)")
    regex_fail = re.compile(r":FAIL:")
    current_test = ""

    test_cases_ran_count = 0
    test_cases_ignored_count = 0
    test_cases_failed_count = 0
    failed_test_cases = []

    while (current_time - start) < timeout_seconds:
        line = fvp_process.stdout.readline()
        if not line:
            break
        line = line.decode("utf-8")
        line = line.rstrip()
        print(line)
        # look for test summary string
        test_summary = regex.search(line)
        if test_summary:
            test_cases_ran_count += int(test_summary.group(1))
            test_cases_failed_count += int(test_summary.group(2))
            test_cases_ignored_count += int(test_summary.group(3))
        # look for test name and FAIL so we can list failed tests
        test_name = regex_test_name.search(line)
        if test_name:
            current_test = test_name.group(1)
        # FAIL can appear on the line with test name or later
        test_failed = regex_fail.search(line)
        if test_failed:
            failed_test_cases.append(current_test)
        if end_string in line:
            break
        current_time = timer()

    print("--------------------  SUMMARY  --------------------")
    print(f"Total Tests: {test_cases_ran_count}")
    print(f"Total Failures: {test_cases_failed_count}")
    print(f"Total Ignored: {test_cases_ignored_count}")
    print("---------------------------------------------------")
    if len(failed_test_cases):
        print("Failed tests:")
        for test_case in failed_test_cases:
            if test_case in test_cases_to_ignore:
                test_cases_failed_count -= 1
                test_case_ignore_reason = test_cases_to_ignore[test_case]
                print(f" * Ignoring failure: {test_case} - {test_case_ignore_reason}")
            else:
                print(f" - {test_case}")
    print("---------------------------------------------------")

    assert test_cases_failed_count == 0
