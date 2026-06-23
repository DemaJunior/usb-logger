# Cross-compilation toolchain for Raspberry Pi (aarch64)
# Usage:
#   cmake --preset rpi-aarch64-debug
# or
#   cmake -S . -B out/build/rpi -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchains/raspi-aarch64.cmake

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Expect toolchain prefix installed on host (Ubuntu example):
#   sudo apt-get install g++-aarch64-linux-gnu
set(TOOLCHAIN_PREFIX aarch64-linux-gnu)

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

# Optional sysroot (recommended). Provide via environment:
#   RPI_SYSROOT=/path/to/sysroot
if(DEFINED ENV{RPI_SYSROOT} AND NOT "$ENV{RPI_SYSROOT}" STREQUAL "")
  set(CMAKE_SYSROOT $ENV{RPI_SYSROOT})
  set(CMAKE_FIND_ROOT_PATH $ENV{RPI_SYSROOT})
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
