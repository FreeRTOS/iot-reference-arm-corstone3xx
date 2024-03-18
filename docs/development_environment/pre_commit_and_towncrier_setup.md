## Setting up pre-commit hooks

As an optional step, you can install and run pre-commit hooks. These are git pre-commit hook scripts useful for identifying issues before submitting change for review. We run pre-commit check every commit to automatically point out issues in code such as missing semicolons, trailing whitespace, and debug statements. By resolving these issues before review, we enable the reviewer to focus on the architecture of a change while not spending time with trivial style nitpicks.


**Note**:

As mentioned before, setting up and running pre-commit during development process is optional. But it'd certainly help with making CI pass as the checks mentioned below are part of the CI checks.

### To install pre-commit tool:

  ```bash
  python3 -m pip pre-commit
  ```

### Using the pre-commit hooks
After installing pre-commit, you can use the existing [.pre-commit-config.yaml](../../.pre-commit-config.yaml) or you can use your own version of `.pre-commit-config.yaml` where you'll have to replace your own file with the existing one without modifying it. Afterwards, you can add the hooks you want to use. For example, you can use the following content:
```
repos:
-   repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.5.0
    hooks:
    -   id: trailing-whitespace
    -   id: end-of-file-fixer
    -   id: check-added-large-files
        args: ['--maxkb=2500']
-   repo: https://github.com/jorisroovers/gitlint
    rev:  v0.19.1
    hooks:
    -   id: gitlint
        args:
        - "--config tools/ci/gitlint/.gitlint"
-   repo: https://github.com/psf/black
    rev: 23.10.1
    hooks:
    -   id: black
        args: [
            "-l 88"
        ]
-   repo: https://github.com/pycqa/flake8
    rev: 6.1.0
    hooks:
    -   id: flake8
        args: # arguments to configure flake8
        # making line length compatible with black
        - "--max-line-length=88"
-   repo: local
    hooks:
      - id: cppcheck
        name: cppcheck
        description: Run `cppcheck` against C/C++ source files
        language: system
        files: \.(c|cc|cpp|cu|c\+\+|cxx|tpp|txx)$
        entry: cppcheck --error-exitcode=1
        args: [
            "--force",
            "--std=c99",
            "--std=c++14",
            # enable everything except "information" level.
            "--enable=style,performance,warning,portability",
            "--template=gcc",
            "--inline-suppr",
            # Do not fail for internal cppcheck error
            "--suppress=internalAstError",
            # As we are passing list of suppression list, some files may
            # not need to suppress any or all of the suppressions producing
            # unmatchedSuppression by cppcheck. Ignore such cases.
            "--suppress=unmatchedSuppression",
            # useStlAlgorithm cannot be mandated in embedded projects
            "--suppress=useStlAlgorithm"
        ]
-   repo: local
    hooks:
      - id: uncrustify
        name: uncrustify
        description: Run 'uncrustify' C/C++ code formatter
        language: script
        entry: tools/run_uncrustify.sh
        require_serial: true
-   repo: local
    hooks:
      - id: banned-api
        name: banned-api
        entry: banned-api-hook
        description: Checks if source code uses banned apis
        types_or: [c, c++]
        language: python

```
You can then run for all files or on every commit as mentioned in the [quick start guide](https://pre-commit.com/index.html#quick-start).

### Hooks used in this project

#### Cppcheck
Cppcheck is a static code analysis tool for the C and C++ programming languages.

Cppcheck needs to be installed for running this hook. Follow the [official documentation](https://cppcheck.sourceforge.io/) to install cppcheck.

Refer to the [.pre-commit-config.yaml](../../.pre-commit-config.yaml) for the default options used for cppcheck.

#### Banned-api

This pre-commit hook is used to check if certain unsafe C/C++ APIs are used in your code. By default, `banned-api` checks the [complete list](../../tools/ci/hooks/banned_api_list.txt) present in the repository.

#### Trims trailing whitespace
Check and trim any trailing whitespace.

#### End of file fixer
Checks that each file has one empty line at the end of it.

#### Check added large files
Prevents giant files from being committed.

#### Gitlint
Checks commit messages against pre-defined style.

#### Black and flake8
Checks Python files formatting.

#### Uncrustify
Checks C/C++ code formatting.

## Towncrier guideline

### Installing Towncrier

Please refer to https://github.com/twisted/towncrier#readme

### Configuration

The configuration options are found in the 'pyproject.toml' file in the root directory.

### Adding a release change note for your merge request changeset

To create the file for your release change (which must be added as part of any merge request changeset):

```
towncrier create --edit <prefix>.<suffix> --config ./pyproject.toml --dir .
```

Where:

`<prefix>` is the date/time in the following format `'%Y%m%d%H%M'`, this can be obtained in linux by running the date command.

```
date +'%Y%m%d%H%M'
```

The same format can be obtained in windows powershell with the following command:
```
Get-Date -Format "yyyyMMddHHmm"
```

`<suffix>` is currently limited to "change", however this may be expanded in the future.

Note, the --edit flag opens an editor immediately to allow the text for the change to be added.

The text should take the form of a one line summary of the change being added (this could be
as simple as a copy of the anticipated merge request title).

A check is run in the CI against each new merge request to ensure that a news/change file has been added
for this changeset.
