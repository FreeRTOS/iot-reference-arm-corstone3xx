# Copyright (c) 2023 Arm Limited. All rights reserved.
# SPDX-License-Identifier: MIT

from timeit import default_timer as timer
from pytest import fixture
from aws_test_util import Flags, create_aws_resources, cleanup_aws_resources
import re

Cases = "Full_OTA_PAL, otaPal_CloseFile_ValidSignature"
Output = "Test cannot succeed as the filename is hardcoded in test"
ignore_tests = {Cases: Output}


@fixture(scope="function")
def aws_resources(build_artefacts_path, credentials_path, signed_update_bin_name):
    flags = Flags(build_artefacts_path, credentials_path, signed_update_bin_name)
    flags = create_aws_resources(flags)
    try:
        # Caller won't actually do anything with this, but we have to yield something.
        yield flags
    finally:
        cleanup_aws_resources(flags)


def test_integration(aws_resources, fvp):
    end_string = "RunQualificationTest returned"

    start = timer()
    current_time = timer()

    regex = re.compile(r"(\d+) Tests (\d+) Failures (\d+) Ignored")
    regex_test_name = re.compile(r"TEST\((.*)\)")
    regex_fail = re.compile(r":FAIL:")
    current_test = ""

    tests = 0
    failures = 0
    ignored = 0
    failed_tests = []

    # Timeout for the test is 20 minutes
    while (current_time - start) < (20 * 60):
        line = fvp.stdout.readline()
        if not line:
            break
        line = line.decode("utf-8")
        line = line.rstrip()
        print(line)
        # look for test summary string
        result = regex.search(line)
        if result:
            tests = tests + int(result.group(1))
            failures = failures + int(result.group(2))
            ignored = ignored + int(result.group(3))
        # look for test name and FAIL so we can list failed tests
        result = regex_test_name.search(line)
        if result:
            current_test = result.group(1)
        # FAIL can appear on the line with test name or later
        result = regex_fail.search(line)
        if result:
            failed_tests.append(current_test)
        if end_string in line:
            break
        current_time = timer()

    print("--------------------  SUMMARY  --------------------")
    print("Total Tests:", tests)
    print("Total Failures:", failures)
    print("Total Ignored:", ignored)
    print("---------------------------------------------------")
    if len(failed_tests):
        print("Failed tests:")
        for fail in failed_tests:
            # ignore tests that we know fail
            if fail in ignore_tests.keys():
                failures = failures - 1
                print(" * Ignoring failure:", fail, "-", ignore_tests[fail])
            else:
                print(" -", fail)
    print("---------------------------------------------------")

    assert failures == 0
