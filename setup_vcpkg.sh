#!/bin/bash

if [ ! -e vcpkg ]; then
  git clone https://github.com/microsoft/vcpkg.git
  pushd vcpkg
  export VCPKG_ROOT="${PWD}"
  git checkout "2024.11.16"
  ./bootstrap-vcpkg.sh -disableMetrics
  ./vcpkg install cxxopts spdlog libuv gtest glfw3 glm
  popd
fi
