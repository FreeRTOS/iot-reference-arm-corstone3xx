# Setting Up your Development Environment

## Cloning the Repository

To clone using HTTPS:
```
git clone https://github.com/FreeRTOS/iot-reference-arm-corstone3xx.git --recurse-submodules
```
Using SSH:
```
git clone git@github.com:FreeRTOS/iot-reference-arm-corstone3xx.git --recurse-submodules
```
If you have downloaded the repo without using the `--recurse-submodules`
argument, you should run:
```
git submodule update --init --recursive
```

## Build requirements

* Ubuntu 20.04 or higher. Please note that the following instructions are
  validated on Ubuntu 20.04.
* Git configuration
  * `git-am` is used to apply the required patches during CMake configuration.
    In order for this to succeed, a minimum of `User Name` and `User Email`
    must be configured.

    ```bash
    git config --global user.email "email@address"
    git config --global user.name "User Name"
    ```
* Setting up python 3 virtual environment

    ```bash
    sudo apt update
    sudo apt install python3.8-venv -y
    python3 -m venv ~/fri-venv
    source ~/fri-venv/bin/activate
    ```

* Installing required python 3 modules

    ```bash
    sudo apt install python3-pip -y
    python3 -m pip install ninja imgtool cffi intelhex cbor2 jinja2 PyYaml pyelftools click
    ```

    **NOTE**: The virtual environment can be deactivated when not needed anymore
    by executing the command below:

    ```bash
    deactivate
    ```
* Installing required libraries

    ```bash
    sudo apt install srecord
    sudo apt install binutils
    ```
* Installing a toolchain

  This project supports the Arm Compiler for Embedded (armclang) and the Arm
  GNU Toolchain (arm-none-eabi-gcc), and you need *one* of them.

  * Arm Compiler for Embedded

    The [Arm Virtual Hardware instance](./setting_up_arm_virtual_hardware.md)
    comes with the Arm Compiler for Embedded in its environment which is ready
    for use.

    If you would like to set up your *local* development environment to use the
    Arm Compiler for Embedded, you can download it from its [official page](https://developer.arm.com/Tools%20and%20Software/Arm%20Compiler%20for%20Embedded).
    Login is required for the download, and you will need a license in order to
    run the toolchain once installed.

    This project has been tested with version *6.18* of the toolchain, which
    is available as `r6p18-00rel0` from the *Revision* drop-down menu on the
    download page after logging in.

  * Arm GNU Toolchain

    This project has been tested with the *10.3-2021.10* release of the Arm
    GNU Toolchain. You can download it and make it available in your development
    environment as follows:

    ```bash
    wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-`uname -m`-linux.tar.bz2

    tar xf gcc-arm-none-eabi-10.3-2021.10-`uname -m`-linux.tar.bz2 --directory ~/

    echo PATH=\"$HOME/gcc-arm-none-eabi-10.3-2021.10/bin:\$PATH\" >> ~/.bashrc
    source ~/.bashrc
    ```

## Setting up pre-commit hooks

As an optional step, you can install and run pre-commit hooks. These are git pre-commit hook scripts useful for identifying issues before submitting change for review. We run pre-commit check every commit to automatically point out issues in code such as missing semicolons, trailing whitespace, and debug statements. By resolving these issues before review, we enable the reviewer to focus on the architecture of a change while not spending time with trivial style nitpicks.


**Note**:

As mentioned before, setting up and running pre-commit during development process is optional. But it'd certainly help with making CI pass as the checks mentioned below are part of the CI checks.

### To install pre-commit tool:

  ```bash
  python3 -m pip pre-commit
  ```

### Using the pre-commit hooks
After installing pre-commit, you can use the existing [.pre-commit-config.yaml](../.pre-commit-config.yaml) or you can use your own version of `.pre-commit-config.yaml` where you'll have to replace your own file with the existing one without modifying it. Afterwards, you can add the hooks you want to use. For example, you can use the following content:
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

Refer to the [.pre-commit-config.yaml](../.pre-commit-config.yaml) for the default options used for cppcheck.

#### Banned-api

This pre-commit hook is used to check if certain unsafe C/C++ APIs are used in your code. By default, `banned-api` checks the [complete list](../tools/ci/hooks/banned_api_list.txt) present in the repository.

#### Trims trailing whitespace
Check and trim any trailing whitespace.

#### End of file fixer
Checks that each file has one empty line at the end of it.

#### Check added large files
Prevents giant files from being committed.

#### Gitlint
Checks commit messages against pre-defined style.

#### Black and flake8
Checks python files formatting.

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
