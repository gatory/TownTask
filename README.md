# Town Game - ECS-based 2D Game

A 2D game built with raylib using an Entity Component System architecture.

## Features

- Multiple buildings (Pomodoro Timer, Library, House)
- Interactive NPCs and objects
- Library system with PDF management (JSON storage)
- Todo list with persistence (JSON storage)
- Exit doors in each building interior
- Full UI with raygui

## Building

### Using CMake (MinGW):
```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
.\TownGame.exe
```

### Using CMake (Visual Studio):
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
.\Release\TownGame.exe
```

## Controls

- **Arrow Keys**: Move player
- **X**: Enter building / Interact with objects
- **Z**: Exit building (when near door)
- **Mouse**: UI interactions
- **Mouse Wheel**: Scroll lists

## Project Structure

```
TownGame/
+-- include/          # Header files
¦   +-- Core/        # ECS core
¦   +-- Data/        # Data management
¦   +-- Systems/     # Game systems
¦   +-- Resources/   # Resource management
¦   +-- Game/        # Game logic
+-- src/             # Implementation files
+-- assets/          # Sprites and textures
+-- data/            # JSON data files
+-- build/           # Build output
```

## Required Assets

Place these files in the `assets/` folder:
- Idle.png (player sprite sheet: 320x32, 10 frames)
- speech_v2_32x24.png (speech bubble)
- shop-800x449.png (pomodoro background)
- library-bg.png (library background)
- house-bg.png (house background)
- librarian.png (48x48 NPC sprite)
- desk.png (80x60 desk sprite)
- door.png (30x40 door sprite - optional)

## Dependencies

- raylib (graphics library)
- raygui (UI library - header-only, included with raylib)
- C++17 compiler (MSVC, MinGW-w64, or Clang)

## License

MIT License
