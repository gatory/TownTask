#!/bin/bash

# TaskTown Build Script
# Builds the TaskTown application

set -e  # Exit on any error

echo "Building TaskTown..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Configure with CMake
echo "Configuring with CMake..."
cmake -B build -S .

# Build the project
echo "Compiling..."
cmake --build build

echo "Build complete!"
echo "Run the application with: ./build/TaskTown"