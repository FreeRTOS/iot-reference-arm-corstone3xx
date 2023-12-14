#! /usr/bin/env python3
#
# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

import yaml
import click
from typing import Dict


@click.command()
@click.argument("manifest-file", type=click.Path(exists=True), default="manifest.yml")
def main(manifest_file: str) -> None:
    """
    Perform the following check:
       - All submodule entries in the manifest.yml are not missing a license attribute

    Args:
        manifest_file (str): Path to the YAML manifest file.
    """
    with open(manifest_file, "r") as f:
        manifest_data: Dict = yaml.safe_load(f)

    missing_license_in_manifest_error: bool = check_license_in_manifest(manifest_data)

    if missing_license_in_manifest_error:
        exit(1)
    else:
        print("All submodule entries have valid license and path attributes.")


def check_license_in_manifest(manifest_data: Dict) -> bool:
    """
    Check if all submodules listed in the manifest file have a license attribute.

    Args:
        manifest_data (Dict): The parsed YAML manifest data.

    Returns:
        bool: True if all submodules entries have a license attribute; False otherwise.
    """
    missing_submodules_license: bool = False
    for dependency in manifest_data["dependencies"]:
        if "license" not in dependency:
            print(
                f"Submodule '{dependency['name']}' is missing"
                f" `license` attribute in the manifest file"
            )
            missing_submodules_license = True
    return missing_submodules_license


if __name__ == "__main__":
    main()
