#pragma once

#include "models/town_state.h"
#include "models/character.h"
#include "engines/task_engine.h"
#include "engines/pomodoro_timer.h"
#include "engines/habit_tracker.h"
#include "engines/note_system.h"
#include "engines/gamification_engine.h"
#include "systems/building_upgrade_system.h"
#include "../ui/screens/screen_manager.h"
#include "../ui/window/window_manager.h"
#include "../ui/window/desktop_overlay_integration.h"
#include "../audio/audio_manager.h"
#include "../audio/audio_integration.h"
#include "../input/input_manager.h"
#include "../ui/animations/animation_manager.h"
#include "../persistence/save_manager.h"
#include <memory>
#include <string>
#include <functional>

// Central application state that coordinates all systems
class ApplicationState {
public:
    ApplicationState();
    ~ApplicationState();
    
    // Initialization and cleanup
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized; }
    
    // Core system access
    TownState& getTownState() { return *townState; }
    TaskEngine& getTaskEngine() { return *taskEngine; }
    PomodoroTimer& getPomodoroTimer() { return *pomodoroTimer; }
    HabitTracker& getHabitTracker() { return *habitTracker; }
    NoteSystem& getNoteSystem() { return *noteSystem; }
    GamificationEngine& getGamificationEngine() { return *gamificationEngine; }
    BuildingUpgradeSystem& getBuildingUpgradeSystem() { return *buildingUpgradeSystem; }
    
    // UI system access
    ScreenManager& getScreenManager() { return *screenManager; }
    InputManager& getInputManager() { return *inputManager; }
    AnimationManager& getAnimationManager() { return *animationManager; }
    
    // Window and audio system access
    std::shared_ptr<WindowManager> getWindowManager() { return windowManager; }
    std::shared_ptr<DesktopOverlayIntegration> getDesktopOverlay() { return desktopOverlay; }
    std::shared_ptr<AudioManager> getAudioManager() { return audioManager; }
    std::shared_ptr<AudioIntegration> getAudioIntegration() { return audioIntegration; }
    
    // Persistence
    SaveManager& getSaveManager() { return *saveManager; }
    
    // Application lifecycle
    void update(float deltaTime);
    void render();
    void handleInput();
    bool shouldClose() const;
    
    // Screen management
    void switchToTownScreen();
    void switchToCoffeeShopScreen();
    void switchToBulletinBoardScreen();
    void switchToLibraryScreen();
    void switchToGymScreen();
    void switchToHomeScreen();
    void switchToSettingsScreen();
    void returnToPreviousScreen();
    
    // Building interactions
    void enterBuilding(const std::string& buildingId);
    void exitCurrentBuilding();
    std::string getCurrentBuildingId() const { return currentBuildingId; }
    bool isInBuilding() const { return !currentBuildingId.empty(); }
    
    // Game state management
    void saveGame();
    bool loadGame();
    void newGame();
    void resetGame();
    
    // Settings management
    void applySettings();
    void saveSettings();
    void loadSettings();
    void resetSettings();
    
    // Event system
    void setupSystemCallbacks();
    void connectScreenCallbacks();
    void connectAudioCallbacks();
    void connectBuildingCallbacks();
    
    // Desktop mode integration
    void toggleDesktopMode();
    bool isDesktopModeEnabled() const;
    
    // Application events
    void setOnApplicationExit(std::function<void()> callback);
    void setOnGameStateChanged(std::function<void()> callback);
    
    // Debug and diagnostics
    void enableDebugMode(bool enable);
    bool isDebugModeEnabled() const { return debugMode; }
    std::vector<std::string> getSystemStatus() const;
    void logSystemState() const;

private:
    // Initialization state
    bool initialized = false;
    bool debugMode = false;
    
    // Core game systems
    std::unique_ptr<TownState> townState;
    std::unique_ptr<TaskEngine> taskEngine;
    std::unique_ptr<PomodoroTimer> pomodoroTimer;
    std::unique_ptr<HabitTracker> habitTracker;
    std::unique_ptr<NoteSystem> noteSystem;
    std::unique_ptr<GamificationEngine> gamificationEngine;
    std::unique_ptr<BuildingUpgradeSystem> buildingUpgradeSystem;
    
    // UI systems
    std::unique_ptr<ScreenManager> screenManager;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<AnimationManager> animationManager;
    
    // Window and audio systems
    std::shared_ptr<WindowManager> windowManager;
    std::shared_ptr<DesktopOverlayIntegration> desktopOverlay;
    std::shared_ptr<AudioManager> audioManager;
    std::shared_ptr<AudioIntegration> audioIntegration;
    
    // Persistence
    std::unique_ptr<SaveManager> saveManager;
    
    // Application state
    std::string currentBuildingId;
    std::string previousScreenName;
    bool gameLoaded = false;
    
    // Event callbacks
    std::function<void()> onApplicationExit;
    std::function<void()> onGameStateChanged;
    
    // Internal methods
    bool initializeCoreSystem();
    bool initializeUISystem();
    bool initializeAudioSystem();
    bool initializeWindowSystem();
    bool initializePersistence();
    
    void createAllScreens();
    void setupScreenTransitions();
    void setupSystemIntegration();
    
    // Screen creation helpers
    std::unique_ptr<Screen> createTownScreen();
    std::unique_ptr<Screen> createCoffeeShopScreen();
    std::unique_ptr<Screen> createBulletinBoardScreen();
    std::unique_ptr<Screen> createLibraryScreen();
    std::unique_ptr<Screen> createGymScreen();
    std::unique_ptr<Screen> createHomeScreen();
    std::unique_ptr<Screen> createSettingsScreen();
    
    // System callback setup
    void setupTaskEngineCallbacks();
    void setupPomodoroCallbacks();
    void setupHabitTrackerCallbacks();
    void setupGamificationCallbacks();
    void setupBuildingUpgradeCallbacks();
    
    // Data flow management
    void updateDataFlow();
    void syncSystemStates();
    void validateSystemIntegrity();
    
    // Error handling
    void handleSystemError(const std::string& system, const std::string& error);
    void recoverFromError();
    
    // Constants
    static constexpr const char* SAVE_FILE_PATH = "tasktown_save.json";
    static constexpr const char* SETTINGS_FILE_PATH = "tasktown_settings.json";
    static constexpr float AUTO_SAVE_INTERVAL = 30.0f; // Auto-save every 30 seconds
};