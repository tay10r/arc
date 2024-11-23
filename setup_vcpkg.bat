@echo off
if not exist vcpkg (
  git clone https://github.com/microsoft/vcpkg.git
)
pushd vcpkg
set "VCPKG_ROOT=%cd%"
git checkout "2024.11.16"
call .\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg.exe install cxxopts spdlog libuv gtest
