#!/bin/bash

# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

sudo apt-get -y update
sudo apt-get -y install make build-essential libssl-dev zlib1g-dev libbz2-dev libreadline-dev libsqlite3-dev wget curl llvm libncurses5-dev libncursesw5-dev xz-utils tk-dev liblzma-dev tk-dev
mkdir /tmp/python_vsi_setup
cd /tmp/python_vsi_setup
wget https://www.python.org/ftp/python/3.9.18/Python-3.9.18.tgz
tar xzf Python-3.9.18.tgz
cd Python-3.9.18
./configure --prefix=/opt/python/3.9.18/ --enable-optimizations
make -j "$(nproc)"
sudo make altinstall
sudo rm -rf /tmp/python_vsi_setup
