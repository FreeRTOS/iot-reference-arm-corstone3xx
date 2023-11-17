# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

import subprocess
from timeit import default_timer as timer


def test_application_output(
    fvp_process: subprocess.Popen,
    pass_output_file: str,
    fail_output_file: str,
    timeout_seconds: int,
) -> None:
    """
    Compare the actual output on the FVP with the expectations in
    pass and fail output files.

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
        if line:
            print(line)
        if pass_output[index] in line:
            index += 1
            if index == len(pass_output):
                break
        for x in fail_output:
            assert x not in line
        current_time = timer()

    assert index == len(pass_output)
