#!/bin/bash

# TaskTown Run Script
# Builds and runs the TaskTown application

set -e  # Exit on any error

echo "Building and running TaskTown..."

# Build the project
./build.sh

# Run the application
echo "Starting TaskTown..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    ./build/TaskTown.app/Contents/MacOS/TaskTown
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    ./build/TaskTown
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    # Windows
    ./build/TaskTown.exe
else
    echo "Unknown OS type: $OSTYPE"
    echo "Try running the executable manually from the build directory"
fi