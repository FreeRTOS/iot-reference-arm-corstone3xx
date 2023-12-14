# tpip_checker.py

## Summary

This script is designed to run as part of the pre-commit against newly raised
merge requests.

It's purpose is to look for possible areas in the merge request files that could
affect third party IP (TPIP).

## How it works

The script takes the diff from the merge request and parses it looking for the
following:

1. For .gitmodules
   a) Any changes

2. For src files
   a) Search for a URL modification

   The list of designated source file types is not exhaustive and is controlled
   by the src_bld_suffixes list in the check_for_url_changes_in_source_files
   function.

## Output

If any possible TPIP changes are found then a comment is added to the MR
detailing what the changes are and in what file, (a clickable link to the file
is provided). A warning label is also added to the MR stating that the MR is
not to be merged until all the comments have been checked.

## Review process

As part of the IP review, both the author and the IP reviewer should consider
any comments highlighted by the script and if necessary address them. Once there
are no comments left that need addressing the label can be manually removed.

Note: if the MR is updated at any time the script will run again and if the
comments have changed the MR comment will be automatically updated. Also if the
label has previously been removed, it will automatically be re-added.

## License

Source code is available under the terms of the MIT License.
