# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

FROM public.ecr.aws/ubuntu/ubuntu:22.04_stable

SHELL ["/bin/bash", "-c"]

ARG USERNAME=ubuntu
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ARG SHELL_CHOICE="zsh"

ENV DEBIAN_FRONTEND=noninteractive
ENV USER=$USERNAME

# Install development tools
RUN apt-get update && \
    apt-get -y install \
    bash-completion \
    build-essential \
    ccache \
    cppcheck \
    curl \
    fd-find \
    gdb \
    lcov \
    libbz2-dev \
    libffi-dev \
    libgdbm-dev \
    libgdbm-compat-dev \
    liblzma-dev \
    libncurses5 \
    libncurses5-dev \
    libncursesw5 \
    libreadline6-dev \
    libsndfile1 \
    libsqlite3-dev \
    libssl-dev \
    libxcursor-dev \
    lzma \
    lzma-dev \
    ninja-build \
    pkg-config \
    python3-pip \
    python3.10-venv \
    software-properties-common \
    srecord \
    sudo \
    tk-dev \
    unzip \
    uuid-dev \
    vim \
    wget \
    zlib1g-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists

# Install git
RUN apt-add-repository -y ppa:git-core/ppa && \
    apt-get update && \
    apt-get -y install git

# Setup zsh
RUN if [[ $SHELL_CHOICE == "zsh" ]]; then \
        apt-get update && \
        apt-get -y install zsh  && \
        apt-get clean && \
        rm -rf /var/lib/apt/lists; \
    fi

# Install Python dependencies
RUN pip3 install \
    cbor \
    cbor2 \
    cffi \
    cmake \
    imgtool \
    intelhex \
    jinja2 \
    ninja \
    pre-commit \
    pyelftools \
    pyhsslms \
    PyYaml \
    setuptools \
    towncrier \
    wheel

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Set the default user
USER $USERNAME

# Install oh-my-zsh
RUN if [[ $SHELL_CHOICE == "zsh" ]]; then \
        sh -c "$(wget https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)" "" --unattended; \
    fi

# Persist bash history - https://code.visualstudio.com/remote/advancedcontainers/persist-bash-history
# Setup shell history. Do not change the order since `oh-my-zsh` updates `~/.zshrc`.
RUN sudo mkdir /commandhistory
RUN if [[ $SHELL_CHOICE == "zsh" ]]; then \
        sudo touch /commandhistory/.zsh_history && \
        sudo chown -R $USERNAME:$USERNAME /commandhistory && \
        echo "export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.zsh_history" >> /home/$USERNAME/.zshrc; \
    fi;
RUN if [[ $SHELL_CHOICE == "bash" ]]; then \
        sudo touch /commandhistory/.bash_history && \
        sudo chown -R $USERNAME:$USERNAME /commandhistory && \
        echo "export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" >> /home/$USERNAME/.bashrc; \
    fi

# Install zsh-autosuggestions plugin
RUN if [[ $SHELL_CHOICE == "zsh" ]]; then \
        git clone https://github.com/zsh-users/zsh-autosuggestions ~/.oh-my-zsh/custom/plugins/zsh-autosuggestions && \
        echo "source ~/.oh-my-zsh/custom/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh" >> /home/$USERNAME/.zshrc; \
    fi

# Setup vim
RUN cat <<EOF > /home/$USERNAME/.vimrc
syntax on
filetype plugin indent on

set backspace=indent,eol,start

set tabstop=4
set textwidth=80
set wm=2

set hlsearch
hi Search ctermbg=LightCyan

:set spell spelllang=en_us
:set spellfile=~/.vim/spell/en.utf-8.add
hi clear SpellBad
hi SpellBad cterm=underline,bold ctermfg=red
EOF

# Download and install arm-none-eabi-gcc
RUN wget -qO - https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-$(uname -m)-arm-none-eabi.tar.xz \
            | sudo tar -xJ -C /opt \
            && sudo mv /opt/arm-gnu-toolchain-13.2.Rel1-$(uname -m)-arm-none-eabi /opt/arm-gnu-toolchain-13.2.Rel1
ENV PATH="/opt/arm-gnu-toolchain-13.2.Rel1/bin:${PATH}"

# Download and install Corstone-300 ecosystem FVP
RUN rm -rf /home/ubuntu/FVP_Corstone_SSE-300; \
    mkdir /tmp/arm-ecosystem-fvp; \
    FVP_LINK=""; \
    if [[ $(uname -m) == "aarch64" ]]; then \
        FVP_LINK="_armv8l"; \
    fi; \
    wget -qO - https://developer.arm.com/-/media/Arm%20Developer%20Community/Downloads/OSS/FVP/Corstone-300/FVP_Corstone_SSE-300_11.24_13_Linux64${FVP_LINK}.tgz \
    | tar -xz -C /tmp/arm-ecosystem-fvp; \
    cd /tmp/arm-ecosystem-fvp && ./FVP_Corstone_SSE-300.sh --i-agree-to-the-contained-eula --no-interactive -q; \
    rm -rf /tmp/arm-ecosystem-fvp
ENV PATH="/home/ubuntu/FVP_Corstone_SSE-300/models/Linux64_armv8l_GCC-9.3:/home/ubuntu/FVP_Corstone_SSE-300/models/Linux64_GCC-9.3:${PATH}"

# Download and install Corstone-310 ecosystem FVP
RUN rm -rf /home/ubuntu/FVP_Corstone_SSE-310; \
    mkdir /tmp/arm-ecosystem-fvp; \
    FVP_LINK=""; \
    if [[ $(uname -m) == "aarch64" ]]; then \
        FVP_LINK="_armv8l"; \
    fi; \
    wget -qO - https://developer.arm.com/-/media/Arm%20Developer%20Community/Downloads/OSS/FVP/Corstone-310/FVP_Corstone_SSE-310_11.24_13_Linux64${FVP_LINK}.tgz \
    | tar -xz -C /tmp/arm-ecosystem-fvp; \
    cd /tmp/arm-ecosystem-fvp && ./FVP_Corstone_SSE-310.sh --i-agree-to-the-contained-eula --no-interactive -q; \
    rm -rf /tmp/arm-ecosystem-fvp
ENV PATH="/home/ubuntu/FVP_Corstone_SSE-310/models/Linux64_armv8l_GCC-9.3:/home/ubuntu/FVP_Corstone_SSE-310/models/Linux64_GCC-9.3:${PATH}"

# Download and install Corstone-315 ecosystem FVP
RUN rm -rf /home/ubuntu/FVP_Corstone_SSE-315; \
    mkdir /tmp/arm-ecosystem-fvp; \
    FVP_LINK=""; \
    if [[ $(uname -m) == "aarch64" ]]; then \
        FVP_LINK="_armv8l"; \
    fi; \
    wget -qO - https://developer.arm.com/-/media/Arm%20Developer%20Community/Downloads/OSS/FVP/Corstone-315/FVP_Corstone_SSE-315_11.24_22_Linux64${FVP_LINK}.tgz \
    | tar -xz -C /tmp/arm-ecosystem-fvp; \
    cd /tmp/arm-ecosystem-fvp && ./FVP_Corstone_SSE-315.sh --i-agree-to-the-contained-eula --no-interactive -q; \
    rm -rf /tmp/arm-ecosystem-fvp
ENV PATH="/home/ubuntu/FVP_Corstone_SSE-315/models/Linux64_armv8l_GCC-9.3:/home/ubuntu/FVP_Corstone_SSE-315/models/Linux64_GCC-9.3:${PATH}"

# Install uncrustify
# With ubuntu 22.04, the version of uncrustify installed is 0.72.0+dfsg1-2.
# However, we need the version 0.69.0. Therefore, build and install it manually.
RUN git clone https://github.com/uncrustify/uncrustify.git /tmp/uncrustify; \
    pushd /tmp/uncrustify; \
    git checkout uncrustify-0.69.0; \
    mkdir build && cd build; \
    cmake -DCMAKE_BUILD_TYPE=Release ..; \
    cmake --build . --config Release; \
    sudo cp uncrustify /usr/local/bin/uncrustify; \
    popd; popd; \
    rm -rf /tmp/uncrustify

# Setup ccache
RUN sudo ln -s /usr/bin/ccache /usr/lib/ccache/arm-none-eabi-gcc; \
    sudo ln -s /usr/bin/ccache /usr/lib/ccache/arm-none-eabi-g++; \
    sudo ln -s /usr/bin/ccache /usr/lib/ccache/arm-none-eabi-c++; \
    sudo ln -s /usr/bin/ccache /usr/lib/ccache/arm-none-eabi-cpp; \
    sudo ln -s /usr/bin/ccache /usr/lib/ccache/arm-none-eabi-ar
ENV PATH="/usr/lib/ccache:${PATH}"
