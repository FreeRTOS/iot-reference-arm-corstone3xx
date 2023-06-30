# Setting Up your Development Environment

## Cloning the Repository

To clone using HTTPS:
```
git clone https://git.gitlab.arm.com/iot/aws/freertos/iot-reference-arm-corstone3xx.git --recurse-submodules
```
Using SSH:
```
git clone git@git.gitlab.arm.com:iot/aws/freertos/iot-reference-arm-corstone3xx.git --recurse-submodules
```
If you have downloaded the repo without using the `--recurse-submodules`
argument, you should run:
```
git submodule update --init --recursive
```

## Prerequisites

* Ubuntu 20.04 or higher. Please note that the following instructions are
  validated on Ubuntu 20.04.
* Setting up python 3 virtual environment
```bash
python3 -m venv ~/fri-venv
source ~/fri-venv/bin/activate
```

* Install required python 3 modules
```bash
sudo apt update
sudo apt install python3-pip
python3 -m pip install ninja imgtool cffi intelhex cbor2 cbor jinja2 PyYaml
```

***NOTE***  
The virtual environment can be deactivated when not needed anymore by executing
the command below:

```bash
deactivate
```
