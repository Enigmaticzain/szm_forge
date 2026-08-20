#!/bin/bash
set -e

echo "=== Packaging SZM Forge as AppImage ==="

APP_NAME="SZM_Forge"
APPDIR="AppDir"
BUILD_DIR="../build"

# Clean up previous runs
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/szm_forge/ui"
mkdir -p "$APPDIR/usr/share/szm_forge/ai_service"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APPDIR/usr/share/applications"

# Check if binary exists
if [ ! -f "$BUILD_DIR/szm_forge" ]; then
    echo "Error: szm_forge binary not found in $BUILD_DIR. Please build it first."
    exit 1
fi

echo "Copying C++ Engine..."
cp "$BUILD_DIR/szm_forge" "$APPDIR/usr/bin/"

echo "Copying React UI..."
if [ -d "../new ui/dist" ]; then
    cp -r ../new\ ui/dist/* "$APPDIR/usr/share/szm_forge/ui/"
else
    echo "Warning: React UI dist not found. Did you run npm run build?"
fi

echo "Copying Python AI Service..."
if [ -d "../ai_service" ]; then
    cp -r ../ai_service/* "$APPDIR/usr/share/szm_forge/ai_service/"
else
    echo "Warning: ai_service not found."
fi

echo "Copying Desktop Entry and Icon..."
cp ../szm-forge.desktop "$APPDIR/usr/share/applications/"
cp ../assets/icon.png "$APPDIR/usr/share/icons/hicolor/256x256/apps/szm-forge.png"

# We must adjust the desktop file inside AppDir
sed -i 's|Exec=.*|Exec=szm_forge|g' "$APPDIR/usr/share/applications/szm-forge.desktop"
sed -i 's|Icon=.*|Icon=szm-forge|g' "$APPDIR/usr/share/applications/szm-forge.desktop"

# Download linuxdeploy if not exists
if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    echo "Downloading linuxdeploy..."
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

echo "Generating AppImage..."
export ARCH=x86_64
./linuxdeploy-x86_64.AppImage --appdir "$APPDIR" -i "$APPDIR/usr/share/icons/hicolor/256x256/apps/szm-forge.png" -d "$APPDIR/usr/share/applications/szm-forge.desktop" --output appimage

echo "=== Done! AppImage created successfully. ==="
