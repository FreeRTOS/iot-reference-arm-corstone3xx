# Setting Up your Linux Development Environment

## Build requirements

* Ubuntu 20.04. Please note that the following instructions are
  validated on Ubuntu 20.04.
* Git configuration
  * `git-apply` is used to apply the required patches during CMake configuration.
    In order for this to succeed, a minimum of `User Name` and `User Email`
    must be configured.

    ```bash
    git config --global user.email "email@address"
    git config --global user.name "User Name"
    ```

* Installing CMake

    ```bash
    sudo apt update
    sudo apt install cmake -y
    ```

* Setting up Python 3 virtual environment

    ```bash
    sudo apt update
    sudo apt install python3.8-venv -y
    python3 -m venv ~/fri-venv
    source ~/fri-venv/bin/activate
    ```

* Installing required Python 3 modules

    ```bash
    sudo apt install python3-pip -y
    python3 -m pip install ninja imgtool cffi intelhex cbor2 jinja2 PyYaml pyelftools click pyhsslms
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

    To set up your development environment to use the Arm Compiler for
    Embedded, you can download it from its [official page](https://developer.arm.com/Tools%20and%20Software/Arm%20Compiler%20for%20Embedded).
    Login is required for the download, and you will need a license in order to
    run the toolchain once installed.

    This project has been tested with version *6.21* of the toolchain, which
    is available as `r6p21-00rel0` from the *Revision* drop-down menu on the
    download page after logging in.

  * Arm GNU Toolchain

    This project has been tested with the *13.2.Rel1-2023.10* release of the Arm
    GNU Toolchain. You can download it and make it available in your development
    environment as follows:

    ```bash
    wget https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz

    tar xf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz --directory ~/

    echo PATH=\"$HOME/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin:\$PATH\" >> ~/.bashrc
    source ~/.bashrc
    ```

Follow the intructions in the [document](./pre_commit_and_towncrier_setup.md)
to setup pre-commit hooks and towncrier.

Run the [setup_python_vsi.sh](../../tools/scripts/setup_python_vsi.sh) script
to setup the needed Python environment for VSI to work:

```bash
./tools/scripts/setup_python_vsi.sh
```

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

## Troubleshooting

* If the error `[MQTT Agent Task] DNS_ReadReply returns -11` is appearing, then
  try disabling DHCP in FreeRTOS TCP/IP stack:
    * In the file,
      `applications/<application_name>/configs/freertos_config/FreeRTOSIPConfig.h`,
      set `ipconfigUSE_DHCP` to value `0`
