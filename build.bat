@echo off
REM TaskTown Build Script for Windows
REM Builds the TaskTown application using CMake

echo Building TaskTown for Windows...

REM Check if CMake is installed
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

REM Configure with CMake
echo Configuring with CMake...
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
if %errorlevel% neq 0 (
    echo Trying with MinGW Makefiles...
    cmake -B build -S . -G "MinGW Makefiles"
    if %errorlevel% neq 0 (
        echo ERROR: CMake configuration failed
        pause
        exit /b 1
    )
)

REM Build the project
echo Compiling...
cmake --build build --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo Build complete!
echo Run the application with: build\Release\TaskTown.exe
echo Or: build\TaskTown.exe (if using MinGW)
pause