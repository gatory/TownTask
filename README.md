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
- C++17 compatible compiler
- Git (for dependency fetching)

### Build Instructions

#### Quick Start
```bash
# Install dependencies (macOS with Homebrew)
./setup.sh

# Build and run
./run.sh
```

#### Manual Build
```bash
# Clone the repository
git clone <repository-url>
cd TaskTown

# Install dependencies (macOS)
brew install cmake raylib nlohmann-json

# Build
./build.sh

# Run
./build/TaskTown.app/Contents/MacOS/TaskTown  # macOS
# or
./build/TaskTown  # Linux
# or
./build/TaskTown.exe  # Windows
```

### Dependencies

The build system automatically fetches the following dependencies:
- **Raylib**: Graphics and input handling
- **nlohmann/json**: JSON serialization

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