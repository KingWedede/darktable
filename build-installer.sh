#!/bin/bash
#
# Build darktable Windows installer package
# Creates a darktable-<version>-win64.exe installer
#

set -e

echo "=========================================="
echo "darktable Windows Installer Builder"
echo "=========================================="
echo ""

# Check environment
if [[ ! "$MSYSTEM" == "UCRT64" ]]; then
    echo "ERROR: This script must be run from the UCRT64 terminal!"
    exit 1
fi

# Check for NSIS
if ! pacman -Qs mingw-w64-ucrt-x86_64-nsis > /dev/null; then
    echo "NSIS is not installed. Installing now..."
    pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-nsis
fi

# Update submodules
echo "Updating submodules..."
git submodule update --init

# Clean build directory
echo "Cleaning build directory..."
rm -rf build

# Configure for installer build
echo "Configuring for installer package..."
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/opt/darktable \
    -DBINARY_PACKAGE_BUILD=ON \
    -S . \
    -B build

# Build
echo "Building darktable..."
cmake --build build --parallel $(nproc)

# Create installer package
echo ""
echo "Creating Windows installer package..."
cmake --build build --target package

# Find the installer
INSTALLER=$(find build -name "darktable-*-win64.exe" -type f | head -n 1)

if [ -n "$INSTALLER" ]; then
    echo ""
    echo "=========================================="
    echo "INSTALLER CREATED SUCCESSFULLY!"
    echo "=========================================="
    echo ""
    echo "Installer location:"
    echo "  $INSTALLER"
    echo ""
    echo "Windows path:"
    # Convert MSYS path to Windows path
    WINPATH=$(cygpath -w "$INSTALLER" 2>/dev/null || echo "$INSTALLER")
    echo "  $WINPATH"
    echo ""
    echo "You can now:"
    echo "1. Navigate to the build folder in Windows Explorer"
    echo "2. Double-click the installer to install darktable"
    echo "3. Share this installer with others"
    echo ""
    echo "NOTE: This installer is optimized for your CPU."
    echo "If you need a generic installer for any PC, use:"
    echo "  -DBINARY_PACKAGE_BUILD=ON"
    echo ""
else
    echo "ERROR: Installer not found!"
    exit 1
fi
