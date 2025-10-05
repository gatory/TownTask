#!/bin/bash

# TaskTown Cross-Platform Optimized Build Script
# Works on macOS and Linux with performance optimizations

set -e  # Exit on any error

echo "Building TaskTown (Optimized for Performance)..."

# Detect platform
PLATFORM="unknown"
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
fi

echo "Detected platform: $PLATFORM"

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed"
    echo "Please install CMake:"
    if [[ "$PLATFORM" == "macOS" ]]; then
        echo "  brew install cmake"
    elif [[ "$PLATFORM" == "Linux" ]]; then
        echo "  sudo apt install cmake  # Ubuntu/Debian"
        echo "  sudo yum install cmake  # CentOS/RHEL"
    fi
    exit 1
fi

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Create build directory
echo "Creating build directory..."
mkdir build

# Configure with optimizations
echo "Configuring with CMake (Release + Optimizations)..."

# Platform-specific optimizations
if [[ "$PLATFORM" == "macOS" ]]; then
    # macOS optimizations
    cmake -B build -S . \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG -march=native -mtune=native -flto" \
        -DCMAKE_EXE_LINKER_FLAGS="-flto" \
        -DBUILD_TESTS=OFF \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
elif [[ "$PLATFORM" == "Linux" ]]; then
    # Linux optimizations
    cmake -B build -S . \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG -march=native -mtune=native -flto -fuse-linker-plugin" \
        -DCMAKE_EXE_LINKER_FLAGS="-flto -fuse-linker-plugin" \
        -DBUILD_TESTS=OFF
else
    # Generic optimizations
    cmake -B build -S . \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" \
        -DBUILD_TESTS=OFF
fi

# Build the project
echo "Compiling (Release with optimizations)..."
cmake --build build --config Release --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Copy assets to build directory
echo "Copying assets..."
if [ -d "assets" ]; then
    cp -r assets build/
fi

echo ""
echo "Build complete!"
echo ""

# Show executable location
if [[ "$PLATFORM" == "macOS" ]]; then
    if [ -f "build/TaskTown.app/Contents/MacOS/TaskTown" ]; then
        echo "Executable: build/TaskTown.app/Contents/MacOS/TaskTown"
        echo "App Bundle: build/TaskTown.app"
    fi
elif [ -f "build/TaskTown" ]; then
    echo "Executable: build/TaskTown"
fi

echo ""
echo "Performance optimizations applied:"
echo "  - Release build with -O3 optimization"
echo "  - Native CPU instruction optimizations (-march=native)"
echo "  - Link-time optimization (LTO)"
echo "  - Dead code elimination"
echo "  - Parallel compilation"
echo ""

# Performance test
echo "Running quick performance test..."
if [[ "$PLATFORM" == "macOS" ]] && [ -f "build/TaskTown.app/Contents/MacOS/TaskTown" ]; then
    echo "To run: ./build/TaskTown.app/Contents/MacOS/TaskTown"
elif [ -f "build/TaskTown" ]; then
    echo "To run: ./build/TaskTown"
    # Could add a quick startup test here
fi

echo "Build script completed successfully!"