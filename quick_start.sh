#!/bin/bash

# SZM Forge Quick Start Script
# Builds and runs the interactive demo with minimal user input

set -e

echo "╔════════════════════════════════════════════════════════════╗"
echo "║         SZM Forge - Quick Start                           ║"
echo "║         Physics-Driven Engineering Platform               ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found. Please install CMake 3.20 or later."
    exit 1
fi

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "❌ C++ compiler not found. Please install GCC 11+ or Clang 14+."
    exit 1
fi

echo "✓ Prerequisites found"
echo ""

# Build
echo "📦 Building SZM Forge..."
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
cd ..

echo ""
echo "✓ Build complete"
echo ""

# Run
echo "🚀 Launching interactive demo..."
echo ""

if [ -f "build/SZM_Forge_Interactive" ]; then
    ./build/SZM_Forge_Interactive
else
    echo "⚠️  Interactive mode not available. Running standard demo..."
    ./build/SZM_Forge
fi
