# Prerequisites

## Setting up the platforms

This reference integration supports [Arm ecosystem FVPs](https://developer.arm.com/downloads/-/arm-ecosystem-fvps).

The Corstone-300 ecosystem FVP is aligned with the Arm MPS3 development
platform. It is based on the Cortex-M55 processor and offers a choice of the
Ethos-U55 and Ethos-U65 processors. This FVP is provided free of charge for the
limited development and validation of open-source software on the Corstone-300
platform while Arm Virtual Hardware is recommended for commercial software.
Follow the [link](https://developer.arm.com/downloads/-/arm-ecosystem-fvps) for
more information.

### Setting up Arm ecosystem FVPs

* Download the Corstone-300 FVP from [here](https://developer.arm.com/downloads/-/arm-ecosystem-fvps)
* Run the following commands to install the Corstone-300 FVP and add FVP path
  to `PATH` variable.
    ```bash
    mkdir FVP_Corstone_SSE-300_11.22_20
    tar -xzf <download location>/FVP_Corstone_SSE-300_11.22_20_Linux64.tgz -C FVP_Corstone_SSE-300_11.22_20
    cd FVP_Corstone_SSE-300_11.22_20
    ./FVP_Corstone_SSE-300.sh
    echo PATH=\"<FVP installation path>/FVP_Corstone_SSE-300/models/Linux64_GCC-9.3:\$PATH\" >> ~/.bashrc
    source ~/.bashrc
    ```

## Setting up development environment

Follow the instructions described in [Setting Up your Development Environment](development_environment.md)
to setup your development environment.

### Setting up Python VSI environment

Run the [setup_python_vsi.sh](../tools/scripts/setup_python_vsi.sh) script to setup the needed python environment for VSI to work.
