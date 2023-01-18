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
If you have downloaded the repo without using the `--recurse-submodules` argument, you should run:
```
git submodule update --init --recursive
```

## Prerequisites

* Ubuntu 20.04 or higher. Please note that the following instructions are validated on Ubuntu 20.04.
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
The virtual environment can be deactivated when not needed anymore by executing the command below:
```bash
deactivate
```


## Downloading Corstone-300 FVP
The [ webpage ](https://developer.arm.com/downloads/-/arm-ecosystem-fvps) contains download links to Arm Ecosystem Fixed Virtual Platforms (FVPs). Download and install `Corstone-300 Ecosystem FVPs` based on your operating system. Ensure that the location of Corstone-300 FVP binary `FVP_Corstone_SSE-300_Ethos-U55` is available in `PATH` variable.

# Building the application
To build the blinky example, run the following command:
```bash
./Tools/scripts/build.sh blinky
```

Run the command below to perform a clean build:
```bash
./Tools/scripts/build.sh blinky -c
```

# Running the application
To run the blinky example, run the following command:
```bash
./Tools/scripts/run.sh blinky
```
