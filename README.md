# TaskTown

A gamified desktop productivity application that combines task management, note-taking, habit tracking, and focus tools within an immersive 2D town environment.

## Features

- **Character-Controlled Interface**: Navigate through a virtual town to access different productivity tools
- **Pomodoro Timer**: Focus sessions in a cozy coffee shop with reward system
- **Task Management**: Visual bulletin board for organizing tasks with priorities and due dates
- **Note System**: Library for creating, organizing, and searching notes
- **Habit Tracking**: Gym environment for building and maintaining daily habits
- **Gamification**: Experience points, building upgrades, and visual progression

## Building

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)
- Git (for dependency fetching)

### Build Instructions

#### Windows

**Option 1: Using Visual Studio (Recommended)**
```cmd
# Prerequisites: Install Visual Studio 2022 with C++ development tools
# Download CMake from https://cmake.org/download/

# Build and run
build.bat
```

**Option 2: Using MinGW**
```cmd
# Prerequisites: Install MinGW-w64 and CMake
# Add both to your system PATH

# Build
build.bat

# Run
run.bat
```

#### macOS
```bash
# Install dependencies
brew install cmake

# Build and run
chmod +x build.sh run.sh
./run.sh
```

#### Linux
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt update
sudo apt install cmake build-essential git

# Or for other distributions, install equivalent packages

# Build and run
chmod +x build.sh run.sh
./run.sh
```

#### Manual Build (All Platforms)
```bash
# Create build directory
mkdir build
cd build

# Configure (choose appropriate generator for your platform)
cmake .. -G "Visual Studio 17 2022" -A x64  # Windows with VS2022
# or
cmake .. -G "MinGW Makefiles"                # Windows with MinGW
# or
cmake ..                                     # macOS/Linux (uses default)

# Build
cmake --build . --config Release

# Run the executable
# Windows: ./Release/TaskTown.exe or ./TaskTown.exe
# macOS: ./TaskTown.app/Contents/MacOS/TaskTown
# Linux: ./TaskTown
```

### Dependencies

The build system automatically fetches the following dependencies:
- **Raylib**: Graphics and input handling
- **nlohmann/json**: JSON serialization
- **Google Test**: Unit testing framework (for development builds)

### Troubleshooting

#### Windows Issues

**"CMake is not recognized"**
- Download and install CMake from https://cmake.org/download/
- Make sure to check "Add CMake to system PATH" during installation
- Restart your command prompt/PowerShell

**"No suitable compiler found"**
- Install Visual Studio 2022 Community (free) with "Desktop development with C++" workload
- Or install MinGW-w64 from https://www.mingw-w64.org/

**Build fails with "raylib not found"**
- This is normal - CMake will automatically download raylib
- Ensure you have internet connection during first build
- If behind corporate firewall, you may need to configure git proxy

**"Access denied" or permission errors**
- Run Command Prompt as Administrator
- Or use PowerShell with execution policy: `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser`

#### macOS/Linux Issues

**"Permission denied" when running scripts**
- Make scripts executable: `chmod +x build.sh run.sh`

**Missing dependencies**
- macOS: Install Xcode Command Line Tools: `xcode-select --install`
- Linux: Install build essentials for your distribution

## Project Structure

```
TaskTown/
├── src/
│   ├── core/              # Core business logic
│   ├── ui/                # User interface layer
│   ├── persistence/       # Data persistence
│   ├── input/             # Input handling
│   ├── audio/             # Audio system
│   └── main.cpp           # Application entry point
├── assets/                # Game assets
├── tests/                 # Unit tests
└── CMakeLists.txt         # Build configuration
```

## Development Status

This project is currently in active development. See the [implementation plan](.kiro/specs/tasktown-core/tasks.md) for current progress and upcoming features.