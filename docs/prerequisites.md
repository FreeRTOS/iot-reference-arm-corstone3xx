# Prerequisites

## Setting up the platforms

This reference integration supports both [Arm ecosystem FVPs](https://developer.arm.com/downloads/-/arm-ecosystem-fvps)
and Arm Virtual Hardware using [Amazon Machine Images](#setting-up-arm-virtual-hardware-using-amazon-machine-images).

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

### Setting up Arm Virtual Hardware using Amazon Machine Images

Follow the instructions described in [Launch Arm Virtual Hardware Instance](setting_up_arm_virtual_hardware.md)
to setup your development environment.

If you have successfully followed the instructions, then you should have a
console (either AWS-Web-Console or Local-Console) to an Arm Virtual Hardware
Instance. From now on, any command-line commands described in this document
must be run on the console connected to the Arm Virtual Hardware Instance.

**Note**

The run example script `Tools/scripts/run.sh` assumes ecosystem FVP by default.
If you are using Arm virtual hardware using Amazon machine images then an
additional argument `--fvp_type vht` must be passed to the run example script.

## Setting up development environment

Follow the instructions described in [Setting Up your Development Environment](development_environment.md)
to setup your development environment.
