# Prerequisites

## Setting up the platforms

This reference integration supports [Arm ecosystem FVPs](https://developer.arm.com/downloads/-/arm-ecosystem-fvps).

The Corstone-310 model is a reference subsystem for a secure SSE-310 SoC
aligned with the Arm MPS3 development platform. It is based on a Cortex®-M85
processor and an Ethos-U55 neural network processor and the DMA-350
Direct Memory Access controller. Follow the [link](https://developer.arm.com/downloads/-/arm-ecosystem-fvps)
for more information.

### Setting up Arm ecosystem FVPs

* Download the Corstone-310 FVP from [here](https://developer.arm.com/downloads/-/arm-ecosystem-fvps)
* Run the following commands to install the Corstone-310 FVP and add FVP path
  to `PATH` variable.
    ```bash
    mkdir FVP_Corstone_SSE-310_11.24_13
    tar -xzf <download location>/FVP_Corstone_SSE-310_11.24_13_Linux64.tgz -C FVP_Corstone_SSE-310_11.24_13
    cd FVP_Corstone_SSE-310_11.24_13
    ./FVP_Corstone_SSE-310.sh
    echo PATH=\"<FVP installation path>/FVP_Corstone_SSE-310_11.24_13/models/Linux64_GCC-9.3:\$PATH\" >> ~/.bashrc
    source ~/.bashrc
    ```

## Setting up development environment

Follow the instructions described in [Setting Up your Development Environment](development_environment.md)
to setup your development environment.

### Setting up Python VSI environment

Run the [setup_python_vsi.sh](../tools/scripts/setup_python_vsi.sh) script to setup the needed python environment for VSI to work.
