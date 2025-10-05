# TaskTown Design Document

## Overview

TaskTown is a cross-platform desktop application built with C++ and Raylib that provides a gamified productivity experience. The application uses a layered architecture with clear separation between core logic, UI rendering, and data persistence. The design emphasizes modularity, testability, and cross-platform compatibility while maintaining high performance and low resource usage.

## Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │   Game Loop     │  │  Input Handler  │  │ State Mgr   │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                      UI Layer                               │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │  Town Renderer  │  │ Building UIs    │  │  Animations │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                     Core Layer                              │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │  Task Engine    │  │  Note System    │  │ Habit Track │ │
│  │  Pomodoro Timer │  │  Game Logic     │  │ Data Models │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                  Persistence Layer                          │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │  JSON Handler   │  │  Save/Load Mgr  │  │ File System │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Directory Structure

```
TaskTown/
├── src/
│   ├── core/                    # Core business logic
│   │   ├── models/              # Data models
│   │   │   ├── task.h/cpp
│   │   │   ├── note.h/cpp
│   │   │   ├── habit.h/cpp
│   │   │   ├── character.h/cpp
│   │   │   └── town_state.h/cpp
│   │   ├── engines/             # Core engines
│   │   │   ├── task_engine.h/cpp
│   │   │   ├── note_system.h/cpp
│   │   │   ├── habit_tracker.h/cpp
│   │   │   ├── pomodoro_timer.h/cpp
│   │   │   └── gamification_engine.h/cpp
│   │   └── utils/               # Utilities
│   │       ├── time_utils.h/cpp
│   │       └── id_generator.h/cpp
│   ├── ui/                      # User interface layer
│   │   ├── renderers/           # Rendering components
│   │   │   ├── town_renderer.h/cpp
│   │   │   ├── character_renderer.h/cpp
│   │   │   └── building_renderer.h/cpp
│   │   ├── screens/             # Screen implementations
│   │   │   ├── town_screen.h/cpp
│   │   │   ├── coffee_shop_screen.h/cpp
│   │   │   ├── bulletin_board_screen.h/cpp
│   │   │   ├── library_screen.h/cpp
│   │   │   └── gym_screen.h/cpp
│   │   ├── components/          # Reusable UI components
│   │   │   ├── button.h/cpp
│   │   │   ├── text_input.h/cpp
│   │   │   ├── list_view.h/cpp
│   │   │   └── progress_bar.h/cpp
│   │   └── animations/          # Animation system
│   │       ├── animation_manager.h/cpp
│   │       └── sprite_animator.h/cpp
│   ├── persistence/             # Data persistence
│   │   ├── json_serializer.h/cpp
│   │   ├── save_manager.h/cpp
│   │   └── file_utils.h/cpp
│   ├── input/                   # Input handling
│   │   ├── input_manager.h/cpp
│   │   └── key_bindings.h/cpp
│   ├── audio/                   # Audio system
│   │   ├── audio_manager.h/cpp
│   │   └── sound_effects.h/cpp
│   └── main.cpp                 # Application entry point
├── assets/
│   ├── sprites/                 # Character and building sprites
│   ├── ui/                      # UI textures and icons
│   ├── sounds/                  # Sound effects and music
│   └── fonts/                   # Font files
├── tests/                       # Unit tests
├── CMakeLists.txt
└── README.md
```

## Components and Interfaces

### Core Data Models

#### Task Model
```cpp
class Task {
public:
    enum Priority { LOW, MEDIUM, HIGH };
    enum Status { PENDING, IN_PROGRESS, COMPLETED };
    
    Task(const std::string& title, Priority priority = MEDIUM);
    
    // Getters/Setters
    uint32_t getId() const;
    std::string getTitle() const;
    void setTitle(const std::string& title);
    Priority getPriority() const;
    void setPriority(Priority priority);
    std::chrono::system_clock::time_point getDueDate() const;
    void setDueDate(const std::chrono::system_clock::time_point& date);
    Status getStatus() const;
    void setStatus(Status status);
    
    // Subtask management
    void addSubtask(std::shared_ptr<Task> subtask);
    std::vector<std::shared_ptr<Task>> getSubtasks() const;
    
    // Serialization
    nlohmann::json toJson() const;
    static Task fromJson(const nlohmann::json& json);
    
private:
    uint32_t id;
    std::string title;
    Priority priority;
    Status status;
    std::chrono::system_clock::time_point dueDate;
    std::chrono::system_clock::time_point createdAt;
    std::vector<std::shared_ptr<Task>> subtasks;
};
```

#### Character Model
```cpp
class Character {
public:
    struct Position {
        float x, y;
        Position(float x = 0, float y = 0) : x(x), y(y) {}
    };
    
    enum Direction { UP, DOWN, LEFT, RIGHT };
    enum State { IDLE, WALKING, INTERACTING, FOCUSED };
    
    Character(const std::string& name, Position startPos);
    
    // Movement
    void move(Direction direction, float deltaTime);
    void setPosition(const Position& pos);
    Position getPosition() const;
    
    // State management
    void setState(State state);
    State getState() const;
    Direction getFacingDirection() const;
    
    // Gamification
    void addExperience(int points);
    int getLevel() const;
    int getExperience() const;
    int getExperienceToNextLevel() const;
    
    // Serialization
    nlohmann::json toJson() const;
    static Character fromJson(const nlohmann::json& json);
    
private:
    std::string name;
    Position position;
    Direction facingDirection;
    State currentState;
    int level;
    int experience;
    float movementSpeed;
};
```

### Core Engines

#### Task Engine Interface
```cpp
class TaskEngine {
public:
    TaskEngine();
    
    // Task management
    uint32_t createTask(const std::string& title, Task::Priority priority = Task::MEDIUM);
    bool updateTask(uint32_t taskId, const Task& updatedTask);
    bool deleteTask(uint32_t taskId);
    bool completeTask(uint32_t taskId);
    
    // Querying
    std::vector<Task> getAllTasks() const;
    std::vector<Task> getTasksByPriority(Task::Priority priority) const;
    std::vector<Task> getOverdueTasks() const;
    std::optional<Task> getTask(uint32_t taskId) const;
    
    // Subtask management
    bool addSubtask(uint32_t parentId, uint32_t subtaskId);
    std::vector<Task> getSubtasks(uint32_t parentId) const;
    
    // Events
    void setTaskCompletionCallback(std::function<void(const Task&)> callback);
    
private:
    std::unordered_map<uint32_t, Task> tasks;
    std::function<void(const Task&)> onTaskCompleted;
    uint32_t nextTaskId;
};
```

#### Pomodoro Timer
```cpp
class PomodoroTimer {
public:
    enum State { STOPPED, RUNNING, PAUSED, COMPLETED };
    enum SessionType { WORK, SHORT_BREAK, LONG_BREAK };
    
    PomodoroTimer();
    
    // Timer control
    void start(int durationMinutes, SessionType type = WORK);
    void pause();
    void resume();
    void stop();
    void reset();
    
    // State queries
    State getState() const;
    SessionType getCurrentSessionType() const;
    int getRemainingSeconds() const;
    int getTotalSeconds() const;
    float getProgress() const; // 0.0 to 1.0
    
    // Session management
    int getCompletedPomodoros() const;
    bool shouldTakeLongBreak() const; // After 4 pomodoros
    
    // Events
    void setCompletionCallback(std::function<void(SessionType)> callback);
    void setTickCallback(std::function<void(int)> callback);
    
    void update(float deltaTime);
    
private:
    State currentState;
    SessionType sessionType;
    float remainingTime;
    float totalTime;
    int completedPomodoros;
    std::function<void(SessionType)> onCompletion;
    std::function<void(int)> onTick;
};
```

### UI System Design

#### Screen Management
```cpp
class Screen {
public:
    virtual ~Screen() = default;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleInput(const InputState& input) = 0;
    virtual void onEnter() {}
    virtual void onExit() {}
};

class ScreenManager {
public:
    void pushScreen(std::unique_ptr<Screen> screen);
    void popScreen();
    void replaceScreen(std::unique_ptr<Screen> screen);
    void update(float deltaTime);
    void render();
    void handleInput(const InputState& input);
    
private:
    std::stack<std::unique_ptr<Screen>> screens;
};
```

#### Window Management and Desktop Overlay
```cpp
enum class WindowMode {
    WINDOWED,
    DESKTOP_OVERLAY
};

class WindowManager {
public:
    WindowManager();
    
    // Window mode management
    void setWindowMode(WindowMode mode);
    WindowMode getWindowMode() const;
    bool isDesktopMode() const;
    
    // Desktop overlay functionality
    void enableDesktopMode();
    void disableDesktopMode();
    void setupTransparentWindow();
    void setupClickThrough();
    
    // Screen boundary management
    Rectangle getScreenBounds() const;
    Vector2 clampToScreen(Vector2 position) const;
    
    // Window state
    void saveWindowState();
    void restoreWindowState();
    
private:
    WindowMode currentMode;
    Rectangle savedWindowRect;
    bool wasMaximized;
    
    void configureWindowForMode(WindowMode mode);
    void handleModeTransition(WindowMode from, WindowMode to);
};
```
```

#### Town Screen Implementation
```cpp
class TownScreen : public Screen {
public:
    TownScreen(Character& character, TownState& townState, WindowManager& windowManager);
    
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const InputState& input) override;
    
private:
    void updateCharacterMovement(const InputState& input, float deltaTime);
    void checkBuildingInteractions();
    void renderTown();
    void renderCharacter();
    void renderUI();
    void renderDesktopOverlay();
    void handleDesktopModeInput(const InputState& input);
    
    Character& character;
    TownState& townState;
    WindowManager& windowManager;
    TownRenderer townRenderer;
    CharacterRenderer characterRenderer;
    std::vector<Building> buildings;
    
    // Desktop mode specific
    float overlayAlpha;
    bool showBuildings;
};
```

### Animation System

#### Sprite Animation
```cpp
class SpriteAnimator {
public:
    struct Animation {
        std::vector<Rectangle> frames;
        float frameDuration;
        bool loop;
    };
    
    SpriteAnimator(Texture2D spriteSheet);
    
    void addAnimation(const std::string& name, const Animation& animation);
    void playAnimation(const std::string& name);
    void update(float deltaTime);
    Rectangle getCurrentFrame() const;
    bool isAnimationComplete() const;
    
private:
    Texture2D spriteSheet;
    std::unordered_map<std::string, Animation> animations;
    std::string currentAnimation;
    int currentFrame;
    float frameTimer;
    bool animationComplete;
};
```

## Data Models

### JSON Data Structure
```json
{
  "user": {
    "name": "Player",
    "character": {
      "position": {"x": 100, "y": 150},
      "level": 5,
      "experience": 1250,
      "facingDirection": "DOWN"
    }
  },
  "tasks": [
    {
      "id": 1,
      "title": "Complete homework",
      "priority": "HIGH",
      "status": "PENDING",
      "dueDate": "2025-10-10T23:59:59Z",
      "createdAt": "2025-10-01T10:00:00Z",
      "subtasks": [2, 3]
    }
  ],
  "notes": [
    {
      "id": 1,
      "title": "C++ Notes",
      "content": "...",
      "tags": ["programming", "cpp"],
      "createdAt": "2025-10-01T10:00:00Z",
      "modifiedAt": "2025-10-02T15:30:00Z"
    }
  ],
  "habits": [
    {
      "id": 1,
      "name": "Reading",
      "streak": 7,
      "lastCompleted": "2025-10-04",
      "targetFrequency": "DAILY",
      "completionHistory": ["2025-09-28", "2025-09-29", "..."]
    }
  ],
  "town": {
    "buildings": {
      "coffeeShop": {"level": 2, "decorations": ["plant_01"]},
      "library": {"level": 1, "decorations": []},
      "bulletinBoard": {"level": 1, "decorations": []},
      "gym": {"level": 3, "decorations": ["dumbbell_01", "treadmill_01"]}
    },
    "globalDecorations": ["tree_01", "bench_02"],
    "unlockedItems": ["coffee_varieties_basic", "gym_equipment_tier1"]
  },
  "gamification": {
    "totalExperience": 1250,
    "level": 5,
    "achievements": ["first_task", "week_streak", "pomodoro_master"],
    "currency": {
      "coffee": 15,
      "experience": 1250
    }
  }
}
```

### Save File Management
```cpp
class SaveManager {
public:
    SaveManager(const std::string& saveDirectory);
    
    bool saveGameState(const GameState& state);
    std::optional<GameState> loadGameState();
    bool createBackup();
    bool restoreFromBackup();
    
    // Auto-save functionality
    void enableAutoSave(int intervalSeconds);
    void disableAutoSave();
    
private:
    std::string saveDirectory;
    std::string mainSaveFile;
    std::string backupSaveFile;
    std::thread autoSaveThread;
    std::atomic<bool> autoSaveEnabled;
    int autoSaveInterval;
    
    bool validateSaveFile(const std::string& filePath);
    void autoSaveLoop();
};
```

## Error Handling

### Error Categories
1. **File System Errors**: Save/load failures, missing assets
2. **Data Validation Errors**: Corrupted save files, invalid JSON
3. **Resource Errors**: Missing textures, audio files
4. **Input Errors**: Invalid user input, key binding conflicts

### Error Handling Strategy
```cpp
class ErrorHandler {
public:
    enum ErrorLevel { INFO, WARNING, ERROR, CRITICAL };
    
    static void logError(ErrorLevel level, const std::string& message, 
                        const std::string& context = "");
    static void showUserError(const std::string& message);
    static bool handleRecoverableError(const std::string& error, 
                                     std::function<bool()> recoveryAction);
    
private:
    static void writeToLogFile(const std::string& logEntry);
    static void displayErrorDialog(const std::string& message);
};

// Usage example
if (!saveManager.saveGameState(currentState)) {
    ErrorHandler::handleRecoverableError(
        "Failed to save game state",
        [&]() { return saveManager.createBackup(); }
    );
}
```

## Testing Strategy

### Unit Testing Framework
- **Framework**: Google Test (gtest)
- **Coverage Target**: 80% code coverage for core logic
- **Test Categories**:
  - Model tests (Task, Note, Habit, Character)
  - Engine tests (TaskEngine, PomodoroTimer, etc.)
  - Serialization tests (JSON conversion)
  - Utility function tests

### Integration Testing
- Screen transition testing
- Save/load functionality testing
- Cross-platform compatibility testing
- Performance benchmarking

### Test Structure Example
```cpp
class TaskEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        taskEngine = std::make_unique<TaskEngine>();
    }
    
    void TearDown() override {
        taskEngine.reset();
    }
    
    std::unique_ptr<TaskEngine> taskEngine;
};

TEST_F(TaskEngineTest, CreateTask_ValidInput_ReturnsTaskId) {
    uint32_t taskId = taskEngine->createTask("Test Task", Task::HIGH);
    EXPECT_GT(taskId, 0);
    
    auto task = taskEngine->getTask(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->getTitle(), "Test Task");
    EXPECT_EQ(task->getPriority(), Task::HIGH);
}

TEST_F(TaskEngineTest, CompleteTask_ValidTask_TriggersCallback) {
    bool callbackTriggered = false;
    taskEngine->setTaskCompletionCallback([&](const Task& task) {
        callbackTriggered = true;
    });
    
    uint32_t taskId = taskEngine->createTask("Test Task");
    taskEngine->completeTask(taskId);
    
    EXPECT_TRUE(callbackTriggered);
}
```

### Performance Testing
- Frame rate consistency testing (target: 60 FPS)
- Memory usage monitoring (target: <150MB)
- Startup time measurement (target: <3 seconds)
- Large dataset handling (1000+ tasks, notes)

## Build System and Dependencies

### CMake Configuration
```cmake
cmake_minimum_required(VERSION 3.16)
project(TaskTown VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(raylib REQUIRED)
find_package(nlohmann_json REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Create executable
add_executable(TaskTown ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(TaskTown raylib nlohmann_json::nlohmann_json)

# Include directories
target_include_directories(TaskTown PRIVATE src/)

# Copy assets
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Platform-specific configurations
if(WIN32)
    set_target_properties(TaskTown PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(TaskTown PROPERTIES MACOSX_BUNDLE TRUE)
endif()
```

### Dependency Management
- **Raylib**: Graphics and input handling
- **nlohmann/json**: JSON serialization
- **Google Test**: Unit testing (development only)
- **CMake**: Build system
- **vcpkg/Conan**: Package management

This design provides a solid foundation for implementing TaskTown with clear separation of concerns, testability, and maintainability while meeting all the performance and cross-platform requirements.