# SZM Forge - Build & Development Guide

This document provides comprehensive instructions for building, developing, and contributing to the SZM Forge physics simulation engine.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Build System Overview](#build-system-overview)
3. [Platform-Specific Build Instructions](#platform-specific-instructions)
4. [Dependency Management](#dependencies)
5. [Development Environment Setup](#dev-setup)
6. [Building Variants](#build-variants)
7. [Debugging & Profiling](#debugging)
8. [Contributing Guidelines](#contributing)
9. [Troubleshooting](#troubleshooting)

---

## Quick Start

### Linux / macOS

```bash
# 1. Clone and navigate to project
git clone <repository>
cd szm_forge

# 2. Create build directory
mkdir -p build && cd build

# 3. Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# 4. Build
cmake --build . --parallel 8

# 5. Run demo
./SZM_Forge
```

### Windows (Visual Studio)

```bash
# PowerShell
$cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
$cmake --build build --config Release --parallel 8
$.\build\Release\SZM_Forge.exe
```

---

## Build System Overview

### CMake Configuration

SZM Forge uses **CMake 3.21+** as its build system, organized hierarchically:

```
CMakeLists.txt (root)
├── src/
│   ├── Core/
│   ├── Physics/
│   ├── Graphics/
│   ├── UI/
│   └── ... (other modules)
├── external/ (third-party dependencies)
└── tests/ (unit tests)
```

### Key CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type: Debug, Release, RelWithDebInfo, MinSizeRel |
| `SZM_BUILD_FULL_DESKTOP` | OFF | Build full desktop GUI (requires GLFW, OpenGL) |
| `SZM_CHAIN_DESKTOP_AFTER_DEMO` | OFF | Launch desktop mode after terminal demo |
| `SZM_ENABLE_BULLET_BACKEND` | OFF | Enable Bullet Physics integration |
| `SZM_ENABLE_THREEJS_INTEGRATION` | ON | Enable Three.js web visualization |
| `SZM_ENABLE_ENTT_ECS` | ON | Enable EnTT Entity Component System |
| `SZM_ENABLE_FASTAPI_SERVER` | ON | Enable FastAPI REST server |
| `SZM_BUILD_WEBUI` | OFF | Build React-based web dashboard |
| `SZM_BUILD_AI_SERVICE` | OFF | Build Python AI service with PyTorch |

### Example Configurations

```bash
# Lightweight terminal-only demo (minimal dependencies)
cmake -DCMAKE_BUILD_TYPE=Release \
  -DSZM_BUILD_FULL_DESKTOP=OFF \
  ..

# Full desktop application with all features
cmake -DCMAKE_BUILD_TYPE=Release \
  -DSZM_BUILD_FULL_DESKTOP=ON \
  -DSZM_BUILD_WEBUI=ON \
  -DSZM_BUILD_AI_SERVICE=ON \
  ..

# Debug build with full diagnostics
cmake -DCMAKE_BUILD_TYPE=Debug \
  -DSZM_BUILD_FULL_DESKTOP=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  ..
```

---

## Platform-Specific Instructions

### Linux (Ubuntu 22.04 LTS)

#### Prerequisites

```bash
# Install build tools
sudo apt update
sudo apt install -y build-essential cmake git

# Install graphics dependencies (for desktop mode)
sudo apt install -y libglfw3-dev libglew-dev

# Install Python (for AI service)
sudo apt install -y python3-dev python3-pip

# Install optional tools
sudo apt install -y clang-format cppcheck

# For sanitizers (debugging)
sudo apt install -y libasan-dev libubsan-dev
```

#### Build Instructions

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build with verbose output (debug)
cmake --build build --verbose

# Build specific target
cmake --build build --target SZM_Forge

# Parallel build (8 cores)
cmake --build build --parallel 8

# Install (optional)
cmake --install build --prefix ~/.local
```

### macOS (Intel & Apple Silicon)

#### Prerequisites

```bash
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install build tools
brew install cmake llvm

# Install graphics dependencies
brew install glfw glew

# For Apple Silicon native build
arch -arm64 brew install glfw glew  # Uses native ARM64 binaries
```

#### Build Instructions

```bash
# Intel Mac
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 8

# Apple Silicon (M1/M2)
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  ..
cmake --build build --parallel 8

# Universal binary (Intel + ARM64)
cmake -B build-universal \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_BUILD_TYPE=Release \
  ..
cmake --build build-universal --parallel 8
```

### Windows (Visual Studio 2022)

#### Prerequisites

1. **Visual Studio 2022 Community Edition** with C++ workload
2. **CMake 3.21+** (install via Visual Studio installer or chocolatey)

```powershell
# Using Chocolatey
choco install cmake git

# Verify installation
cmake --version
```

#### Build Instructions

```powershell
# Generate Visual Studio project files
cmake -S . -B build -G "Visual Studio 17 2022"

# Build Release configuration
cmake --build build --config Release --parallel 8

# Build Debug configuration
cmake --build build --config Debug --parallel 4

# Open in Visual Studio IDE
start build/SZM_Forge.sln
```

#### Troubleshooting Windows Builds

- **Missing C++ workload:** Run Visual Studio Installer, add C++ development tools
- **CMake not found:** Add to PATH: `C:\Program Files\CMake\bin`
- **GLFW/OpenGL errors:** Install GLFW development files via vcpkg:
  ```powershell
  git clone https://github.com/Microsoft/vcpkg.git
  cd vcpkg
  .\vcpkg integrate install
  .\vcpkg install glfw3:x64-windows glew:x64-windows
  ```

---

## Dependencies

### Core Dependencies (Always Required)

| Library | Version | Purpose |
|---------|---------|---------|
| C++ Standard Library | C++20 | Language support |

### Optional Dependencies

| Library | Version | Feature | Install |
|---------|---------|---------|---------|
| GLFW | 3.3+ | Window & input (desktop GUI) | `apt install libglfw3-dev` |
| GLEW | 2.0+ | OpenGL extensions (desktop GUI) | `apt install libglew-dev` |
| Eigen | 3.3+ | Linear algebra (physics) | `apt install libeigen3-dev` |
| ImGui | 1.89+ | UI framework (docking branch) | `external/imgui/` (bundled) |
| EnTT | 3.10+ | ECS architecture | `external/entt/` (bundled) |
| Bullet | 3.x | Physics backend | `external_projects/bullet3/` |

### Dependency Installation by Platform

#### Linux (Ubuntu)

```bash
# Graphics + Simulation
sudo apt install -y libglfw3-dev libglew-dev libeigen3-dev

# Optional: Physics backends
sudo apt install -y libbullet-dev
```

#### macOS

```bash
# Graphics + Simulation
brew install glfw glew eigen

# Optional: Physics backends
brew install bullet
```

#### Windows (vcpkg)

```powershell
vcpkg install glfw3:x64-windows glew:x64-windows eigen3:x64-windows
```

---

## Development Environment Setup

### IDE Configuration

#### Visual Studio Code

1. **Install Extensions:**
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft)
   - Clang-Format (Xaver Klemenschits)

2. **Create `.vscode/settings.json`:**

```json
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "editor.formatOnSave": true,
    "[cpp]": {
        "editor.defaultFormatter": "ms-vscode.cpptools",
        "editor.tabSize": 4
    },
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build"
}
```

3. **Create `.vscode/c_cpp_properties.json`:**

```json
{
    "configurations": [
        {
            "name": "Linux/Mac",
            "includePath": [
                "${workspaceFolder}/src",
                "${workspaceFolder}/external"
            ],
            "defines": ["SZM_PLATFORM_LINUX"]
        }
    ]
}
```

#### Visual Studio 2022

1. Open `build/SZM_Forge.sln`
2. Set startup project: Solution Explorer → Right-click SZM_Forge → Set as Startup Project
3. Build configuration: Debug/Release dropdown
4. Run: F5 or Debug → Start Debugging

#### CLion (JetBrains)

1. Open project directory
2. CMake toolchain automatically detected
3. Build: Ctrl+F9 / Cmd+F9
4. Run: Shift+F10 / Ctrl+R

### Code Style & Formatting

#### Automatic Formatting

```bash
# Format all C++ files using clang-format
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Or use the provided script
./scripts/format_code.sh
```

#### IDE Integration

- **VS Code:** Install Clang-Format extension, enable format-on-save
- **Visual Studio:** Tools → Options → Text Editor → C/C++ → Code Style → Formatting
- **CLion:** Settings → Editor → Code Style → C++

### Git Hooks

Set up pre-commit hooks for code quality:

```bash
# Install pre-commit framework
pip install pre-commit

# Create .pre-commit-config.yaml
cat > .pre-commit-config.yaml << 'EOF'
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.4.0
    hooks:
      - id: end-of-file-fixer
      - id: trailing-whitespace
  - repo: https://github.com/google/clang-format
    rev: v16.0.0
    hooks:
      - id: clang-format
        types: [c++]
EOF

# Install hooks
pre-commit install
```

---

## Building Variants

### Debug Build (Development)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Debug flags: -g -O0 -DDEBUG
# Enables: Asserts, full debug info, no optimizations
```

**Use for:**
- Development and feature implementation
- Debugging with breakpoints
- Memory leak detection

### Release Build (Deployment)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 8

# Optimizations: -O3 -DNDEBUG
# Disables: Asserts, debug info stripped
```

**Use for:**
- Shipping builds
- Performance testing
- Distribution

### RelWithDebInfo (Profiling)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build

# Optimizations: -O2 with debug symbols
# Best for: Performance analysis while debugging
```

### Address Sanitizer Build (Memory Safety)

```bash
cmake -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
  ..
cmake --build build-asan

# Run with sanitizer enabled
ASAN_OPTIONS=verbosity=1 ./build-asan/SZM_Forge
```

### Thread Sanitizer Build (Race Detection)

```bash
cmake -B build-tsan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread" \
  ..
cmake --build build-tsan

# Run with thread sanitizer enabled
./build-tsan/SZM_Forge
```

---

## Debugging & Profiling

### GDB Debugging (Linux/macOS)

```bash
# Build with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Start debugger
gdb ./build/SZM_Forge

# GDB commands
(gdb) break main
(gdb) run arg1 arg2
(gdb) next
(gdb) step
(gdb) print variable
(gdb) backtrace
(gdb) watch variable
```

### Visual Studio Debugger (Windows)

1. Build Debug configuration
2. Debug → Start Debugging (F5)
3. Set breakpoints: Click left margin
4. Step: F10 (step over), F11 (step into)
5. Watch variables: Debug → Windows → Watch

### LLDB Debugging (macOS)

```bash
# Build with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Start debugger
lldb ./build/SZM_Forge

# LLDB commands
(lldb) breakpoint set -n main
(lldb) run arg1 arg2
(lldb) next
(lldb) step-in
(lldb) frame variable
(lldb) bt (backtrace)
```

### Profiling with perf (Linux)

```bash
# Build with profiling info
cmake -B build-prof \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer" \
  ..
cmake --build build-prof

# Record performance data
perf record -g ./build-prof/SZM_Forge

# Analyze results
perf report
perf flamegraph
```

### Memory Profiling with Valgrind

```bash
# Build with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run with Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./build/SZM_Forge

# Generate report
valgrind --leak-check=full --log-file=valgrind.log ./build/SZM_Forge
cat valgrind.log
```

---

## Contributing Guidelines

### Code Quality Standards

1. **C++ Standard:** C++20 (use modern features)
2. **Naming Convention:** See [StyleGuide.hpp](src/Core/StyleGuide.hpp)
3. **Documentation:** Use Doxygen comments (///) for public APIs
4. **Error Handling:** Use exceptions for error conditions; log via Logger

### Commit Workflow

```bash
# 1. Create feature branch
git checkout -b feature/your-feature

# 2. Make changes, follow code style
# ... edit files ...

# 3. Format code
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# 4. Build and test
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# 5. Commit
git add .
git commit -m "feat: Describe your feature

- Detail 1
- Detail 2"

# 6. Push and create PR
git push origin feature/your-feature
```

### Pull Request Checklist

- [ ] Code follows [StyleGuide.hpp](src/Core/StyleGuide.hpp) conventions
- [ ] All functions documented with Doxygen comments
- [ ] Error handling implemented (no silent failures)
- [ ] Build passes: `cmake --build build`
- [ ] No memory leaks: Run Address Sanitizer
- [ ] Tests pass (if applicable)
- [ ] Commit messages follow conventional commits

---

## Troubleshooting

### Build Issues

#### CMake Configuration Fails

```bash
# Clean build directory
rm -rf build
mkdir build
cd build

# Verbose output
cmake --debug-output ..
```

#### Missing Dependencies

```bash
# Check CMake error message for missing library
# Install via package manager (apt/brew/vcpkg)
# Or specify path manually:
cmake -DCMAKE_PREFIX_PATH=/opt/dependencies ..
```

#### Compiler Errors

```bash
# Check C++ standard is C++20
cmake --debug-output .. | grep "CMAKE_CXX_STANDARD"

# Force specific compiler
cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ..
```

### Runtime Issues

#### Segmentation Fault

```bash
# Run with AddressSanitizer
cmake -B build-asan \
  -DCMAKE_CXX_FLAGS="-fsanitize=address" ..
./build-asan/SZM_Forge
```

#### Memory Leaks

```bash
# Use Valgrind
valgrind --leak-check=full ./build/SZM_Forge
```

#### Thread Crashes

```bash
# Run with ThreadSanitizer
cmake -B build-tsan \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread" ..
./build-tsan/SZM_Forge
```

### Documentation

For more detailed information:

- **API Documentation:** Generate with Doxygen: `doxygen Doxyfile`
- **Physics Theory:** See `docs/physics-theory.md`
- **Architecture:** See `INTEGRATION_ARCHITECTURE.md`

---

## Support & Contact

For build issues or questions:

1. Check this guide and FAQ
2. Open an issue on GitHub with:
   - Platform (Linux/macOS/Windows)
   - Build command used
   - CMake version
   - Compiler version
   - Full error output

3. Contact: [maintainer contact info]

---

**Last Updated:** 2026-04-28  
**SZM Forge Version:** 0.2.1  
**CMake Minimum Version:** 3.21
