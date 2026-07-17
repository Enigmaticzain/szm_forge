#!/bin/bash

# SZM Forge Setup Script for Kali Linux
# Run this once to install all dependencies

echo "Installing SZM Forge dependencies for Kali Linux..."

# Update package list
sudo apt-get update

# Install build tools
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git

# Install graphics & UI dependencies
sudo apt-get install -y \
    libglfw3-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxkbcommon-dev

# Install math library
sudo apt-get install -y libeigen3-dev

# Clone ImGui docking branch if not present
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [ ! -d "$SCRIPT_DIR/external/imgui" ] || [ -z "$(ls -A "$SCRIPT_DIR/external/imgui")" ]; then
    echo "Cloning ImGui docking branch..."
    mkdir -p "$SCRIPT_DIR/external"
    git clone -b docking https://github.com/ocornut/imgui.git "$SCRIPT_DIR/external/imgui"
fi

echo "Setup complete! You can now run: ./build_and_run.sh"
