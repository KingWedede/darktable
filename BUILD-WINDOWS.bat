@echo off
REM ============================================
REM darktable Windows One-Click Build Script
REM ============================================
REM
REM This script automatically builds darktable
REM Double-click this file to start the build
REM

echo ==========================================
echo darktable Windows Builder
echo ==========================================
echo.

REM Check if MSYS2 is installed
if not exist "C:\msys64\ucrt64.exe" (
    echo ERROR: MSYS2 not found!
    echo.
    echo Please install MSYS2 first:
    echo 1. Download from https://www.msys2.org
    echo 2. Run the installer
    echo 3. Then run this script again
    echo.
    pause
    exit /b 1
)

echo MSYS2 found. Starting build process...
echo.
echo This will:
echo 1. Update MSYS2 packages
echo 2. Install build dependencies
echo 3. Build darktable
echo 4. Create launcher
echo.
echo Press any key to continue or Ctrl+C to cancel...
pause >nul

REM Launch MSYS2 UCRT64 and run the build script
C:\msys64\ucrt64.exe -here -c "cd '%~dp0' && ./build-windows.sh"

echo.
echo ==========================================
echo Build process completed!
echo ==========================================
echo.
echo Check the output above for any errors.
echo.
pause
