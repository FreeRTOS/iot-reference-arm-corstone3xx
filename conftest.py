# Copyright (c) 2023 Arm Limited. All rights reserved.
# SPDX-License-Identifier: MIT

import pytest
import os
import subprocess
from functools import reduce


def pytest_addoption(parser):
    parser.addoption("--build-path", action="store", default="build")
    parser.addoption("--credentials-path", action="store", default="credentials")
    parser.addoption(
        "--avh", action="store", default="/opt/VHT/VHT_Corstone_SSE-300_Ethos-U55"
    )
    parser.addoption("--avh-options", action="store", default="")


@pytest.fixture()
def build_path(pytestconfig):
    root = os.path.dirname(os.path.abspath(__file__))
    yield root + "/" + pytestconfig.getoption("--build-path")


@pytest.fixture
def credentials_path(pytestconfig):
    root = os.path.dirname(os.path.abspath(__file__))
    yield root + "/" + pytestconfig.getoption("--credentials-path")


@pytest.fixture
def fvp_path(pytestconfig):
    yield pytestconfig.getoption("--avh")


@pytest.fixture
def vsi_script_path():
    yield os.path.dirname(os.path.abspath(__file__)) + "/lib/AVH/audio"


@pytest.fixture
def fvp_options(pytestconfig):
    raw_options = pytestconfig.getoption("--avh-options")

    if raw_options == "":
        return []

    options = raw_options.split(",")

    def options_builder(options, opt):
        options.append("-C")
        options.append(opt)
        return options

    return reduce(options_builder, options, [])


@pytest.fixture(scope="function")
def fvp(fvp_path, build_path, vsi_script_path, fvp_options):
    # Fixture of the FVP, when it returns, the FVP is started and
    # traces are accessible through the .stdout of the object returned.
    # When the test is terminated, the FVP subprocess is closed.
    # Note: It can take few seconds to terminate the FVP
    cmdline = [
        fvp_path,
        "-a",
        f"{build_path}/Projects/aws-iot-example/aws-iot-example_merged.elf",
        "-C",
        "core_clk.mul=200000000",
        "-C",
        "mps3_board.visualisation.disable-visualisation=1",
        "-C",
        "mps3_board.telnetterminal0.start_telnet=0",
        "-C",
        "mps3_board.uart0.out_file=-",
        "-C",
        "mps3_board.uart0.unbuffered_output=1",
        "-C",
        "mps3_board.uart0.shutdown_on_eot=1",
        "-C",
        "cpu0.semihosting-enable=1",
        "-C",
        "mps3_board.smsc_91c111.enabled=1",
        "-C",
        "mps3_board.hostbridge.userNetworking=1",
        "-C",
        "mps3_board.DISABLE_GATING=1",
        "-V",
        f"{vsi_script_path}",
    ]

    cmdline.extend(fvp_options)

    fvp_env = os.environ.copy()
    proc = subprocess.Popen(cmdline, stdout=subprocess.PIPE, env=fvp_env)
    yield proc
    proc.terminate()
    proc.wait()
