#!/bin/bash
set -e

echo "╔════════════════════════════════════════════════════════════╗"
echo "║  SZM Forge — Installing Dependencies & Building            ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Install all dependencies
echo "[1/4] Installing dependencies..."
sudo apt-get update
sudo apt-get install -y \
    cmake \
    g++ \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libeigen3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev

echo "      ✓ Dependencies installed"
echo ""

# Configure build
echo "[2/4] Configuring build..."
cd /mnt/D/projects/szm_forge
cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
echo "      ✓ Configuration complete"
echo ""

# Build
echo "[3/4] Building SZM Forge..."
cmake --build build-full --parallel $(nproc)
echo "      ✓ Build complete"
echo ""

# Run
echo "[4/4] Launching application..."
echo ""
./build-full/SZM_Forge
