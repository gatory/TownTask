# Windows Setup Guide for TaskTown

This guide will help Windows developers get TaskTown running on their machines.

## Quick Setup (Recommended)

### Step 1: Install Prerequisites

1. **Install Visual Studio 2022 Community** (Free)
   - Download from: https://visualstudio.microsoft.com/vs/community/
   - During installation, select "Desktop development with C++"
   - This includes the MSVC compiler and Windows SDK

2. **Install CMake**
   - Download from: https://cmake.org/download/
   - Choose "Windows x64 Installer"
   - **Important**: Check "Add CMake to system PATH" during installation

3. **Install Git** (if not already installed)
   - Download from: https://git-scm.com/download/win
   - Use default settings during installation

### Step 2: Clone and Build

1. **Open Command Prompt or PowerShell**
   - Press `Win + R`, type `cmd`, press Enter
   - Or search for "Command Prompt" in Start menu

2. **Clone the repository**
   ```cmd
   git clone <repository-url>
   cd TaskTown
   ```

3. **Build and run**
   ```cmd
   build.bat
   ```

4. **Run the application**
   - The executable will be at: `build\Release\TaskTown.exe`
   - Or use: `run.bat` to build and run automatically

## Alternative Setup (MinGW)

If you prefer not to install Visual Studio:

### Step 1: Install MinGW-w64

1. **Download MinGW-w64**
   - Go to: https://www.mingw-w64.org/downloads/
   - Choose "MingW-W64-builds"
   - Download the installer

2. **Install with these settings:**
   - Architecture: x86_64
   - Threads: posix
   - Exception: seh
   - Build revision: latest

3. **Add to PATH**
   - Add `C:\mingw64\bin` to your system PATH
   - (Adjust path based on your installation location)

### Step 2: Install CMake and Git
   - Same as above (CMake and Git)

### Step 3: Build
   ```cmd
   build.bat
   ```
   - The script will automatically detect MinGW if Visual Studio is not available

## Verification

After building, you should see:
- A window opens with "TaskTown - Gamified Productivity"
- A green grass background with colored buildings
- A blue character that you can move with WASD or arrow keys
- Buildings highlight when you get close
- "Press E to enter" prompt appears near buildings

## Common Issues

### "CMake is not recognized"
- Restart your command prompt after installing CMake
- Verify installation: `cmake --version`

### "No suitable compiler found"
- Make sure Visual Studio 2022 is installed with C++ tools
- Or install MinGW-w64 as described above

### Build takes a long time on first run
- This is normal - CMake is downloading dependencies (Raylib, etc.)
- Subsequent builds will be much faster

### Antivirus blocking the executable
- Some antivirus software may flag the executable
- Add the `build` folder to your antivirus exclusions

## Development

### Running Tests
```cmd
# Build with tests
cmake --build build --config Release

# Run tests
build\Release\TaskTownTests.exe
```

### IDE Setup
- Open the project folder in Visual Studio 2022
- Or use Visual Studio Code with C++ extensions
- CMake integration should work automatically

## Getting Help

If you encounter issues:
1. Check this troubleshooting guide
2. Verify all prerequisites are installed correctly
3. Try deleting the `build` folder and running `build.bat` again
4. Ask the team for help with specific error messages