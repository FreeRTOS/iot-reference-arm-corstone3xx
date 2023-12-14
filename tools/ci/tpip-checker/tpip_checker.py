# Copyright (c) 2022-2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

"""
This module contains functionality to check for potentially missing
TPIP.
"""

from pathlib import Path
import re
import sys
import logging
import pathlib
from urllib.parse import urlparse, urljoin
import click
import gitlab

TPIP_COMMENT_HEADER = "**TPIP checker report**"
TPIP_WARNING_LABEL = "Do not merge: TPIP warning"


def check_line_for_url_match(line, file, file_url, current_line_marker):
    """
    Checks the supplied line to see if it matches any diff change
    containing a URL of the forms:
    http:
    https:
    svn+ssh:

    If matched returns a corresponding comment else an empty string
    """

    comment = ""
    url_changed_re = re.compile(r"[-\+].*[https*|svn\+ssh]:\/\/")
    if url_changed_re.match(line):
        # A URL line has been modified
        comment += f"\n[{file}]({file_url})\n"
        comment += f"\n- {current_line_marker}\n"
        comment += "\n- A URL line has been modified.\n"

    return comment


def check_for_url_changes_in_source_files(file, file_url, change):
    """
    Checks if any urls have been added or removed in a designated
    set of file types
    """

    line_marker_re = re.compile("@@.*@@")
    comment = ""
    current_line_marker = ""

    # List of source/build file extensions to be used to determine
    # which files should be checked for URL addition / removal
    src_bld_suffixes = [".c", ".h", ".cpp", ".yml", ".yaml"]

    # Check if the file is a source or build file
    suffix = pathlib.Path(file).suffix

    if suffix in src_bld_suffixes:
        for line in change["diff"].splitlines():
            # Check for change to line marker
            if line_marker_re.match(line):
                current_line_marker = line.strip()

            comment += check_line_for_url_match(
                line, file, file_url, current_line_marker
            )

    return comment


def check_file_changes(change, path_base, server_url):
    """
    Checks the file diffs for the specified file and returns
    a descriptive string if potential TPIP changes are present.
    Returns an empty string if not.

    """
    file = change["new_path"]
    file_path = Path(path_base).parent / file
    file_url = urljoin(server_url, str(file_path))

    logging.info(f"\nFile: {file_url}\nDiffs:\n{change['diff']}")
    comment = ""

    if ".gitmodules" in file:
        # Check file changes and look for module change
        comment += f"\n[{file}]({file_url})\n"
        comment += "\n- The gitmodules content has been changed.\n"

    else:
        comment += check_for_url_changes_in_source_files(file, file_url, change)

    return comment


@click.command()
@click.option(
    "--server-url",
    default="https://gitlab.arm.com",
    required=True,
    help="GitLab server url",
)
@click.option(
    "--private-token",
    required=True,
    help="Private token for GitLab API requests",
)
@click.option(
    "--project-id",
    required=True,
    help="GitLab project id",
)
@click.option(
    "--merge-req-id",
    required=True,
    help="ID for the merge request",
)
@click.option(
    "--verbose",
    default=False,
    is_flag=True,
    help="Enable verbose logging",
)
def check_files_for_tpip(server_url, private_token, project_id, merge_req_id, verbose):
    """
    This program reads the changes provided as part of a specifc
    merge request and looks for changes that could indicate the
    addition or removal of TPIP. If any such changes are found then
    a comment is added to the merge request to alert the author and
    ip reviewers.
    """

    if verbose:
        logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)
    else:
        logging.basicConfig(stream=sys.stdout, level=logging.INFO)

    project = gitlab.Gitlab(url=server_url, private_token=private_token).projects.get(
        project_id
    )

    # Check each file for TPIP related changes
    # 1. Flag up any changes to .gitmodules
    # 2. In other source files check if an 'http' address has been added
    #    or removed.
    #
    # In all cases if any matches are found, comment in the MR for the
    # attention of the ip reviewer.

    merge_req = project.mergerequests.get(merge_req_id)
    merge_req_changes = merge_req.changes()

    # Get the repo URL and strip off the scheme and network location
    # This is required as the file as the URL provided by the MR diffs is in
    # the incorrect format. We also need to add in the source branch details.
    repo_url = urlparse(project.http_url_to_repo)._replace(scheme="https").path
    rel_repo_path = pathlib.Path(repo_url).with_suffix("")
    path_base = Path(rel_repo_path) / "-/blob" / merge_req_changes["source_branch"]

    mr_comment = ""
    for change in merge_req_changes["changes"]:
        mr_comment += check_file_changes(change, path_base, server_url)

    if len(mr_comment) > 0:
        mr_comment = TPIP_COMMENT_HEADER + "\n\n" + mr_comment
        mr_comment += """
        \nPlease ensure the LICENSE.md file has been updated to reflect any
        possible TPIP changes indicated by the aforementioned changes.
        """
        # Check if a comment already exists and if so update that one
        update_comment = None
        for note in merge_req.notes.list():
            if TPIP_COMMENT_HEADER in note.body:
                update_comment = merge_req.notes.get(note.id)

        if update_comment is not None:
            # Check if the TPIP warning has changed compared to the last
            # one and only update the comment and label accordingly
            if mr_comment.strip() != (update_comment.body).strip():
                logging.info("Updating previous TPIP comment.")

                update_comment.body = mr_comment
                update_comment.save()

                # The TPIP warning has changed so re-add the label
                merge_req.labels.append(TPIP_WARNING_LABEL)
                merge_req.save()
            else:
                logging.info("No additional TPIP warnings compared to previously.")

        else:
            logging.info("Creating new TPIP comment and statement")

            # Add a TPIP warning label

            # First check if the label already exists or not in the project
            if TPIP_WARNING_LABEL not in [
                label.name for label in project.labels.list()
            ]:
                project.labels.create({"name": TPIP_WARNING_LABEL, "color": "#e2432a"})

            merge_req.labels.append(TPIP_WARNING_LABEL)
            merge_req.notes.create({"body": mr_comment})
            merge_req.save()


if __name__ == "__main__":
    check_files_for_tpip()
