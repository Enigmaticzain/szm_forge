#!/bin/bash
# SZM Forge Codes Integration - Quick Setup Script

set -e

echo "=========================================="
echo "SZM Forge Integration Setup"
echo "=========================================="

# Check prerequisites
echo "Checking prerequisites..."
command -v cmake >/dev/null 2>&1 || { echo "❌ CMake not found"; exit 1; }
command -v git >/dev/null 2>&1 || { echo "❌ Git not found"; exit 1; }

# Create symbolic links
echo ""
echo "Creating symbolic links..."
mkdir -p external
ln -sf ../codes/three.js-dev external/three.js 2>/dev/null || true
ln -sf ../codes/entt-main external/entt 2>/dev/null || true
ln -sf ../codes/fastapi-master external/fastapi 2>/dev/null || true
echo "✅ Symbolic links created"

# Configure CMake
echo ""
echo "Configuring CMake..."
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DSZM_BUILD_FULL_DESKTOP=ON \
  -DSZM_ENABLE_THREEJS_INTEGRATION=ON \
  -DSZM_ENABLE_ENTT_ECS=ON \
  -DSZM_ENABLE_FASTAPI_SERVER=ON

# Build
echo ""
echo "Building SZM Forge..."
cmake --build build --parallel --config Release

echo ""
echo "=========================================="
echo "✅ Core Build Complete!"
echo "=========================================="

# Optional: Web UI
read -p "Build React Web UI? (requires Node.js) [y/n]: " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Setting up web UI..."
    cd webui
    npm install
    npm run build
    cd ..
    echo "✅ Web UI built to webui/dist/"
fi

# Optional: AI Service
read -p "Setup Python AI Service? (requires Python 3.8+) [y/n]: " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Setting up AI service..."
    cd ai_service
    python -m venv venv
    source venv/bin/activate
    pip install -r requirements.txt
    cd ..
    echo "✅ AI service ready at ai_service/"
    echo "   Run: cd ai_service && source venv/bin/activate && python main.py"
fi

echo ""
echo "=========================================="
echo "🚀 Ready to Launch!"
echo "=========================================="
echo ""
echo "1. Start Desktop App:"
echo "   ./build/SZM_Forge"
echo ""
echo "2. Launch Web UI:"
echo "   cd webui && npm run dev"
echo ""
echo "3. Deploy AI Service:"
echo "   cd ai_service && source venv/bin/activate && python main.py"
echo ""
echo "=========================================="
