@echo off
REM SZM Forge Quick Start Script (Windows)
REM Builds and runs the interactive demo with minimal user input

setlocal enabledelayedexpansion

echo.
echo ╔════════════════════════════════════════════════════════════╗
echo ║         SZM Forge - Quick Start                           ║
echo ║         Physics-Driven Engineering Platform               ║
echo ╚════════════════════════════════════════════════════════════╝
echo.

REM Check for CMake
where cmake >nul 2>nul
if errorlevel 1 (
    echo ❌ CMake not found. Please install CMake 3.20 or later.
    pause
    exit /b 1
)

echo ✓ Prerequisites found
echo.

REM Build
echo 📦 Building SZM Forge...
if not exist build mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
if errorlevel 1 (
    echo ❌ CMake configuration failed
    pause
    exit /b 1
)

cmake --build . --config Release --parallel
if errorlevel 1 (
    echo ❌ Build failed
    pause
    exit /b 1
)
cd ..

echo.
echo ✓ Build complete
echo.

REM Run
echo 🚀 Launching interactive demo...
echo.

if exist "build\Release\SZM_Forge_Interactive.exe" (
    build\Release\SZM_Forge_Interactive.exe
) else if exist "build\SZM_Forge_Interactive.exe" (
    build\SZM_Forge_Interactive.exe
) else (
    echo ⚠️  Interactive mode not available. Running standard demo...
    if exist "build\Release\SZM_Forge.exe" (
        build\Release\SZM_Forge.exe
    ) else (
        build\SZM_Forge.exe
    )
)

pause
