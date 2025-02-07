#!/bin/bash

# Define variables
DTS_FILE_NAME="vexpress-v2p-ca9.dts"
KERNEL_FILENAME_EXTRACT="linux-5.10.19"
BUILD_DIR="$KERNEL_FILENAME_EXTRACT/build"
DTS_PREPROCESSED="preprocessed-vexpress-v2p-ca9.dts"
DTB_OUTPUT="vexpress-v2p-ca9.dtb"

# Check if the kernel extract directory exists
if [ -d "$KERNEL_FILENAME_EXTRACT" ]; then
    echo "Copying $DTS_FILE_NAME to $BUILD_DIR..."
    cp -f "$DTS_FILE_NAME" "$BUILD_DIR"
    cd "$BUILD_DIR" || { echo "Failed to change directory to $BUILD_DIR"; exit 1; }
    
    echo "Preprocessing Device Tree Source..."
    gcc -E -Wp,-MMD,arch/arm/boot/dts/.vexpress-v2p-ca9.dtb.d.pre.tmp \
        -nostdinc -I../scripts/dtc/include-prefixes -undef -D__DTS__ \
        -x assembler-with-cpp -o "$DTS_PREPROCESSED" ../arch/arm/boot/dts/vexpress-v2p-ca9.dts
    
    echo "Compiling Device Tree Blob..."
    dtc -I dts -O dtb "$DTS_PREPROCESSED" -o "$DTB_OUTPUT"
    
    cd ../.. || { echo "Failed to return to the original directory"; exit 1; }
    echo "Device tree compilation completed successfully."
else
    echo "Error: $KERNEL_FILENAME_EXTRACT does not exist. Please download and extract it first."
    exit 1
fi