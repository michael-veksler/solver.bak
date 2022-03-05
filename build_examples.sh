#!/usr/bin/env bash

# This script is intended to be run from an Ubuntu Docker image (or other Debian-derived distros).
# It uses `apt-get` to install dependencies required by the examples,

set -euo pipefail

function install_nana() {
  # CMakeLists.txt is 'supposed' to install all of these automatically, but for
  # some reason it doesn't always work. Installing them manually does work.
  apt-get install -y --no-install-recommends libjpeg8-dev libpng-dev \
    libasound2-dev alsa-utils alsa-oss libx11-dev libxft-dev libxcursor-dev
  # Note: Nana's headers trigger the -Wshadow warning, and cannot be compiled with -Werror.
  # Supposedly, this is fixed in the develop-1.8 branch, but I cannot confirm.
}

# call the above functions to install the required dependencies. As an example for qt:
# install_qt

# build with:
cmake -S . -B "./build"
cmake --build "./build"