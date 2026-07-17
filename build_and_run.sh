#!/bin/bash

# SZM Forge Build & Run Script (Auto-updating)
# Detects new modules and rebuilds as needed

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build-full"
EXECUTABLE="$BUILD_DIR/SZM_Forge"
TIMESTAMP_FILE="$BUILD_DIR/.last_build_timestamp"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== SZM Forge Full Desktop Mode ===${NC}"

# Check dependencies
echo -e "${BLUE}Checking dependencies...${NC}"

# Install pkg-config if missing
if ! command -v pkg-config &> /dev/null; then
    echo -e "${YELLOW}pkg-config not found. Installing...${NC}"
    sudo apt-get update && sudo apt-get install -y pkg-config
fi

if ! command -v cmake &> /dev/null; then
    echo -e "${YELLOW}CMake not found. Installing...${NC}"
    sudo apt-get install -y cmake
fi

if ! pkg-config --exists glfw3 2>/dev/null; then
    echo -e "${YELLOW}GLFW3 not found. Installing...${NC}"
    sudo apt-get install -y libglfw3-dev
fi

if ! pkg-config --exists eigen3 2>/dev/null; then
    echo -e "${YELLOW}Eigen3 not found. Installing...${NC}"
    sudo apt-get install -y libeigen3-dev
fi

# Check if ImGui is cloned
if [ ! -d "$PROJECT_DIR/external/imgui" ] || [ -z "$(ls -A "$PROJECT_DIR/external/imgui")" ]; then
    echo -e "${YELLOW}ImGui docking branch not found. Cloning...${NC}"
    mkdir -p "$PROJECT_DIR/external"
    git clone -b docking https://github.com/ocornut/imgui.git "$PROJECT_DIR/external/imgui"
fi

# Detect if source files changed
CURRENT_HASH=$(find "$PROJECT_DIR/src" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "CMakeLists.txt" \) -exec md5sum {} \; | sort | md5sum | cut -d' ' -f1)
LAST_HASH=""
if [ -f "$TIMESTAMP_FILE" ]; then
    LAST_HASH=$(cat "$TIMESTAMP_FILE")
fi

if [ "$CURRENT_HASH" != "$LAST_HASH" ] || [ ! -f "$EXECUTABLE" ]; then
    echo -e "${BLUE}Building SZM Forge (full desktop mode)...${NC}"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake .. -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
    cmake --build . --parallel $(nproc)
    echo "$CURRENT_HASH" > "$TIMESTAMP_FILE"
    echo -e "${GREEN}Build complete!${NC}"
else
    echo -e "${GREEN}No changes detected. Skipping rebuild.${NC}"
fi

# Run the executable
echo -e "${BLUE}Launching SZM Forge...${NC}"
cd "$PROJECT_DIR"
"$EXECUTABLE"
