#!/bin/bash
#
# TarkOS Cross-Compiler Build Script
# Builds i686-elf-gcc and i686-elf-binutils for OS development
#
# Usage: ./build_cross.sh
# Time: ~20-40 minutes depending on system
#

set -e

# Configuration
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# Versions
BINUTILS_VERSION="2.41"
GCC_VERSION="13.2.0"

# Directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$HOME/src/cross-build"

echo "========================================"
echo "TarkOS Cross-Compiler Build Script"
echo "========================================"
echo "Target: $TARGET"
echo "Prefix: $PREFIX"
echo "Build directory: $BUILD_DIR"
echo "========================================"

# Create directories
mkdir -p "$PREFIX"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Download sources if not present
if [ ! -f "binutils-$BINUTILS_VERSION.tar.xz" ]; then
    echo "[1/6] Downloading binutils $BINUTILS_VERSION..."
    wget "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz"
fi

if [ ! -f "gcc-$GCC_VERSION.tar.xz" ]; then
    echo "[2/6] Downloading GCC $GCC_VERSION..."
    wget "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz"
fi

# Extract sources
if [ ! -d "binutils-$BINUTILS_VERSION" ]; then
    echo "[3/6] Extracting binutils..."
    tar xf "binutils-$BINUTILS_VERSION.tar.xz"
fi

if [ ! -d "gcc-$GCC_VERSION" ]; then
    echo "[4/6] Extracting GCC..."
    tar xf "gcc-$GCC_VERSION.tar.xz"
fi

# Build binutils
echo "[5/6] Building binutils for $TARGET..."
mkdir -p build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror
make -j$(nproc)
make install
cd ..

# Build GCC
echo "[6/6] Building GCC for $TARGET..."
mkdir -p build-gcc
cd build-gcc
../gcc-$GCC_VERSION/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c \
    --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..

echo "========================================"
echo "Cross-compiler built successfully!"
echo "========================================"
echo ""
echo "Add the following to your ~/.bashrc or ~/.profile:"
echo ""
echo "  export PATH=\"\$HOME/opt/cross/bin:\$PATH\""
echo ""
echo "Then run: source ~/.bashrc"
echo ""
echo "Verify installation:"
echo "  i686-elf-gcc --version"
echo "  i686-elf-ld --version"
echo "========================================"
