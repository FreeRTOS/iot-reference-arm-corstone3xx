# Copyright 2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

from datetime import datetime, timezone
import json
import pathlib
import os
import subprocess
import sys

HOLDER = "Arm"
CURRENT_YEAR = str(datetime.now(timezone.utc).year)


# Ensures all modified files with Arm copyright have the current year;
# lists the paths of files that need updating
def main():
    outdated_year_files = []
    report = json.loads(pathlib.Path("scancode_report.json").read_text())
    repo_root = pathlib.Path(os.getcwd()).resolve()
    path_prefix = f"{repo_root.parent.name}/{repo_root.name}/"

    # Builds a dictionary of all files and their corresponding copyright
    all_copyrights = {}
    for file in report.get("files", []):
        scanned_path = file.get("path", "")
        if not scanned_path.startswith(path_prefix):
            continue
        relative_path = scanned_path[slice(len(path_prefix), None)]
        all_copyrights[relative_path] = [
            cp["value"] for cp in file.get("copyrights", [])
        ]

    # Determine what files have been modified by looking at the commit range
    base = os.getenv("CI_MERGE_REQUEST_DIFF_BASE_SHA") or "HEAD^"
    head = os.getenv("CI_COMMIT_SHA") or "HEAD"
    changed = set(
        subprocess.check_output(
            ["git", "diff", "--name-only", f"{base}...{head}"], text=True
        ).splitlines()
    )

    for relative_path in changed:
        file_copyrights = all_copyrights.get(relative_path, [])
        # If a file doesn't have a copyright, or it is correctly formatted, skip it
        if (not file_copyrights) or (
            any(HOLDER in cp and CURRENT_YEAR in cp for cp in file_copyrights)
        ):
            continue
        if any(HOLDER in cp for cp in file_copyrights):
            outdated_year_files.append(relative_path)

    if outdated_year_files:
        print(
            "Please update the copyright year to",
            CURRENT_YEAR,
            "in the following files:",
        )
        print("\n".join(f"  - {p}" for p in outdated_year_files))
        sys.exit(1)


if __name__ == "__main__":
    main()
