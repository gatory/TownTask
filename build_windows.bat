@echo off
REM TaskTown Cross-Platform Build Script for Windows
REM Optimized build with performance settings

echo Building TaskTown for Windows (Optimized)...

REM Check if CMake is installed
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

REM Clean previous build
if exist "build" (
    echo Cleaning previous build...
    rmdir /s /q build
)

REM Create build directory
echo Creating build directory...
mkdir build

REM Configure with optimizations
echo Configuring with CMake (Release + Optimizations)...
cmake -B build -S . ^
    -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG /GL" ^
    -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/LTCG /OPT:REF /OPT:ICF" ^
    -DBUILD_TESTS=OFF

if %errorlevel% neq 0 (
    echo Trying with MinGW Makefiles...
    cmake -B build -S . ^
        -G "MinGW Makefiles" ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG -march=native" ^
        -DBUILD_TESTS=OFF
    
    if %errorlevel% neq 0 (
        echo ERROR: CMake configuration failed
        pause
        exit /b 1
    )
)

REM Build the project
echo Compiling (Release)...
cmake --build build --config Release --parallel

if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

REM Copy assets to build directory
echo Copying assets...
if exist "assets" (
    xcopy /E /I /Y assets build\Release\assets
    if exist "build\assets" (
        xcopy /E /I /Y assets build\assets
    )
)

echo.
echo Build complete!
echo.
echo Executable locations:
if exist "build\Release\TaskTown.exe" (
    echo   build\Release\TaskTown.exe
)
if exist "build\TaskTown.exe" (
    echo   build\TaskTown.exe
)

echo.
echo Performance optimizations applied:
echo   - Release build with full optimizations
echo   - Link-time code generation (LTCG)
echo   - Dead code elimination
echo   - Native CPU optimizations
echo.

pause