#!/bin/bash
# Automatically build and launch SZM Forge
cd /mnt/D/projects/szm_forge

echo "Building SZM Forge in Desktop Mode..."
cmake -B build -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build -j$(nproc)

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Launching SZM Forge..."
    ./build/SZM_Forge "$@"
else
    echo "Build failed. Press Enter to exit."
    read -r
fi
