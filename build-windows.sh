#!/bin/bash
#
# Automated darktable build script for Windows (MSYS2/UCRT64)
# This script automates the entire build process
#

set -e  # Exit on error

echo "=========================================="
echo "darktable Windows Build Script"
echo "=========================================="
echo ""

# Check if we're in UCRT64 environment
if [[ ! "$MSYSTEM" == "UCRT64" ]]; then
    echo "ERROR: This script must be run from the UCRT64 terminal!"
    echo "Please launch 'UCRT64' from your Start menu and try again."
    exit 1
fi

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check for required tools
echo "Checking for required build tools..."
if ! command_exists cmake; then
    echo "ERROR: cmake not found. Please install build tools first:"
    echo "  pacman -S --needed mingw-w64-ucrt-x86_64-{cc,cmake,gcc-libs,ninja,omp}"
    exit 1
fi

if ! command_exists ninja; then
    echo "ERROR: ninja not found. Please install build tools first:"
    echo "  pacman -S --needed mingw-w64-ucrt-x86_64-{cc,cmake,gcc-libs,ninja,omp}"
    exit 1
fi

echo "✓ Build tools found"
echo ""

# Update submodules
echo "Updating git submodules..."
git submodule update --init
echo "✓ Submodules updated"
echo ""

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build directory..."
    rm -rf build
    echo "✓ Build directory cleaned"
    echo ""
fi

# Build type (default to Release)
BUILD_TYPE="${1:-Release}"
INSTALL_PREFIX="/opt/darktable"

echo "Build configuration:"
echo "  Build type: $BUILD_TYPE"
echo "  Install prefix: $INSTALL_PREFIX"
echo ""

# Configure
echo "Configuring darktable..."
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
    -S . \
    -B build

echo "✓ Configuration complete"
echo ""

# Build
echo "Building darktable (this may take a while)..."
echo "Using $(nproc) CPU cores for parallel build"
cmake --build build --parallel $(nproc)

echo "✓ Build complete!"
echo ""

# Install
echo "Installing to $INSTALL_PREFIX..."
cmake --install build

echo "✓ Installation complete!"
echo ""

# Create launcher script
echo "Creating launcher script..."
cat > "$INSTALL_PREFIX/launch-darktable.bat" << 'EOFBAT'
@echo off
REM darktable launcher for Windows
REM This sets up the environment and launches darktable

set MSYSTEM=UCRT64
set "PATH=C:\msys64\ucrt64\bin;%PATH%"

REM Optional: Use separate config directory for this build
REM Uncomment the next line to keep configs separate from any installed version
REM set DT_CONFIGDIR=%USERPROFILE%\.config\darktable-custom

start "" "%~dp0bin\darktable.exe" %*
EOFBAT

echo "✓ Launcher created: $INSTALL_PREFIX/launch-darktable.bat"
echo ""

echo "=========================================="
echo "BUILD SUCCESSFUL!"
echo "=========================================="
echo ""
echo "You can now run darktable in three ways:"
echo ""
echo "1. From UCRT64 terminal:"
echo "   $INSTALL_PREFIX/bin/darktable.exe"
echo ""
echo "2. From Windows (double-click):"
echo "   $INSTALL_PREFIX/launch-darktable.bat"
echo ""
echo "3. Run from build directory (no install):"
echo "   ./build/bin/darktable.exe"
echo ""
echo "To create a Windows installer (.exe):"
echo "   cmake --build build --target package"
echo ""
