#!/bin/bash

# Get the directory of the script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Define cross-compile path
export CROSS_COMPILE_PATH="$SCRIPT_DIR/../gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf/bin"
export PATH=$PATH:$CROSS_COMPILE_PATH

# Ensure CROSS_COMPILE_PATH is set correctly
if [ -z "$CROSS_COMPILE_PATH" ]; then
    echo "Error: CROSS_COMPILE_PATH is not set. Please update the script with the correct path."
    exit 1
fi

# Build kernel modules
make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm KDIR=../linux-5.10.19/build/

# Compile test_adxl
arm-linux-gnueabihf-gcc -Wall -o test_adxl test_adxl_concurrence.c

# Compile main
arm-linux-gnueabihf-gcc -Wall -o main main.c

echo "Compilation completed successfully."
