# Copyright (c) 2023-2024, Arm Limited. All rights reserved.
# SPDX-License-Identifier: MIT

import pytest
import os
from pathlib import Path
import subprocess
from functools import reduce


def get_absolute_path(filename) -> str:
    """
    Get the absolute path of the file
    """
    return Path(__file__).parent.parent.parent / filename


def pytest_addoption(parser):
    parser.addoption(
        "--audio-file-path",
        action="store",
        default="",
        help="Path to the input audio file to be used by applications.",
    )
    parser.addoption("--frm-file-path", action="store", default="")
    parser.addoption("--build-artefacts-path", action="store", default="")
    parser.addoption("--credentials-path", action="store", default="credentials")
    parser.addoption("--fvp", action="store", default="FVP_Corstone_SSE-310")
    parser.addoption("--fvp-options", action="store", default="")
    parser.addoption("--merged-elf-name", action="store", default="")
    parser.addoption("--signed-update-bin-name", action="store", default="")
    parser.addoption("--timeout-seconds", type=int, action="store", default=1800)
    parser.addoption("--pass-output-file", action="store", default="")
    parser.addoption("--fail-output-file", action="store", default="")
    parser.addoption(
        "--pythonhome-path",
        type=str,
        action="store",
        default="",
        help="PYTHONHOME path to be used to configure FVP's working environemnt",
    )


@pytest.fixture()
def build_artefacts_path(pytestconfig):
    yield get_absolute_path(pytestconfig.getoption("--build-artefacts-path"))


@pytest.fixture
def credentials_path(pytestconfig):
    yield get_absolute_path(pytestconfig.getoption("--credentials-path"))


@pytest.fixture
def fvp_path(pytestconfig):
    yield pytestconfig.getoption("--fvp")


@pytest.fixture
def audio_file_path(pytestconfig):
    os.environ["AVH_AUDIO_FILE"] = str(
        get_absolute_path(pytestconfig.getoption("--audio-file-path"))
    )
    yield pytestconfig.getoption("--audio-file-path")


@pytest.fixture
def frm_file_path(pytestconfig):
    yield pytestconfig.getoption("--frm-file-path")


@pytest.fixture
def fvp_options(pytestconfig):
    raw_options = pytestconfig.getoption("--fvp-options")

    if raw_options == "":
        return []

    options = raw_options.split(",")

    def options_builder(options, opt):
        options.append("-C")
        options.append(opt)
        return options

    return reduce(options_builder, options, [])


@pytest.fixture
def merged_elf_name(pytestconfig):
    yield (
        Path(__file__).parent.parent.parent
        / pytestconfig.getoption("--build-artefacts-path")
        / pytestconfig.getoption("--merged-elf-name")
    )


@pytest.fixture
def signed_update_bin_name(pytestconfig):
    yield pytestconfig.getoption("--signed-update-bin-name")


@pytest.fixture
def timeout_seconds(pytestconfig):
    yield pytestconfig.getoption("--timeout-seconds")


@pytest.fixture
def pass_output_file(pytestconfig):
    yield get_absolute_path(pytestconfig.getoption("--pass-output-file"))


@pytest.fixture
def fail_output_file(pytestconfig):
    yield get_absolute_path(pytestconfig.getoption("--fail-output-file"))


@pytest.fixture
def pythonhome_path(pytestconfig):
    yield pytestconfig.getoption("--pythonhome-path")


@pytest.fixture(scope="function")
def fvp_process(
    fvp_path,
    merged_elf_name,
    fvp_options,
    audio_file_path,
    pythonhome_path,
    frm_file_path,
):
    # Fixture of the FVP, when it returns, the FVP is started and
    # traces are accessible through the .stdout of the object returned.
    # When the test is terminated, the FVP subprocess is closed.
    # Note: It can take few seconds to terminate the FVP

    if "SSE-315" in fvp_path:
        cmdline = [
            fvp_path,
            "-a",
            f"{merged_elf_name}",
            "-C",
            "core_clk.mul=200000000",
            "-C",
            "mps4_board.visualisation.disable-visualisation=1",
            "-C",
            "mps4_board.telnetterminal0.start_telnet=0",
            "-C",
            "mps4_board.uart0.out_file=-",
            "-C",
            "mps4_board.uart0.unbuffered_output=1",
            "-C",
            "mps4_board.uart0.shutdown_on_eot=1",
            "-C",
            "mps4_board.subsystem.cpu0.semihosting-enable=1",
            "-C",
            "mps4_board.smsc_91c111.enabled=1",
            "-C",
            "mps4_board.hostbridge.userNetworking=1",
            "-C",
            "mps4_board.subsystem.ethosu.extra_args=--fast",
        ]

        vsi_options = [
            "-C",
            f"mps4_board.v_path={Path(__file__).parent.parent / 'scripts'}",
        ]

        frm_options = [
            "-C",
            "mps4_board.isp_c55_capture_ds.do_capture=0",
            "-C",
            "mps4_board.isp_c55_capture_fr.do_capture=0",
            "-C",
            f"mps4_board.isp_c55_camera.image_file={frm_file_path}",
        ]
    else:
        cmdline = [
            fvp_path,
            "-a",
            f"{merged_elf_name}",
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
            "-C",
            "ethosu.extra_args=--fast",
        ]

        vsi_options = [
            "-C",
            f"mps3_board.v_path={Path(__file__).parent.parent / 'scripts'}",
        ]

        frm_options = []

    cmdline.extend(fvp_options)

    fvp_env = os.environ.copy()

    # The FVP VSI python script path option is used only in case of VSI as input
    # audio source where audio file path would point to the input audio file.
    # If this option is used with ROM as audio source, it would break if PYTHONHOME
    # path is not set.
    if audio_file_path:
        cmdline.extend(vsi_options)
    if frm_file_path:
        cmdline.extend(frm_options)
    if pythonhome_path != "":
        fvp_env["PYTHONHOME"] = pythonhome_path
    proc = subprocess.Popen(cmdline, stdout=subprocess.PIPE, env=fvp_env)
    yield proc
    proc.terminate()
    proc.wait()
