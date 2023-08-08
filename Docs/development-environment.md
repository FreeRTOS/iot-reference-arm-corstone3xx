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

## Prerequisites

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
    python3 -m pip install ninja imgtool cffi intelhex cbor2 jinja2 PyYaml
    ```

    **NOTE**: The virtual environment can be deactivated when not needed anymore
    by executing the command below:

    ```bash
    deactivate
    ```
* Installing a toolchain

  This project supports the Arm Compiler for Embedded (armclang) and the Arm
  GNU Toolchain (arm-none-eabi-gcc), and you need *one* of them.

  * Arm Compiler for Embedded

    The [Arm Virtual Hardware instance](./setting-up-arm-virtual-hardware.md)
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
