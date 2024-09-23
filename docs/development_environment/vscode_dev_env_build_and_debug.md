# Visual Studio Code build and debug

[Visual Studio Code](https://code.visualstudio.com/) is a great and simple IDE
that can be used to setup a development environment for this reference
integration. It also provides tools for building, running and debugging
reference applications.

A [Docker](https://docs.docker.com/get-started/overview) container containing
everything needed to develop is provided for use with VSCode.

This repository supports the [docker/remote container workflow](../../.devcontainer)
in Visual Studio Code, and has a container environment setup automatically.
You can read more about this workflow [Developing inside a Container](https://code.visualstudio.com/docs/devcontainers/containers).

[VScode's](../../.vscode) [tasks](https://code.visualstudio.com/docs/editor/tasks) and
[debugging](https://code.visualstudio.com/docs/editor/debugging) are used to
improve the user experience of developing of reference applications.

## Supported operation systems

* Linux
  * Ubuntu 20.04
* Windows 11
* Mac OS 14.x

## Visual Studio Code devcontainers and Docker setup

If you are not familiar with Visual Studio Code devcontainers or Docker and are
interested in exploring, please start at
https://code.visualstudio.com/docs/devcontainers/containers and
https://docs.docker.com/get-started/overview before continuing.

* *Windows Only* Enable the Windows Subsystem for Linux (WSL) following
instructions here: https://docs.microsoft.com/en-us/windows/wsl/install-win10
* *Windows Only* Install Ubuntu from the Windows App Store here:
https://apps.microsoft.com/detail/9mttcl66cpxj?hl=en-us&gl=US
* Install [Git](https://git-scm.com/) for your operating system
* *Windows Only* Enable git to use LF instead of CLRF by default:
git config --global core.autocrlf false
* Install [Docker](https://www.docker.com/) for your operating system of choice
from here: https://docs.docker.com/install
* Install [Visual Studio Code](https://code.visualstudio.com/) for your
operating system of choice here: https://code.visualstudio.com/Download
* Install the [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
extension for Visual Studio Code
* Launch Visual Studio Code

As documented [here](https://code.visualstudio.com/remote/advancedcontainers/improve-performance),
the Visual Studio Code devcontainers uses `bind` mount to make the source code
on your host machine to be available inside the container by default. On macOS
and Windows, `bind` mount may cause slower disk performance while building the
project. Therefore, we strongly recommend,
[Open a Git repository in an isolated container volume](https://code.visualstudio.com/docs/devcontainers/containers#_quick-start-open-a-git-repository-or-github-pr-in-an-isolated-container-volume).

> Note:
    When using isolated container volumes, the source code is only available
    inside the container and not on the host. Therefore, we strongly suggest
    backing up container volumes regularly using Docker extension
    [Volumes Backup & Share](https://hub.docker.com/extensions/docker/volumes-backup-extension).

### Open this project in a container volume

* Run
```
Command Palette (F1) => Dev Containers: Clone Repository in Container Volume...
```
* Enter
```
https://github.com/FreeRTOS/iot-reference-arm-corstone3xx
```
* The VS Code window (instance) will reload, clone the source code, and start
building the dev container. A progress notification provides status updates.

If building the container fails, then check the [troubleshooting](#troubleshooting)
section for possible solutions.

Note:
  `zsh` and `ohmyzsh` is setup by default in the container. If you would like to
  use `bash`, then:
  * Set `SHELL_CHOICE` in [devcontainer.json](../../.devcontainer/devcontainer.json)
  to `bash`
  * Run: Command Palette (F1) => Dev Containers: Rebuild Container

    The container will be rebuilt with `bash` as the default shell.

Once the container is built successfully, the source code is displayed in the
VS Code explorer in the side bar.

Follow the intructions in the [document](./pre_commit_and_towncrier_setup.md)
to setup pre-commit hooks and towncrier.

Run the [setup_python_vsi.sh](../../tools/scripts/setup_python_vsi.sh) script
to setup the needed python environment for VSI to work:

```bash
./tools/scripts/setup_python_vsi.sh
```

## Building a reference application

These options allow you to build reference applications.

> The Arm GNU Toolchain is provided with the container and is the default build
toolchain. If you wish to use the Arm Compiler for Embedded instead, you can
download it from its [official page](https://developer.arm.com/Tools%20and%20Software/Arm%20Compiler%20for%20Embedded).
Login is required for the download, and you will need a license in order to
run the toolchain once installed.
>
> This project has been tested with version *6.21* of the toolchain, which
is available as `r6p21-00rel0` from the *Revision* drop-down menu on the
download page after logging in.

* **Using generic vscode task:**
```
Command Palette (F1)
=> Tasks: Run Task...
=> Build Arm Featured Reference Integration applications
=> (reference application name)
=> (build target)
=> (Path to device certificate)
=> (Path to device private key)
=> (build directory path)
=> (build toolchain)
=> (clean build)
```

* **calling build script directly:**
```
./tools/scripts/build.sh <reference application name> --toolchain <GNU/ARMCLANG>
--certificate_path <certificate pem's path>
--private_key_path <private key pem's path> --target <target name>
--inference <inference engine> --audio <audio input> --conn-stack <connectivity stack>
--psa-crypto-implementation <library providing PSA Crypto APIs implementation>
```

## Running a reference application

These options allow you to run reference applications on a FVP platform.

* **Using generic vscode task:**
```
Command Palette (F1)
=> Tasks: Run Task...
=> Run Arm Featured Reference Integration applications
=> (reference application name)
=> (build target)
=> (camera frames path)
=> (build directory path)
```

* **calling run script directly:**
```
./tools/scripts/run.sh <reference application name> --target <target name>
-f <camera frames path> -p (build directory path)
```

## Debugging a reference application

Debugging a reference application is a more complex action. It's required to run
debug server and connect the client to it. The [Cortex-debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) is used in this VSCode
debugging solution. It requires the ARM GCC Toolchain and GDB server instance
(e.g OpenOCD) but can also works in remote server mode.
In this case, the external GDB server in used so the first step is to run FVP
with debug plugin. We use the [GDBRemoteConnection library](../../tools/plugin)
and pass it to executed FVP (--allow-debug-plugin --plugin
GDBRemoteConnection.so).
These options allow you to run reference applications in debug mode.

### Getting GDBRemoteConnection library

* Register/login to [Arm developer hub](https://developer.arm.com)
* On the top right corner, click on `Downloads`
* In the search window enter `FM000A`
  * Click on the search result `Fast Models (FM000A)`
* Download `Third Party Add-ons for Fast Models <version> on Linux`[AArch64/x86]
  based on the processor architecture of the machine
* copy downloaded ``Third Party Add-ons for Fast Models` to devcontainer
(drag and drop from host to devcontainer)

  ```bash
  tar -xzf FastModels_ThirdParty_IP_11-25_b15_Linux64[_armv8l].tgz
  cd FastModels_ThirdParty_IP_11-25_Linux64[_armv8l]
  ./setup.bin --i-accept-the-end-user-license-agreement
  ```

  > Note:
      Using this option means you have read and accepted the terms and
      conditions of the End User License Agreement for the product and version
      installed.

      NOTICE FOR SOFTWARE FILES DELIVERED BY ARM LIMITED FOR CONVENIENCE ONLY

      THIS NOTICE ("NOTICE") IS FOR THE USE OF THE SOFTWARE ACCOMPANYING THIS
      NOTICE. ARM IS ONLY DELIVERING THE SOFTWARE TO YOU FOR CONVENIENCE ON
      CONDITION THAT YOU ACCEPT THAT THE SOFTWARE IS NOT LICENSED TO YOU BY ARM
      BUT THAT THE SOFTWARE IS SUBJECT TO A SEPARATE LICENSE. BY CLICKING
      "I AGREE" OR BY INSTALLING OR OTHERWISE USING OR COPYING THE SOFTWARE YOU
      INDICATE THAT YOU AGREE TO BE BOUND BY ALL THE TERMS OF THE INDIVIDUAL
      LICENSE AGREEMENTS OF THE SOFTWARE AND IF YOU DO NOT AGREE TO THE TERMS OF
      THIS NOTICE, ARM IS UNWILLING TO DELIVER THE SOFTWARE TO YOU AND YOU MAY
      NOT INSTALL, USE OR COPY THE SOFTWARE.

  Once the installation is successful, the GDBRemoteConnection library can be
  found in
  `/home/<username>/ARM/FastModelsPortfolio_11.25/plugins/Linux64_armv8l_GCC-9.3`

* copy the `GDBRemoteConnection.so` to
`/workspaces/iot-reference-arm-corstone3xx/tools/plugin`

* **Using generic vscode task:**
```
Command Palette (F1)
=> Tasks: Run Task...
=> Debug Arm Featured Reference Integration applications
=> (reference application name)
=> (run target)
=> (build directory path)
=> (camera frames path)
```

The next step is to run the debug session using VSCode launch configuration.

```
Run and Debug (Ctrl+Shift+D)
=> Start Debugging (F5)
=> (reference application name)
```

As soon as a debugging session starts, the DEBUG CONSOLE panel is displayed and
shows debugging output.

## Troubleshooting

### Linux machines

* If building the container fails with:
  ```
  ERROR [3/5] RUN apk add --no-cache  git-lfs  nodejs  python3  npm  make  g++
  docker-cli  docker-cli-buildx  docker-cli-compose  openssh-client-def
  ```
    then, try turing VPN off and rebuild the container.
* If the error `[MQTT Agent Task] DNS_ReadReply returns -11` is appearing even
  with VPN turned off, then try disabling DHCP in FreeRTOS TCP/IP stack:
    * In the file,
      `applications/<application_name>/configs/freertos_config/FreeRTOSIPConfig.h`,
      set `ipconfigUSE_DHCP` to value `0`

### Windows machines

* If building the container fails with:
  ```
  ERROR [3/5] RUN apk add --no-cache  git-lfs  nodejs  python3  npm  make  g++
  docker-cli  docker-cli-buildx  docker-cli-compose  openssh-client-def
  ```
    then, try turing VPN off and rebuild the container.
