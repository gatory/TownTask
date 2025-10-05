#!/bin/bash

# TaskTown Development Setup Script
# This script installs the required dependencies for building TaskTown

echo "Setting up TaskTown development environment..."

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Error: Homebrew is not installed. Please install Homebrew first:"
    echo "https://brew.sh/"
    exit 1
fi

# Install CMake
echo "Installing CMake..."
brew install cmake

# Install pkg-config (helpful for finding libraries)
echo "Installing pkg-config..."
brew install pkg-config

# Optional: Install raylib and nlohmann-json via Homebrew
# (The CMake file will fetch them automatically if not found)
echo "Installing optional dependencies..."
brew install raylib nlohmann-json

echo "Setup complete!"
echo ""
echo "To build TaskTown:"
echo "  mkdir build"
echo "  cd build"
echo "  cmake .."
echo "  make"
echo ""
echo "Or use the build script:"
echo "  ./build.sh"