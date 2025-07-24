#!/bin/bash
# Build script for Sig language compiler

set -e

# Parse arguments
INSTALL=false
if [ "$1" = "--install" ] || [ "$1" = "-i" ]; then
    INSTALL=true
fi

echo "Building Sig Language Compiler..."

# Check if LLVM is available
if ! command -v llvm-config &> /dev/null; then
    echo "Error: LLVM not found. Please install LLVM development packages."
    echo "   Ubuntu/Debian: sudo apt install llvm-dev"
    echo "   Arch Linux: sudo pacman -S llvm"
    echo "   macOS: brew install llvm"
    exit 1
fi

# Check LLVM version
LLVM_VERSION=$(llvm-config --version)
echo "Found LLVM version: $LLVM_VERSION"

# Clean previous build
echo "Cleaning previous build..."
rm -rf CMakeFiles/ CMakeCache.txt cmake_install.cmake *.cmake

# Configure with CMake
echo "Configuring build..."
cmake .

# Build
echo "Compiling..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build complete! Sig compiler is ready."

# Install if requested
if [ "$INSTALL" = true ]; then
    echo ""
    ./install.sh
else
    echo ""
    echo "Try it out:"
    echo "   ./sig examples/hello_world.sg          # Create executable"
    echo "   ./sig examples/variables.sg -o myapp   # Custom output name"
    echo "   ./sig examples/functions.sg --jit      # JIT execution"
    echo "   ./sig examples/variables.sg --ir       # Show LLVM IR"
    echo ""
    echo "To install system-wide, run: ./install.sh"
    echo "   Or build and install in one step: ./build.sh --install"
fi
