#!/bin/bash

DOWNLOAD_KERNEL_URL="https://perso.telecom-paristech.fr/duc/cours/linux/data/linux_build.tar.xz"
DOWNLOAD_FILESYSTEM_URL="https://perso.telecom-paristech.fr/duc/cours/linux/data/rootfs.cpio.gz"
DOWNLOAD_QEMU_ARM_URL="https://perso.telecom-paristech.fr/duc/cours/linux/data/ubuntu_2204/qemu-system-arm"
DOWNLOAD_GCC_ARM_URL="https://releases.linaro.org/components/toolchain/binaries/7.4-2019.02/arm-linux-gnueabihf/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf.tar.xz"

KERNEL_FILENAME="linux_build.tar.xz"
KERNEL_FILENAME_EXTRACT="linux-5.10.19" 
FILESYSTEM_FILENAME="rootfs.cpio.gz"
FILESYSTEM_FILENAME_EXTRACT="rootfs.cpio"
QEMU_ARM_COMMAND="qemu-system-arm"
GCC_ARM_FILENAME="gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf.tar.xz"
GCC_ARM_FILENAME_EXTRACT="gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf"

##########################
########## LINUX KERNEL
##########################
# Download the kernel file if it does not exist
if [ ! -f "$KERNEL_FILENAME" ]; then
    echo "Downloading $KERNEL_FILENAME..."
    wget -O "$DOWNLOAD_KERNEL_URL"
else
    echo "$KERNEL_FILENAME already exists, skipping download."
fi

# Extract the kernel folder if it exists
if [ ! -d "$KERNEL_FILENAME_EXTRACT" ]; then
    echo "Extracting $KERNEL_FILENAME..."
    tar -xJf "$KERNEL_FILENAME"
else
    echo "$KERNEL_FILENAME_EXTRACT already exists, skipping extraction."
fi

##########################
########## FILESYSTEM
##########################
# Download the filesystem file if it does not exist
if [ ! -f "$FILESYSTEM_FILENAME" ]; then
    echo "Downloading $FILESYSTEM_FILENAME..."
    wget -O "$DOWNLOAD_FILESYSTEM_URL"
else
    echo "$FILESYSTEM_FILENAME already exists, skipping download."
fi

# Extract the filesystem file if it exists
if [ ! -f "$FILESYSTEM_FILENAME_EXTRACT" ]; then
    echo "Extracting $FILESYSTEM_FILENAME_EXTRACT..."
    tar -xJf "$FILESYSTEM_FILENAME"
else
    echo "$FILESYSTEM_FILENAME_EXTRACT already exists, skipping extraction."
fi

##########################
########## qemu-system-arm
##########################
# Download the qemu-system-arm command if it does not exist
if [ ! -f "$QEMU_ARM_COMMAND" ]; then
    echo "Downloading $QEMU_ARM_COMMAND..."
    wget -O "$DOWNLOAD_QEMU_ARM_URL"
    chmod u+x "$QEMU_ARM_COMMAND"
else
    echo "$QEMU_ARM_COMMAND already exists, skipping download."
fi

##########################
########## gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf
##########################
# Download the cross compiler file if it does not exist
if [ ! -f "$GCC_ARM_FILENAME" ]; then
    echo "Downloading $GCC_ARM_FILENAME..."
    wget -O "$DOWNLOAD_GCC_ARM_URL"
else
    echo "$GCC_ARM_FILENAME already exists, skipping download."
fi

# Extract the cross compiler folder if it exists
if [ ! -d "$GCC_ARM_FILENAME_EXTRACT" ]; then
    echo "Extracting $GCC_ARM_FILENAME_EXTRACT..."
    tar -xJf "$GCC_ARM_FILENAME"
else
    echo "$GCC_ARM_FILENAME_EXTRACT already exists, skipping extraction."
fi