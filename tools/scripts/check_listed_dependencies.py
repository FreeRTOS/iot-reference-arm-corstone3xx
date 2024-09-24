#! /usr/bin/env python3
#
# Copyright 2023-2024 Arm Limited and/or its affiliates
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
       - All dependency entries in the manifest.yml are not missing:
            - a license attribute
            - a TPIP category attribute
            - a version attribute
            - a path attribute
            - a security risk attribute
    Args:
        manifest_file (str): Path to the YAML manifest file.
    """
    with open(manifest_file, "r") as f:
        manifest_data: Dict = yaml.safe_load(f)

    if check_the_manifest(manifest_data):
        print("All dependency entries in the manifest have mandatory attributes.")
    else:
        exit(1)


def check_the_manifest(manifest_data: Dict) -> bool:
    """
    Check if all dependencies listed in the manifest have mandatory attributes.

    Args:
        manifest_data (Dict): The parsed YAML manifest data.

    Returns:
        bool: True if all dependencies listed in the manifest have mandatory
        attributes; False otherwise.
    """
    manifest_has_all_attributes: bool = True
    for dependency in manifest_data["dependencies"]:
        if "license" not in dependency:
            print(
                f"Dependency '{dependency['name']}' is missing"
                f" `license` attribute in the manifest file"
            )
            manifest_has_all_attributes = False
        if "tpip-category" not in dependency:
            print(
                f"Dependency '{dependency['name']}' is missing"
                f" `tpip-category` attribute in the manifest file"
            )
            manifest_has_all_attributes = False
        if "security-risk" not in dependency:
            print(
                f"Dependency '{dependency['name']}' is missing"
                f" `security-risk` attribute in the manifest file"
            )
            manifest_has_all_attributes = False
        if "version" not in dependency:
            print(
                f"Dependency '{dependency['name']}' is missing"
                f" `version` attribute in the manifest file"
            )
            manifest_has_all_attributes = False
        if "path" not in dependency["repository"]:
            print(
                f"Dependency '{dependency['name']}' is missing"
                f" `path` attribute in the manifest file"
            )
            manifest_has_all_attributes = False
    return manifest_has_all_attributes


if __name__ == "__main__":
    main()
